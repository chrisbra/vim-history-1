/* vi:set ts=8 sts=4 sw=4:
 *
 * VIM - Vi IMproved	by Bram Moolenaar
 *			Netbeans integration by David Weatherford
 *
 * Do ":help uganda"  in Vim to read copying and usage conditions.
 * Do ":help credits" in Vim to see a list of people who contributed.
 */

/*
 * Implements client side of org.netbeans.modules.emacs editor
 * integration protocol.  Be careful!  The protocol uses offsets
 * which are *between* characters, whereas vim uses line number
 * and column number which are *on* characters.
 * See ":help netbeans-protocol" for explanation.
 */

#include "vim.h"

#if defined(FEAT_NETBEANS_INTG) || defined(PROTO)

/* Note: when making changes here also adjust configure.in. */
#include <stdarg.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#ifdef HAVE_LIBGEN_H
# include <libgen.h>
#endif

#include "version.h"

#define INET_SOCKETS

#define GUARDED		10000 /* typenr for "guarded" annotation */
#define GUARDEDOFFSET 1000000 /* base for "guarded" sign id's */

/* The first implementation (working only with Netbeans) returned "1.1".  The
 * protocol implemented here also supports A-A-P. */
static char *ExtEdProtocolVersion = "2.1";

static long pos2off __ARGS((buf_T *, pos_T *));
static pos_T *off2pos __ARGS((buf_T *, long));
static pos_T *get_off_or_lnum __ARGS((buf_T *buf, char_u **argp));
static long get_buf_size __ARGS((buf_T *));

static void netbeans_connect __ARGS((void));

static void nb_init_graphics __ARGS((void));
static void coloncmd __ARGS((char *cmd, ...));
#ifdef FEAT_GUI_MOTIF
static void messageFromNetbeans __ARGS((XtPointer, int *, XtInputId *));
#endif
#ifdef FEAT_GUI_GTK
static void messageFromNetbeans __ARGS((gpointer, gint, GdkInputCondition));
#endif
static void nb_parse_cmd __ARGS((char_u *));
static int  nb_do_cmd __ARGS((int, char_u *, int, int, char_u *));
static void nb_send __ARGS((char *buf, char *fun));
static void warn __ARGS((char *fmt, ...));
#ifdef FEAT_BEVAL
static void netbeans_beval_cb __ARGS((BalloonEval *beval, int state));
#endif

static int sd = -1;			/* socket fd for Netbeans connection */
#ifdef FEAT_GUI_MOTIF
static XtInputId inputHandler;		/* Cookie for input */
#endif
#ifdef FEAT_GUI_GTK
static gint inputHandler;		/* Cookie for input */
#endif
static int cmdno;			/* current command number for reply */
static int haveConnection = FALSE;	/* socket is connected and
					   initialization is done */
static int oldFire = 1;
static int exit_delay = 2;		/* exit delay in seconds */

#ifdef FEAT_BEVAL
# if defined(FEAT_GUI_MOTIF) || defined(FEAT_GUI_ATHENA)
extern Widget	textArea;
# endif
BalloonEval	*balloonEval = NULL;
#endif

/*
 * Include the debugging code if wanted.
 */
#ifdef NBDEBUG
# include "nbdebug.c"
#endif

/* Connect back to Netbeans process */
#if defined(FEAT_GUI_MOTIF) || defined(PROTO)
    void
netbeans_Xt_connect(void *context)
{
    netbeans_connect();
    if (sd > 0)
    {
	/* tell notifier we are interested in being called
	 * when there is input on the editor connection socket
	 */
	inputHandler = XtAppAddInput((XtAppContext)context, sd,
			     (XtPointer)(XtInputReadMask + XtInputExceptMask),
						   messageFromNetbeans, NULL);
    }
}

    static void
netbeans_disconnect(void)
{
    if (inputHandler != (XtInputId)NULL)
    {
	XtRemoveInput(inputHandler);
	inputHandler = (XtInputId)NULL;
    }
    sd = -1;
    haveConnection = FALSE;
}
#endif /* FEAT_MOTIF_GUI */

#if defined(FEAT_GUI_GTK) || defined(PROTO)
    void
netbeans_gtk_connect(void)
{
# ifdef FEAT_BEVAL
    /*
     * Set up the Balloon Expression Evaluation area.
     * Always create it but disable it when 'ballooneval' isn't set.
     */
    balloonEval = gui_mch_create_beval_area(gui.drawarea, NULL,
					    &netbeans_beval_cb, NULL);
    if (!p_beval)
	gui_mch_disable_beval_area(balloonEval);
# endif

    netbeans_connect();
    if (sd > 0)
    {
	/*
	 * Tell gdk we are interested in being called when there
	 * is input on the editor connection socket
	 */
	inputHandler = gdk_input_add(sd, (GdkInputCondition)
		((int)GDK_INPUT_READ + (int)GDK_INPUT_EXCEPTION),
						   messageFromNetbeans, NULL);
    }
}

    static void
netbeans_disconnect(void)
{
    if (inputHandler != 0)
    {
	gdk_input_remove(inputHandler);
	inputHandler = 0;
    }
    sd = -1;
    haveConnection = FALSE;
}
#endif /* FEAT_GUI_GTK */

    static void
netbeans_connect(void)
{
#ifdef INET_SOCKETS
    struct sockaddr_in	server;
    struct hostent *	host;
    int			port;
#else
    struct sockaddr_un	server;
#endif
    char	buf[32];
    char *	hostname;
    char *	address;
    char *	password;

    /* netbeansArg is -nb or -nb:<host>:<addr>:<password> */
    if (netbeansArg[3] == ':')
	netbeansArg += 4;
    else
	netbeansArg = NULL;

    hostname = netbeansArg;
    if (hostname == NULL || *hostname == '\0')
	hostname = getenv("__NETBEANS_HOST");
    if (hostname == NULL || *hostname == '\0')
	hostname = "localhost"; /* default */

    address = strchr(hostname, ':');
    if (address != NULL)
	*address++ = '\0';
    else
	address = getenv("__NETBEANS_SOCKET");
    if (address == NULL || *address == '\0')
	address = "3219";  /* default */

    password = strchr(address, ':');
    if (password != NULL)
	*password++ = '\0';
    else
	password = getenv("__NETBEANS_VIM_PASSWORD");
    if (password == NULL || *password == '\0')
	password = "changeme"; /* default */

#ifdef INET_SOCKETS
    port = atoi(address);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
	perror("socket() in netbeans_connect()");
	return;
    }

    /* Get the server internet address and put into addr structure */
    /* fill in the socket address structure and connect to server */
    memset((char *)&server, '\0', sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if ((host = gethostbyname(hostname)) == NULL)
    {
	if (access(hostname, R_OK) >= 0)
	{
	    /* DEBUG: input file */
	    sd = open(hostname, O_RDONLY);
	    return;
	}
	perror("gethostbyname() in netbeans_connect()");
	sd = -1;
	return;
    }
    memcpy((char *)&server.sin_addr, host->h_addr, host->h_length);
#else
    if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
	perror("socket()");
	return;
    }

    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, address);
#endif
    /* Connect to server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(server)))
    {
	nbdebug(("netbeans_connect: Connect failed with errno %d\n", errno));
	if (errno == ECONNREFUSED)
	{
	    close(sd);
#ifdef INET_SOCKETS
	    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	    {
		perror("socket()#2 in netbeans_connect()");
		return;
	    }
#else
	    if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
	    {
		perror("socket()#2 in netbeans_connect()");
		return;
	    }
#endif
	    if (connect(sd, (struct sockaddr *)&server, sizeof(server)))
	    {
		int retries = 36;
		int success = FALSE;
		while (retries--
			     && ((errno == ECONNREFUSED) || (errno == EINTR)))
		{
		    nbdebug(("retrying...\n"));
		    sleep(5);
		    if (connect(sd, (struct sockaddr *)&server,
				sizeof(server)) == 0)
		    {
			success = TRUE;
			break;
		    }
		}
		if (!success)
		{
		    /* Get here when the server can't be found. */
		    perror(_("Cannot connect to Netbeans #2"));
		    getout(1);
		}
	    }

	}
	else
	{
	    perror(_("Cannot connect to Netbeans"));
	    getout(1);
	}
    }

    sprintf(buf, "AUTH %s\n", password);
    nb_send(buf, "netbeans_connect");

    sprintf(buf, "0:version=0 \"%s\"\n", ExtEdProtocolVersion);
    nb_send(buf, "externaleditor_version");

    nbdebug(("netbeans_connect: Connection succeeded\n"));

/*    nb_init_graphics();  delay until needed */

    haveConnection = TRUE;

    return;
}


struct keyqueue
{
    int		     key;
    struct keyqueue *next;
    struct keyqueue *prev;
};

typedef struct keyqueue keyQ_T;

static keyQ_T keyHead; /* dummy node, header for circular queue */


/*
 * Queue up key commands sent from netbeans.
 */
    static void
postpone_keycommand(int key)
{
    keyQ_T *node;

    node = (keyQ_T *)alloc(sizeof(keyQ_T));

    if (keyHead.next == NULL) /* initialize circular queue */
    {
	keyHead.next = &keyHead;
	keyHead.prev = &keyHead;
    }

    /* insert node at tail of queue */
    node->next = &keyHead;
    node->prev = keyHead.prev;
    keyHead.prev->next = node;
    keyHead.prev = node;

    node->key = key;
}

/*
 * Handle any queued-up NetBeans keycommands to be send.
 */
    static void
handle_key_queue(void)
{
    while (keyHead.next && keyHead.next != &keyHead)
    {
	/* first, unlink the node */
	keyQ_T *node = keyHead.next;
	keyHead.next = node->next;
	node->next->prev = node->prev;

	/* now, send the keycommand */
	netbeans_keycommand(node->key);

	/* Finally, dispose of the node */
	vim_free(node);
    }
}


struct cmdqueue
{
    char_u	    *buffer;
    struct cmdqueue *next;
    struct cmdqueue *prev;
};

typedef struct cmdqueue queue_T;

static queue_T head;  /* dummy node, header for circular queue */


/*
 * Put the buffer on the work queue; possibly save it to a file as well.
 */
    static void
save(char_u *buf, int len)
{
    queue_T *node;

    node = (queue_T *)alloc(sizeof(queue_T));
    if (node == NULL)
	return;	    /* out of memory */
    node->buffer = alloc(len + 1);
    if (node->buffer == NULL)
    {
	vim_free(node);
	return;	    /* out of memory */
    }
    mch_memmove(node->buffer, buf, (size_t)len);
    node->buffer[len] = NUL;

    if (head.next == NULL)   /* initialize circular queue */
    {
	head.next = &head;
	head.prev = &head;
    }

    /* insert node at tail of queue */
    node->next = &head;
    node->prev = head.prev;
    head.prev->next = node;
    head.prev = node;

#ifdef NBDEBUG
    {
	static int outfd = -2;

	/* possibly write buffer out to a file */
	if (outfd == -3)
	    return;

	if (outfd == -2)
	{
	    char *file = getenv("__NETBEANS_SAVE");
	    if (file == NULL)
		outfd = -3;
	    else
		outfd = open(file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	}

	if (outfd >= 0)
	    write(outfd, buf, len);
    }
#endif
}


/*
 * While there's still a command in the work queue, parse and execute it.
 */
    static void
nb_parse_messages(void)
{
    char_u	*p;
    queue_T	*node;

    while (head.next != &head)
    {
	node = head.next;

	/* Locate the first line in the first buffer. */
	p = vim_strchr(node->buffer, '\n');
	if (p == NULL)
	{
	    /* Command isn't complete.  If there is no following buffer,
	     * return (wait for more). If there is another buffer following,
	     * prepend the text to that buffer and delete this one.  */
	    if (node->next == &head)
		return;
	    p = alloc(STRLEN(node->buffer) + STRLEN(node->next->buffer) + 1);
	    if (p == NULL)
		return;	    /* out of memory */
	    STRCPY(p, node->buffer);
	    STRCAT(p, node->next->buffer);
	    vim_free(node->next->buffer);
	    node->next->buffer = p;

	    /* dispose of the node and buffer */
	    head.next = node->next;
	    node->next->prev = node->prev;
	    vim_free(node->buffer);
	    vim_free(node);
	}
	else
	{
	    /* There is a complete command at the start of the buffer.
	     * Terminate it with a NUL.  When no more text is following unlink
	     * the buffer.  Do this before executing, because new buffers can
	     * be added while busy handling the command. */
	    *p++ = NUL;
	    if (*p == NUL)
	    {
		head.next = node->next;
		node->next->prev = node->prev;
	    }

	    /* now, parse and execute the commands */
	    nb_parse_cmd(node->buffer);

	    if (*p == NUL)
	    {
		/* buffer finished, dispose of the node and buffer */
		vim_free(node->buffer);
		vim_free(node);
	    }
	    else
	    {
		/* more follows, move to the start */
		mch_memmove(node->buffer, p, STRLEN(p) + 1);
	    }
	}
    }
}

/* Buffer size for reading incoming messages. */
#define MAXMSGSIZE 4096

/*
 * Read and process a command from netbeans.
 */
/*ARGSUSED*/
#ifdef FEAT_GUI_MOTIF
    static void
messageFromNetbeans(XtPointer clientData, int *unused1, XtInputId *unused2)
#endif
#ifdef FEAT_GUI_GTK
    static void
messageFromNetbeans(gpointer clientData, gint unused1,
						    GdkInputCondition unused2)
#endif
{
    static char_u	*buf = NULL;
    int			len;
    int			readlen = 0;
    static int		level = 0;

    if (sd < 0)
    {
	nbdebug(("messageFromNetbeans() called without a socket\n"));
	return;
    }

    ++level;  /* recursion guard; this will be called from the X event loop */

    /* Allocate a buffer to read into. */
    if (buf == NULL)
    {
	buf = alloc(MAXMSGSIZE);
	if (buf == NULL)
	    return;	/* out of memory! */
    }

    /* Keep on reading for as long as there is something to read. */
    for (;;)
    {
	len = read(sd, buf, MAXMSGSIZE);
	if (len <= 0)
	    break;	/* error or nothing more to read */

	/* Store the read message in the queue. */
	save(buf, len);
	readlen += len;

	if (len < MAXMSGSIZE)
	    break;	/* did read everything that's available */
    }

    if (readlen <= 0)
    {
	/* read error or didn't read anything */
	netbeans_disconnect();
	nbdebug(("messageFromNetbeans: Error in read() from socket\n"));
	if (len < 0)
	    perror(_("read from Netbeans socket"));
	return; /* don't try to parse it */;
    }

    /* Parse the messages, but avoid recursion. */
    if (level == 1)
	nb_parse_messages();

    --level;
}

/*
 * Issue a warning.  This used to go to stderr but that doesn't show up when
 * started from a GUI.
 */
    static void
warn(char *fmt, ...)
{
    va_list ap;
    char    buf[1000];

    va_start(ap, fmt);
    vsnprintf(buf, 1000, fmt, ap);
    va_end(ap);
    EMSG(buf);
}

/*
 * Issue a warning to stderr and possibly abort.
 */
#if 0 /* we should never abort, changed files could be lost! */
static void die __ARGS((char *fmt, ...));

    static void
die(char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
/*     exit(1); */
    if (getenv("__NETBEANS_INTG_ABORT"))
	abort();
}
#else
# define die warn
#endif

/*
 * Handle one NUL terminated command.
 *
 * format of a command from netbeans:
 *
 *    6:setTitle!84 "a.c"
 *
 *    bufno
 *     colon
 *      cmd
 *		!
 *		 cmdno
 *		    args
 *
 * for function calls, the ! is replaced by a /
 */
    static void
nb_parse_cmd(char_u *cmd)
{
    char_u	*verb;
    char_u	*q;
    int		bufno;
    int		isfunc = -1;

    if (STRCMP(cmd, "DISCONNECT") == 0)
    {
	/* We assume the server knows that we can safely exit! */
	if (sd >= 0)
	    close(sd);
	/* Disconnect before exiting, Motif hangs in a Select error
	 * message otherwise. */
	netbeans_disconnect();
	getout(0);
	/* NOTREACHED */
    }

    if (STRCMP(cmd, "DETACH") == 0)
    {
	/* The IDE is breaking the connection. */
	if (sd >= 0)
	    close(sd);
	netbeans_disconnect();
	return;
    }

    bufno = strtol((char *)cmd, (char **)&verb, 10);

    if (*verb != ':')
    {
	warn("missing colon: %s", cmd);
	return;
    }
    ++verb; /* skip colon */

    for (q = verb; *q; q++)
    {
	if (*q == '!')
	{
	    *q++ = NUL;
	    isfunc = 0;
	    break;
	}
	else if (*q == '/')
	{
	    *q++ = NUL;
	    isfunc = 1;
	    break;
	}
    }

    if (isfunc < 0)
    {
	warn("missing ! or / in: %s", cmd);
	return;
    }

    cmdno = strtol((char *)q, (char **)&q, 10);

    if (nb_do_cmd(bufno, verb, isfunc, cmdno, q) == FAIL)
    {
	nbdebug(("nb_parse_cmd: Command error for \"%s\"\n", cmd));
	die("bad return from nb_do_cmd");
    }
}

struct nbbuf_struct
{
    buf_T		*bufp;
#if 0 /* never used */
    unsigned int	 netbeansOwns:1;
    unsigned int	 fireCaret:1;
#endif
    unsigned int	 fireChanges:1;
    unsigned int	 initDone:1;
    unsigned int	 modified:1;
#if 0  /* never used */
    char		*internalname;
#endif
    char		*displayname;
    char_u		*partial_line;
    int			*signmap;
    ushort		 signmaplen;
    ushort		 signmapused;
};

typedef struct nbbuf_struct nbbuf_T;

static nbbuf_T *buf_list = 0;
int buf_list_size = 0;
int buf_list_used = -1; /* last index in use */

static char **globalsignmap;
static int globalsignmaplen;
static int globalsignmapused;

static int  mapsigntype __ARGS((nbbuf_T *, int localsigntype));
static void addsigntype __ARGS((nbbuf_T *, int localsigntype, char_u *typeName,
			char_u *tooltip, char_u *glyphfile,
			int usefg, int fg, int usebg, int bg));

static int curPCtype = -1;

/*
 * Get the Netbeans buffer number for the specified buffer.
 */
    static int
nb_getbufno(buf_T *bufp)
{
    int i;

    for (i = 0; i <= buf_list_used; i++)
    {
	if (buf_list[i].bufp == bufp)
	    return i;
    }

    return -1;
}

/*
 * Given a Netbeans buffer number, return the netbeans buffer.
 * Returns NULL for 0 or a negative number. A 0 bufno means a
 * non-buffer related command has been sent.
 */
    static nbbuf_T *
nb_get_buf(int bufno)
{
    /* find or create a buffer with the given number */
    int incr;

    if (bufno <= 0)
	return NULL;

    if (!buf_list)
    {
	/* initialize */
	buf_list = (nbbuf_T *)alloc_clear(100 * sizeof(nbbuf_T));
	buf_list_size = 100;
    }
    if (bufno > buf_list_used) /* new */
    {
	if (bufno >= buf_list_size) /* grow list */
	{
	    incr = 100;
	    buf_list_size += incr;
	    buf_list = (nbbuf_T *)vim_realloc(
				buf_list, buf_list_size * sizeof(nbbuf_T));
	    memset(buf_list + buf_list_size - incr, 0, incr * sizeof(nbbuf_T));
	}

	while (buf_list_used < bufno)
	{
	    /* Default is to fire text changes. */
	    buf_list[buf_list_used].fireChanges = 1;
	    ++buf_list_used;
	}
    }

    return buf_list + bufno;
}

/*
 * Return the number of buffers that are modified.
 */
    static int
count_changed_buffers(void)
{
    buf_T	*bufp;
    int		n;

    n = 0;
    for (bufp = firstbuf; bufp != NULL; bufp = bufp->b_next)
	if (bufp->b_changed)
	    ++n;
    return n;
}

/*
 * End the netbeans session.
 */
    void
netbeans_end(void)
{
    int i;
    static char buf[128];

    if (!haveConnection)
	return;

    for (i = 0; i <= buf_list_used; i++)
    {
	if (!buf_list[i].bufp)
	    continue;
	if (netbeansForcedQuit)
	{
	    /* mark as unmodified so NetBeans won't put up dialog on "killed" */
	    sprintf(buf, "%d:unmodified=%d\n", i, cmdno);
	    nbdebug(("EVT: %s", buf));
	    nb_send(buf, "netbeans_end");
	}
	sprintf(buf, "%d:killed=%d\n", i, cmdno);
	nbdebug(("EVT: %s", buf));
/*	nb_send(buf, "netbeans_end");    avoid "write failed" messages */
	if (sd >= 0)
	    write(sd, buf, STRLEN(buf));  /* ignore errors */
    }

    /* Give NetBeans a chance to write some clean-up cmds to the socket before
     * we close the connection.  Other clients may set the delay to zero. */
    if (exit_delay > 0)
	sleep(exit_delay);
}

/*
 * Send a message to netbeans.
 */
    static void
nb_send(char *buf, char *fun)
{
    if (sd < 0)
	warn("%s(): write while not connected", fun);
    else if (write(sd, buf, STRLEN(buf)) != STRLEN(buf))
	warn("%s(): write failed", fun);
}

/*
 * Some input received from netbeans requires a response. This function
 * handles a response with no information (except the command number).
 */
    static void
nb_reply_nil(int cmdno)
{
    char reply[32];

    if (!haveConnection)
	return;

    sprintf(reply, "%d\n", cmdno);

    nbdebug(("    REPLY: %s", reply));

    nb_send(reply, "nb_reply_nil");
}


/*
 * Send a response with text.
 */
    static void
nb_reply_text(int cmdno, char_u *result)
{
    char_u *reply;

    if (!haveConnection)
	return;

    reply = alloc(STRLEN(result) + 32);
    sprintf((char *)reply, "%d %s\n", cmdno, (char *)result);

    nbdebug(("    REPLY: %s", reply));
    nb_send((char *)reply, "nb_reply_text");

    vim_free(reply);
}


/*
 * Send a response with a number result code.
 */
    static void
nb_reply_nr(int cmdno, long result)
{
    char reply[32];

    if (!haveConnection)
	return;

    sprintf(reply, "%d %ld\n", cmdno, result);

    nbdebug(("REPLY: %s", reply));

    nb_send(reply, "nb_reply_nr");
}


/*
 * Encode newline, ret, backslash, double quote for transmission to NetBeans.
 */
    static char_u *
nb_quote(char_u *txt)
{
    char_u *buf = alloc(2 * STRLEN(txt) + 1);
    char_u *p = txt;
    char_u *q = buf;

    for (; *p; p++)
    {
	switch (*p)
	{
	    case '\"':
	    case '\\':
		*q++ = '\\'; *q++ = *p; break;
	 /* case '\t': */
	 /*     *q++ = '\\'; *q++ = 't'; break; */
	    case '\n':
		*q++ = '\\'; *q++ = 'n'; break;
	    case '\r':
		*q++ = '\\'; *q++ = 'r'; break;
	    default:
		*q++ = *p;
		break;
	}
    }
    *q++ = '\0';

    return buf;
}


/*
 * Remove top level double quotes; convert backslashed chars.
 * Returns an allocated string (NULL for failure).
 * If "endp" is not NULL it is set to the character after the terminating
 * quote.
 */
    static char *
nb_unquote(char_u *p, char_u **endp)
{
    char *result = 0;
    char *q;
    int done = 0;

    /* result is never longer than input */
    result = (char *)alloc_clear(STRLEN(p) + 1);
    if (result == NULL)
	return NULL;

    if (*p++ != '"')
    {
	nbdebug(("nb_unquote called with string that doesn't start with a quote!: %s", p));
	result[0] = NUL;
	return result;
    }

    for (q = result; !done && *p != NUL;)
    {
	switch (*p)
	{
	    case '"':
		/*
		 * Unbackslashed dquote marks the end, if first char was dquote.
		 */
		done = 1;
		break;

	    case '\\':
		++p;
		switch (*p)
		{
		    case '\\':	*q++ = '\\';	break;
		    case 'n':	*q++ = '\n';	break;
		    case 't':	*q++ = '\t';	break;
		    case 'r':	*q++ = '\r';	break;
		    case '"':	*q++ = '"';	break;
		}
		++p;
		break;

	    default:
		*q++ = *p++;
	}
    }

    if (endp != NULL)
	*endp = p;

    return result;
}

#define SKIP_STOP 2
#define streq(a,b) (strcmp(a,b) == 0)
static int needupdate = 0;
static int inAtomic = 0;

/*
 * Do the actual processing of a single netbeans command or function.
 * The differance between a command and function is that a function
 * gets a response (its required) but a command does not.
 * For arguments see comment for nb_parse_cmd().
 */
    static int
nb_do_cmd(
    int		bufno,
    char_u	*cmd,
    int		func,
    int		cmdno,
    char_u	*args)	    /* points to space before arguments or NUL */
{
    int		doupdate = 0;
    long	off = 0;
    nbbuf_T	*buf = nb_get_buf(bufno);
    static int	skip = 0;
    int		retval = OK;

    nbdebug(("%s %d: (%d) %s %s\n", (func) ? "FUN" : "CMD", cmdno, bufno, cmd,
	STRCMP(cmd, "insert") == 0 ? "<text>" : (char *)args));

    if (func)
    {
/* =====================================================================*/
	if (streq((char *)cmd, "getModified"))
	{
	    if (buf == NULL || buf->bufp == NULL)
		/* Return the number of buffers that are modified. */
		nb_reply_nr(cmdno, (long)count_changed_buffers());
	    else
		/* Return whether the buffer is modified. */
		nb_reply_nr(cmdno, (long)buf->bufp->b_changed);
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "saveAndExit"))
	{
	    /* Note: this will exit Vim if successful. */
	    coloncmd(":confirm qall");

	    /* We didn't exit: return the number of changed buffers. */
	    nb_reply_nr(cmdno, (long)count_changed_buffers());
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "getCursor"))
	{
	    char_u text[200];

	    /* Note: nb_getbufno() may return -1.  This indicates the IDE
	     * didn't assign a number to the current buffer in response to a
	     * fileOpened event. */
	    sprintf((char *)text, "%d %ld %d %ld",
		    nb_getbufno(curbuf),
		    (long)curwin->w_cursor.lnum,
		    (int)curwin->w_cursor.col,
		    pos2off(curbuf, &curwin->w_cursor));
	    nb_reply_text(cmdno, text);
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "getLength"))
	{
	    long len = 0;

	    if (buf == NULL || buf->bufp == NULL)
	    {
		nbdebug(("    null bufp in getLength"));
		die("null bufp in getLength");
		retval = FAIL;
	    }
	    else
	    {
		len = get_buf_size(buf->bufp);
		/* adjust for a partial last line */
		if (buf->partial_line != NULL)
		{
		    nbdebug(("    Adjusting buffer len for partial last line: %d\n",
				STRLEN(buf->partial_line)));
		    len += STRLEN(buf->partial_line);
		}
	    }
	    nb_reply_nr(cmdno, len);
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "getText"))
	{
	    long	len;
	    linenr_T	nlines;
	    char_u	*text = NULL;
	    linenr_T	lno = 1;
	    char_u	*p;
	    char_u	*line;

	    if (buf == NULL || buf->bufp == NULL)
	    {
		nbdebug(("    null bufp in getText"));
		die("null bufp in getText");
		retval = FAIL;
	    }
	    else
	    {
		len = get_buf_size(buf->bufp);
		nlines = buf->bufp->b_ml.ml_line_count;
		text = alloc((unsigned)((len > 0)
						 ? ((len + nlines) * 2) : 4));
		if (text == NULL)
		{
		    nbdebug(("    nb_do_cmd: getText has null text field\n"));
		    retval = FAIL;
		}
		else
		{
		    p = text;
		    *p++ = '\"';
		    for (; lno <= nlines ; lno++)
		    {
			line = nb_quote(ml_get_buf(buf->bufp, lno, FALSE));
			if (line != NULL)
			{
			    STRCPY(p, line);
			    p += STRLEN(line);
			    *p++ = '\\';
			    *p++ = 'n';
			}
			vim_free(line);
		    }
		    *p++ = '\"';
		    *p = '\0';
		}
	    }
	    if (text == NULL)
		nb_reply_text(cmdno, (char_u *)"");
	    else
	    {
		nb_reply_text(cmdno, text);
		vim_free(text);
	    }
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "remove"))
	{
	    long count;
	    pos_T first, last;
	    pos_T *pos;
	    int oldFire = netbeansFireChanges;
	    int oldSuppress = netbeansSuppressNoLines;
	    int wasChanged;

	    if (skip >= SKIP_STOP)
	    {
		nbdebug(("    Skipping %s command\n", (char *) cmd));
		nb_reply_nil(cmdno);
		return OK;
	    }

	    if (buf == NULL || buf->bufp == NULL)
	    {
		nbdebug(("    null bufp in remove"));
		die("null bufp in remove");
		retval = FAIL;
	    }
	    else
	    {
		netbeansFireChanges = FALSE;
		netbeansSuppressNoLines = TRUE;

		if (curbuf != buf->bufp)
		    set_curbuf(buf->bufp, DOBUF_GOTO);
		wasChanged = buf->bufp->b_changed;
		off = strtol((char *)args, (char **)&args, 10);
		count = strtol((char *)args, (char **)&args, 10);
		/* delete "count" chars, starting at "off" */
		pos = off2pos(buf->bufp, off);
		if (!pos)
		{
		    nb_reply_text(cmdno, (char_u *)"!bad position");
		    netbeansFireChanges = oldFire;
		    netbeansSuppressNoLines = oldSuppress;
		    return FAIL;
		}
		first = *pos;
		nbdebug(("    FIRST POS: line %d, col %d\n", first.lnum, first.col));
		pos = off2pos(buf->bufp, off+count-1);
		if (!pos)
		{
		    nb_reply_text(cmdno, (char_u *)"!bad count");
		    netbeansFireChanges = oldFire;
		    netbeansSuppressNoLines = oldSuppress;
		    return FAIL;
		}
		last = *pos;
		nbdebug(("    LAST POS: line %d, col %d\n", last.lnum, last.col));
		curwin->w_cursor = first;
		doupdate = 1;

		/* keep part of first line */
		if (first.lnum == last.lnum && first.col != last.col)
		{
		    /* deletion is within one line */
		    char_u *p = ml_get(first.lnum);
		    mch_memmove(p + first.col, p + last.col + 1, STRLEN(p + last.col) + 1);
		    nbdebug(("    NEW LINE %d: %s\n", first.lnum, p));
		    ml_replace(first.lnum, p, TRUE);
		}

		if (first.lnum < last.lnum)
		{
		    int i;

		    /* delete signs from the lines being deleted */
		    for (i = first.lnum; i <= last.lnum; i++)
		    {
			int id = buf_findsign_id(buf->bufp, (linenr_T)i);
			if (id > 0)
			{
			    nbdebug(("    Deleting sign %d on line %d\n", id, i));
			    buf_delsign(buf->bufp, id);
			}
			else
			    nbdebug(("    No sign on line %d\n", i));
		    }

		    /* delete whole lines */
		    nbdebug(("    Deleting lines %d through %d\n", first.lnum, last.lnum));
		    del_lines(last.lnum - first.lnum + 1, FALSE);
		}
		buf->bufp->b_changed = wasChanged; /* logically unchanged */
		netbeansFireChanges = oldFire;
		netbeansSuppressNoLines = oldSuppress;

		u_blockfree(buf->bufp);
		u_clearall(buf->bufp);
	    }
	    nb_reply_nil(cmdno);
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "insert"))
	{
	    pos_T	*pos;
	    char_u	*to_free;
	    char_u	*nl;
	    int		lnum;
	    pos_T	old_w_cursor;
	    int		old_b_changed;

	    if (skip >= SKIP_STOP)
	    {
		nbdebug(("    Skipping %s command\n", (char *) cmd));
		nb_reply_nil(cmdno);
		return OK;
	    }

	    /* get offset */
	    off = strtol((char *)args, (char **)&args, 10);

	    /* get text to be inserted */
	    ++args; /* skip space */
	    args = to_free = (char_u *)nb_unquote(args, NULL);

	    if (buf == NULL || buf->bufp == NULL)
	    {
		nbdebug(("    null bufp in insert"));
		die("null bufp in insert");
		retval = FAIL;
	    }
	    else if (args != NULL)
	    {
		oldFire = netbeansFireChanges;
		netbeansFireChanges = 0;

		if (curbuf != buf->bufp)
		    set_curbuf(buf->bufp, DOBUF_GOTO);
		old_b_changed = buf->bufp->b_changed;

		if (buf->partial_line != NULL)
		{
		    nbdebug(("    Combining with partial line\n"));
		    off -= STRLEN(buf->partial_line);
		    pos = off2pos(buf->bufp, off);
		    if (pos && pos->col != 0)
			off -= pos->col;  /* want start of line */
		    buf->partial_line = vim_realloc(buf->partial_line,
				STRLEN(buf->partial_line) + STRLEN(args) + 1);
		    STRCAT(buf->partial_line, args);
		    vim_free(to_free);
		    args = buf->partial_line;
		    buf->partial_line = NULL;
		    to_free = args;
		}
		pos = off2pos(buf->bufp, off);
		if (pos)
		{
		    if (pos->lnum == 0)
			pos->lnum = 1;
		    nbdebug(("    POSITION: line = %d, col = %d\n",
							pos->lnum, pos->col));
		}
		else
		{
		    /* if the given position is not found, assume we want
		     * the end of the file.  See setLocAndSize HACK.
		     */
		    pos->lnum = buf->bufp->b_ml.ml_line_count;
		    nbdebug(("    POSITION: line = %d (EOF)\n",pos->lnum));
		}
		lnum = pos->lnum;
		old_w_cursor = curwin->w_cursor;
		curwin->w_cursor = *pos;

		doupdate = 1;
		while (*args)
		{
		    nl = (char_u *)strchr((char *)args, '\n');
		    if (!nl)
		    {
			nbdebug(("    PARTIAL[%d]: %s\n", lnum, args));
			break;
		    }
		    *nl = '\0';
		    nbdebug(("    INSERT[%d]: %s\n", lnum, args));
		    ml_append((linenr_T)(lnum++ - 1), args,
						     STRLEN(args) + 1, FALSE);
		    args = nl + 1;
		}
		appended_lines_mark(pos->lnum - 1, lnum - pos->lnum);

		if (*args)
		{
		    /*
		     * Incomplete line, squirrel away and wait for next insert.
		     */
		    nbdebug(("    PARTIAL-SAVED: %s\n", args));
		    buf->partial_line = vim_realloc(buf->partial_line,
							    STRLEN(args) + 1);
		    STRCPY(buf->partial_line, args);
		}
		curwin->w_cursor = old_w_cursor;

		/*
		 * XXX - GRP - Is the next line right? If I've inserted
		 * text the buffer has been updated but not written. Will
		 * netbeans guarantee to write it? Even if I do a :q! ?
		 */
		buf->bufp->b_changed = old_b_changed; /* logically unchanged */
		netbeansFireChanges = oldFire;

		u_blockfree(buf->bufp);
		u_clearall(buf->bufp);
	    }
	    vim_free(to_free);
	    nb_reply_nil(cmdno); /* or !error */
	}
	else
	{
	    nbdebug(("UNIMPLEMENTED FUNCTION: %s\n", cmd));
	    nb_reply_nil(cmdno);
	    retval = FAIL;
	}
    }
    else /* Not a function; no reply required. */
    {
/* =====================================================================*/
	if (streq((char *)cmd, "create"))
	{
	    /* Create a buffer without a name. */
	    if (buf == NULL)
	    {
		die("null buf in create");
		return FAIL;
	    }
	    vim_free(buf->displayname);
	    buf->displayname = NULL;
	    nbdebug(("    CREATE %d\n", bufno));

	    netbeansReadFile = 0; /* don't try to open disk file */
	    do_ecmd(0, NULL, 0, 0, ECMD_ONE, ECMD_HIDE + ECMD_OLDBUF);
	    netbeansReadFile = 1;
	    buf->bufp = curbuf;
	    maketitle();
	    gui_update_menus(0);
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "startDocumentListen"))
	{
	    if (buf == NULL)
	    {
		die("null buf in startDocumentListen");
		return FAIL;
	    }
	    buf->fireChanges = 1;
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "stopDocumentListen"))
	{
	    if (buf == NULL)
	    {
		die("null buf in stopDocumentListen");
		return FAIL;
	    }
	    buf->fireChanges = 0;
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "setTitle"))
	{
	    if (buf == NULL)
	    {
		die("null buf in setTitle");
		return FAIL;
	    }
	    vim_free(buf->displayname);
	    buf->displayname = nb_unquote(++args, NULL);
	    nbdebug(("    SETTITLE %d %s\n", bufno, buf->displayname));
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "initDone"))
	{
	    if (buf == NULL || buf->bufp == NULL)
	    {
		die("null buf in initDone");
		return FAIL;
	    }
	    doupdate = 1;
	    buf->initDone = 1;
	    if (curbuf != buf->bufp)
		set_curbuf(buf->bufp, DOBUF_GOTO);
#if defined(FEAT_AUTOCMD)
	    apply_autocmds(EVENT_BUFREADPOST, 0, 0, FALSE, buf->bufp);
#endif

	    /* handle any postponed key commands */
	    handle_key_queue();
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "setBufferNumber")
		|| streq((char *)cmd, "putBufferNumber"))
	{
	    char_u	*to_free;
	    buf_T	*bufp;

	    if (buf == NULL)
	    {
		die("null buf in setBufferNumber");
		return FAIL;
	    }
	    to_free = (char_u *)nb_unquote(++args, NULL);
	    if (to_free == NULL)
		return FAIL;
	    bufp = buflist_findname(to_free);
	    vim_free(to_free);
	    if (bufp == NULL)
	    {
		warn("File %s not found in setBufferNumber", args);
		return FAIL;
	    }
	    buf->bufp = bufp;

	    /* "setBufferNumber" has the side effect of jumping to the buffer
	     * (don't know why!).  Don't do that for "putBufferNumber". */
	    if (*cmd != 'p')
		coloncmd(":buffer %d", bufp->b_fnum);
	    else
	    {
		buf->initDone = 1;

		/* handle any postponed key commands */
		handle_key_queue();
	    }

#if 0  /* never used */
	    buf->internalname = (char *)alloc_clear(8);
	    sprintf(buf->internalname, "<%d>", bufno);
	    buf->netbeansOwns = 0;
#endif
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "setFullName"))
	{
	    if (buf == NULL)
	    {
		die("null buf in setFullName");
		return FAIL;
	    }
	    vim_free(buf->displayname);
	    buf->displayname = nb_unquote(++args, NULL);
	    nbdebug(("    SETFULLNAME %d %s\n", bufno, buf->displayname));

	    netbeansReadFile = 0; /* don't try to open disk file */
	    do_ecmd(0, (char_u *)buf->displayname, 0, 0, ECMD_ONE,
						     ECMD_HIDE + ECMD_OLDBUF);
	    netbeansReadFile = 1;
	    buf->bufp = curbuf;
	    maketitle();
	    gui_update_menus(0);
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "editFile"))
	{
	    if (buf == NULL)
	    {
		die("null buf in editFile");
		return FAIL;
	    }
	    /* Edit a file: like create + setFullName + read the file. */
	    vim_free(buf->displayname);
	    buf->displayname = nb_unquote(++args, NULL);
	    nbdebug(("    EDITFILE %d %s\n", bufno, buf->displayname));
	    do_ecmd(0, (char_u *)buf->displayname, NULL, NULL, ECMD_ONE,
						     ECMD_HIDE + ECMD_OLDBUF);
	    buf->bufp = curbuf;
	    buf->initDone = 1;
	    doupdate = 1;
#if defined(FEAT_TITLE)
	    maketitle();
#endif
	    gui_update_menus(0);
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "setVisible"))
	{
	    ++args;
	    if (buf == NULL || buf->bufp == NULL)
	    {
/*		warn("null bufp in setVisible"); */
		return FAIL;
	    }
	    if (streq((char *)args, "T"))
	    {
		exarg_T exarg;
		exarg.cmd = (char_u *)"goto";
		exarg.forceit = FALSE;
		goto_buffer(&exarg, DOBUF_FIRST, FORWARD, buf->bufp->b_fnum);
		doupdate = 1;

		/* Side effect!!!. */
		if (!gui.starting)
		    gui_mch_set_foreground();
	    }
	    else
	    {
		/* bury the buffer - not yet */
	    }
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "raise"))
	{
	    /* Bring gvim to the foreground. */
	    if (!gui.starting)
		gui_mch_set_foreground();
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "setModified"))
	{
	    ++args;
	    if (buf == NULL || buf->bufp == NULL)
	    {
/*		warn("null bufp in setModified"); */
		return FAIL;
	    }
	    if (streq((char *)args, "T"))
		buf->bufp->b_changed = 1;
	    else
		buf->bufp->b_changed = 0;
	    buf->modified = buf->bufp->b_changed;
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "setMark"))
	{
	    /* not yet */
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "showBalloon"))
	{
#if defined(FEAT_BEVAL)
	    static char	*text = NULL;

	    /*
	     * Set up the Balloon Expression Evaluation area.
	     * Ignore 'ballooneval' here.
	     * The text pointer must remain valid for a while.
	     */
	    if (balloonEval != NULL)
	    {
		vim_free(text);
		text = nb_unquote(++args, NULL);
		if (text != NULL)
		    gui_mch_post_balloon(balloonEval, (char_u *)text);
	    }
#endif
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "setDot"))
	{
	    pos_T *pos;
#ifdef NBDEBUG
	    char_u *s;
#endif

	    ++args;
	    if (buf == NULL || buf->bufp == NULL)
	    {
		die("null bufp in setDot");
		return FAIL;
	    }

	    if (curbuf != buf->bufp)
		set_curbuf(buf->bufp, DOBUF_GOTO);
#ifdef FEAT_VISUAL
	    /* Don't want Visual mode now. */
	    if (VIsual_active)
		end_visual_mode();
#endif
#ifdef NBDEBUG
	    s = args;
#endif
	    pos = get_off_or_lnum(buf->bufp, &args);
	    if (pos)
	    {
		curwin->w_cursor = *pos;
		check_cursor();
	    }
	    else
		nbdebug(("    BAD POSITION in setDot: %s\n", s));

	    /* gui_update_cursor(TRUE, FALSE); */
	    /* update_curbuf(NOT_VALID); */
	    update_topline();		/* scroll to show the line */
	    update_screen(VALID);
	    setcursor();
	    out_flush();
	    gui_update_cursor(TRUE, FALSE);
	    gui_mch_flush();
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "close"))
	{
#ifdef NBDEBUG
	    char *name = "<NONE>";
#endif

	    if (buf == NULL)
	    {
		die("null buf in close");
		return FAIL;
	    }

#ifdef NBDEBUG
	    if (buf->displayname != NULL)
		name = buf->displayname;
#endif
/*	    if (buf->bufp == NULL) */
/*		die("null bufp in close"); */
	    nbdebug(("    CLOSE %d: %s\n", bufno, name));
	    need_mouse_correct = TRUE;
	    if (buf->bufp != NULL)
		close_buffer(NULL, buf->bufp, 0);
	    doupdate = 1;
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "setStyle")) /* obsolete... */
	{
	    nbdebug(("    setStyle is obsolete!"));
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "setExitDelay"))
	{
	    /* New in version 2.1. */
	    exit_delay = strtol((char *)args, (char **)&args, 10);
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "defineAnnoType"))
	{
#ifdef FEAT_SIGNS
	    int typeNum;
	    char_u *typeName;
	    char_u *tooltip;
	    char_u *glyphFile;
	    int use_fg = 0;
	    int use_bg = 0;
	    int fg = -1;
	    int bg = -1;

	    if (buf == NULL)
	    {
		die("null buf in defineAnnoType");
		return FAIL;
	    }

	    typeNum = strtol((char *)args, (char **)&args, 10);
	    args = skipwhite(args);
	    typeName = (char_u *)nb_unquote(args, &args);
	    args = skipwhite(args + 1);
	    tooltip = (char_u *)nb_unquote(args, &args);
	    args = skipwhite(args + 1);
	    glyphFile = (char_u *)nb_unquote(args, &args);
	    args = skipwhite(args + 1);
	    if (STRNCMP(args, "none", 4) == 0)
		args += 5;
	    else
	    {
		use_fg = 1;
		fg = strtol((char *)args, (char **)&args, 10);
	    }
	    if (STRNCMP(args, "none", 4) == 0)
		args += 5;
	    else
	    {
		use_bg = 1;
		bg = strtol((char *)args, (char **)&args, 10);
	    }
	    if (typeName != NULL && tooltip != NULL && glyphFile != NULL)
		addsigntype(buf, typeNum, typeName, tooltip, glyphFile,
						      use_fg, fg, use_bg, bg);
	    else
		vim_free(typeName);

	    /* don't free typeName; it's used directly in addsigntype() */
	    vim_free(tooltip);
	    vim_free(glyphFile);

#endif
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "addAnno"))
	{
#ifdef FEAT_SIGNS
	    int serNum;
	    int localTypeNum;
	    int typeNum;
# ifdef NBDEBUG
	    int len;
# endif
	    pos_T *pos;

	    if (buf == NULL || buf->bufp == NULL)
	    {
		die("null bufp in addAnno");
		return FAIL;
	    }

	    doupdate = 1;

	    serNum = strtol((char *)args, (char **)&args, 10);

	    /* Get the typenr specific for this buffer and convert it to
	     * the global typenumber, as used for the sign name. */
	    localTypeNum = strtol((char *)args, (char **)&args, 10);
	    typeNum = mapsigntype(buf, localTypeNum);

	    pos = get_off_or_lnum(buf->bufp, &args);

# ifdef NBDEBUG
	    len =
# endif
		strtol((char *)args, (char **)&args, 10);
# ifdef NBDEBUG
	    if (len != -1)
	    {
		nbdebug(("    partial line annotation -- Not Yet Implemented!"));
	    }
# endif
	    if (serNum >= GUARDEDOFFSET)
	    {
		nbdebug(("    too many annotations! ignoring..."));
		return FAIL;
	    }
	    if (pos)
	    {
		coloncmd(":sign place %d line=%d name=%d buffer=%d",
			   serNum, pos->lnum, typeNum, buf->bufp->b_fnum);
		if (typeNum == curPCtype)
		    coloncmd(":sign jump %d buffer=%d", serNum,
						       buf->bufp->b_fnum);
	    }
	    /* XXX only redraw what changed. */
	    redraw_later(CLEAR);
#endif
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "removeAnno"))
	{
#ifdef FEAT_SIGNS
	    int serNum;

	    if (buf == NULL || buf->bufp == NULL)
	    {
		nbdebug(("    null bufp in removeAnno"));
		return FAIL;
	    }
	    doupdate = 1;
	    serNum = strtol((char *)args, (char **)&args, 10);
	    coloncmd(":sign unplace %d buffer=%d",
		     serNum, buf->bufp->b_fnum);
	    redraw_buf_later(buf->bufp, NOT_VALID);
#endif
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "moveAnnoToFront"))
	{
#ifdef FEAT_SIGNS
	    nbdebug(("    moveAnnoToFront: Not Yet Implemented!"));
#endif
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "guard") || streq((char *)cmd, "unguard"))
	{
	    int len;
	    pos_T first;
	    pos_T last;
	    pos_T *pos;
	    int un = (cmd[0] == 'u');
	    static int guardId = GUARDEDOFFSET;

	    if (skip >= SKIP_STOP)
	    {
		nbdebug(("    Skipping %s command\n", (char *) cmd));
		return OK;
	    }

	    nb_init_graphics();

	    if (buf == NULL || buf->bufp == NULL)
	    {
		nbdebug(("    null bufp in %s command", cmd));
		return FAIL;
	    }
	    if (curbuf != buf->bufp)
		set_curbuf(buf->bufp, DOBUF_GOTO);
	    off = strtol((char *)args, (char **)&args, 10);
	    len = strtol((char *)args, 0, 10);
	    pos = off2pos(buf->bufp, off);
	    doupdate = 1;
	    if (!pos)
		nbdebug(("    no such start pos in %s, %ld\n", cmd, off));
	    else
	    {
		first = *pos;
		pos = off2pos(buf->bufp, off + len - 1);
		if (pos != NULL && pos->col == 0) {
			/*
			 * In Java Swing the offset is a position between 2
			 * characters. If col == 0 then we really want the
			 * previous line as the end.
			 */
			pos = off2pos(buf->bufp, off + len - 2);
		}
		if (!pos)
		    nbdebug(("    no such end pos in %s, %ld\n",
			    cmd, off + len - 1));
		else
		{
		    long lnum;
		    last = *pos;
		    /* set highlight for region */
		    nbdebug(("    %sGUARD %ld,%d to %ld,%d\n", (un) ? "UN" : "",
			     first.lnum, first.col,
			     last.lnum, last.col));
#ifdef FEAT_SIGNS
		    for (lnum = first.lnum; lnum <= last.lnum; lnum++)
		    {
			if (un)
			{
			    /* never used */
			}
			else
			{
			    if (buf_findsigntype_id(buf->bufp, lnum,
				GUARDED) == 0)
			    {
				coloncmd(
				    ":sign place %d line=%d name=%d buffer=%d",
				     guardId++, lnum, GUARDED,
				     buf->bufp->b_fnum);
			    }
			}
		    }
#endif
		    redraw_buf_later(buf->bufp, NOT_VALID);
		}
	    }
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "startAtomic"))
	{
	    inAtomic = 1;
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "endAtomic"))
	{
	    inAtomic = 0;
	    if (needupdate)
	    {
		doupdate = 1;
		needupdate = 0;
	    }
/* =====================================================================*/
	}
	else if (streq((char *)cmd, "version"))
	{
	    nbdebug(("    Version = %s\n", (char *) args));
	}
	/*
	 * Unrecognized command is ignored.
	 */
    }
    if (inAtomic && doupdate)
    {
	needupdate = 1;
	doupdate = 0;
    }

    if (buf != NULL && buf->initDone && doupdate)
    {
	update_screen(NOT_VALID);
	setcursor();
	out_flush();
	gui_update_cursor(TRUE, FALSE);
	gui_mch_flush();
    }

    return retval;
}


/*
 * Process a vim colon command.
 */
    static void
coloncmd(char *cmd, ...)
{
    char buf[1024];
    va_list ap;

    va_start(ap, cmd);
    vsprintf(buf, cmd, ap);
    va_end(ap);

    nbdebug(("    COLONCMD %s\n", buf));

/*     ALT_INPUT_LOCK_ON; */
    do_cmdline((char_u *)buf, NULL, NULL, DOCMD_NOWAIT | DOCMD_KEYTYPED);
/*     ALT_INPUT_LOCK_OFF; */

    setcursor();		/* restore the cursor position */
    out_flush();		/* make sure output has been written */

    gui_update_cursor(TRUE, FALSE);
    gui_mch_flush();
}

#ifdef HAVE_READLINK

/*
 * Check symlinks for infinite recursion.
 * "level" is for recursion control.
 */
    static void
resolve_symlinks(char *filename, int level)
{
    struct stat sbuf;

    if ((level > 0) && (lstat(filename, &sbuf) == 0) && (S_ISLNK(sbuf.st_mode)))
    {
	char buf[MAXPATHLEN+1];
	int len = readlink(filename, buf, MAXPATHLEN);

	if (len < 0 || len == MAXPATHLEN)
	{
	    die("readlink() failed, errno = %d\n", errno);
	}
	else
	{
	    buf[len] = '\0';

	    if (buf[0] == '/')
	    {
		/* link value is absolute */
		strcpy(filename, buf);
	    }
	    else
	    {
		/* link is relative */
		char *p = strrchr(filename, '/');

		if (p == 0)
		    die("missing slash!?!");
		else
		    if ((p - filename) + strlen(buf) > MAXPATHLEN)
			die("buffer overflow in resolve_symlinks()");
		    else
			strcpy(p+1, buf);
	    }

	    /* check for symlinks in resulting path */
	    resolve_symlinks(filename, level-1);
	}
    }
}

#endif /* HAVE_READLINK */

static char *rundir = "";

/*
 * Set rundir -- Dynamically find VIMRUNTIME dir
 */
    void
netbeans_setRunDir(char *argv0)
{
    char	fullpath[MAXPATHLEN];
    char	*p;
    static char buf[MAXPATHLEN];

    if (*argv0 == '/')
	strcpy(fullpath, argv0);
    else if (strchr(argv0, '/'))
    {
	getcwd(fullpath, MAXPATHLEN);
	strcat(fullpath, "/");
	strcat(fullpath, argv0);
    }
    else /* no slash, have to search path */
    {
	char *path = getenv("PATH");
	if (path)
	{
	    char *pathbuf = (char *)vim_strsave((char_u *)path);
	    path = strtok(pathbuf, ":");
	    do
	    {
		strcpy(fullpath, path);
		strcat(fullpath, "/");
		strcat(fullpath, argv0);
		if (access(fullpath, X_OK) == 0)
		    break;
		else
		    fullpath[0] = NUL;
	    } while ((path=strtok(NULL, ":")) != NULL);
	    vim_free(pathbuf);
	}
    }

#ifdef HAVE_READLINK
    /* resolve symlinks to get "real" base dir */
    resolve_symlinks(fullpath, 1000);
#endif /* HAVE_READLINK */

    /* search backwards for "bin" or "src" dir in fullpath */

    if (fullpath[0] != NUL)
    {
	p = strrchr(fullpath, '/');
	while (p)
	{
	    if (strncmp(p, "/bin", 4) == 0 || strncmp(p, "/src", 4) == 0)
	    {
		/* vim is in /.../bin  or /.../src */
		rundir = (char *)vim_strsave((char_u *)fullpath);
		break;
	    }
	    *p = NUL;
	    p = strrchr(fullpath, '/');
	}
    }

    /* now find "doc" dir from the rundir (if $VIMRUNTIME is not set) */

    if ((p = getenv("VIMRUNTIME")) != NULL && *p != NUL)
	return;

    strcpy(buf, rundir);
    strcat(buf, "/../share/vim/");
    strcat(buf, "vim61/doc");
    if (access(buf, R_OK) < 0)
    {
	strcpy(buf, rundir);
	strcat(buf, "/../runtime/doc");
	if (access(buf, R_OK) < 0)
	{
	    /* not found! */
	    return;
	}
	else
	{
	    strcpy(buf, rundir);
	    strcat(buf, "/../runtime");
	}
    }
    else
    {
	strcpy(buf, rundir);
	strcat(buf, "/../share/vim/vim61");
    }
    default_vimruntime_dir = (char_u *)buf;
}

/*
 * Initialize highlights and signs for use by netbeans  (mostly obsolete)
 */
    static void
nb_init_graphics(void)
{
    static int did_init = FALSE;

    if (!did_init)
    {
	coloncmd(":highlight NBGuarded guibg=Cyan guifg=Black");
	coloncmd(":sign define %d linehl=NBGuarded", GUARDED);

	did_init = TRUE;
    }
}

/*
 * Convert key to netbeans name.
 */
    static void
netbeans_keyname(int key, char *buf)
{
    char *name = 0;
    char namebuf[2];
    int ctrl  = 0;
    int shift = 0;
    int alt   = 0;

    if (mod_mask & MOD_MASK_CTRL)
	ctrl = 1;
    if (mod_mask & MOD_MASK_SHIFT)
	shift = 1;
    if (mod_mask & MOD_MASK_ALT)
	alt = 1;


    switch (key)
    {
	case K_F1:		name = "F1";		break;
	case K_S_F1:	name = "F1";	shift = 1;	break;
	case K_F2:		name = "F2";		break;
	case K_S_F2:	name = "F2";	shift = 1;	break;
	case K_F3:		name = "F3";		break;
	case K_S_F3:	name = "F3";	shift = 1;	break;
	case K_F4:		name = "F4";		break;
	case K_S_F4:	name = "F4";	shift = 1;	break;
	case K_F5:		name = "F5";		break;
	case K_S_F5:	name = "F5";	shift = 1;	break;
	case K_F6:		name = "F6";		break;
	case K_S_F6:	name = "F6";	shift = 1;	break;
	case K_F7:		name = "F7";		break;
	case K_S_F7:	name = "F7";	shift = 1;	break;
	case K_F8:		name = "F8";		break;
	case K_S_F8:	name = "F8";	shift = 1;	break;
	case K_F9:		name = "F9";		break;
	case K_S_F9:	name = "F9";	shift = 1;	break;
	case K_F10:		name = "F10";		break;
	case K_S_F10:	name = "F10";	shift = 1;	break;
	case K_F11:		name = "F11";		break;
	case K_S_F11:	name = "F11";	shift = 1;	break;
	case K_F12:		name = "F12";		break;
	case K_S_F12:	name = "F12";	shift = 1;	break;
	default:
			if (key >= ' ' && key <= '~')
			{
			    /* Allow ASCII characters. */
			    name = namebuf;
			    namebuf[0] = key;
			    namebuf[1] = NUL;
			}
			else
			    name = "X";
			break;
    }

    buf[0] = '\0';
    if (ctrl)
	strcat(buf, "C");
    if (shift)
	strcat(buf, "S");
    if (alt)
	strcat(buf, "M"); /* META */
    if (ctrl || shift || alt)
	strcat(buf, "-");
    strcat(buf, name);
}

#ifdef FEAT_BEVAL
/*
 * Function to be called for balloon evaluation.  Grabs the text under the
 * cursor and sends it to the debugger for evaluation.  The debugger should
 * respond with a showBalloon command when there is a useful result.
 */
/*ARGSUSED*/
    static void
netbeans_beval_cb(
	BalloonEval	*beval,
	int		 state)
{
    char_u	*filename;
    char_u	*text;
    int		line;
    int		col;
    char	buf[MAXPATHLEN * 2 + 25];
    char_u	*p;

    /* Don't do anything when 'ballooneval' is off, messages scrolled the
     * windows up or we have no connection. */
    if (!p_beval || msg_scrolled > 0 || !haveConnection)
	return;

    if (gui_mch_get_beval_info(beval, &filename, &line, &text, &col) == OK)
    {
	/* Send debugger request.  Only when the text is of reasonable
	 * length. */
	if (text != NULL && text[0] != NUL && STRLEN(text) < MAXPATHLEN)
	{
	    p = nb_quote(text);
	    if (p != NULL)
		sprintf(buf, "0:balloonText=%d \"%s\"\n", cmdno, p);
	    vim_free(p);
	    nbdebug(("EVT: %s", buf));
	    nb_send(buf, "netbeans_beval_cb");
	}
	vim_free(text);
    }
}
#endif

/*
 * Tell netbeans that the window was opened, ready for commands.
 */
    void
netbeans_startup_done(void)
{
    char *cmd = "0:startupDone=0\n";

    if (!haveConnection)
	return;

    nbdebug(("EVT: %s", cmd));
    nb_send(cmd, "netbeans_startup_done");

# if defined(FEAT_BEVAL) && defined(FEAT_GUI_MOTIF)
    if (gui.in_use)
    {
	/*
	 * Set up the Balloon Expression Evaluation area for Motif.
	 * GTK can do it earlier...
	 * Always create it but disable it when 'ballooneval' isn't set.
	 */
	balloonEval = gui_mch_create_beval_area(textArea, NULL,
						    &netbeans_beval_cb, NULL);
	if (!p_beval)
	    gui_mch_disable_beval_area(balloonEval);
    }
# endif
}

#if defined(FEAT_GUI_MOTIF) || defined(PROTO)
/*
 * Tell netbeans that the window was moved or resized.
 */
    void
netbeans_frame_moved(int new_x, int new_y)
{
    char buf[128];

    if (!haveConnection)
	return;

    sprintf(buf, "0:geometry=%d %d %d %d %d\n",
		    cmdno, (int)Columns, (int)Rows, new_x, new_y);
    nbdebug(("EVT: %s", buf));
    nb_send(buf, "netbeans_frame_moved");
}
#endif

/*
 * Tell netbeans the user opened a file.
 */
    void
netbeans_file_opened(char *filename)
{
    char buffer[2*MAXPATHLEN];

    if (!haveConnection)
	return;

    sprintf(buffer, "0:fileOpened=%d \"%s\" %s %s\n",
	    0,
	    filename,
	    "F",  /* open in NetBeans */
	    "F"); /* modified */

    nbdebug(("EVT: %s", buffer));

    nb_send(buffer, "netbeans_file_opened");
    if (p_acd && vim_chdirfile((char_u *)filename) == OK)
	shorten_fnames(TRUE);
}

/*
 * Tell netbeans a file was closed.
 */
    void
netbeans_file_closed(buf_T *bufp)
{
    int		bufno = nb_getbufno(bufp);
    nbbuf_T	*nbbuf = nb_get_buf(bufno);
    char	buffer[2*MAXPATHLEN];

    if (!haveConnection)
	return;

    if (!netbeansCloseFile)
    {
	nbdebug(("ignoring file_closed for %s\n", bufp->b_ffname));
	return;
    }

    nbdebug(("netbeans_file_closed() bufno =  %d, file = %s, displayname = %s\n",
		bufno, bufp->b_ffname,
		(nbbuf != NULL) ? nbbuf->displayname : "<>"));

    if (bufno <= 0)
	return;

    sprintf(buffer, "%d:killed=%d\n", bufno, cmdno);

    nbdebug(("EVT: %s", buffer));

    nb_send(buffer, "netbeans_file_closed");

    if (nbbuf != NULL)
	nbbuf->bufp = NULL;
}

/*
 * Get a pointer to the Netbeans buffer for Vim buffer "bufp".
 * Return NULL if there is no such buffer or changes are not to be reported.
 * Otherwise store the buffer number in "*bufnop".
 */
    static nbbuf_T *
nb_bufp2nbbuf_fire(buf_T *bufp, int *bufnop)
{
    int		bufno;
    nbbuf_T	*nbbuf;

    if (!haveConnection || !netbeansFireChanges)
	return NULL;		/* changes are not reported at all */

    bufno = nb_getbufno(bufp);
    if (bufno <= 0)
	return NULL;		/* file is not known to NetBeans */

    nbbuf = nb_get_buf(bufno);
    if (nbbuf != NULL && !nbbuf->fireChanges)
	return NULL;		/* changes in this buffer are not reported */

    *bufnop = bufno;
    return nbbuf;
}

/*
 * Tell netbeans the user inserted some text.
 */
    void
netbeans_inserted(
    buf_T	*bufp,
    linenr_T	linenr,
    colnr_T	col,
    int		oldlen,
    char_u	*txt,
    int		newlen)
{
    char_u	*buf;
    int		bufno;
    nbbuf_T	*nbbuf;
    pos_T	pos;
    long	off;
    char_u	*p;
    char_u	*newtxt;

    nbbuf = nb_bufp2nbbuf_fire(bufp, &bufno);
    if (nbbuf == NULL)
	return;

    nbbuf->modified = 1;

    pos.lnum = linenr;
    pos.col = col;

    off = pos2off(bufp, &pos);

/*     nbdebug(("linenr = %d, col = %d, off = %ld\n", linenr, col, off)); */

    buf = alloc(128 + 2*newlen);

    if (oldlen > 0)
    {
	/* some chars were replaced; send "remove" EVT */
	sprintf((char *)buf, "%d:remove=%d %ld %d\n",
						   bufno, cmdno, off, oldlen);
	nbdebug(("EVT: %s", buf));
	nb_send((char *)buf, "netbeans_inserted");
    }
    else if (oldlen < 0)
    {
	/* can't happen? */
	nbdebug(("unexpected: oldlen < 0 in netbeans_inserted"));
    }

    /* send the "insert" EVT */
    newtxt = alloc(newlen + 1);
    STRNCPY(newtxt, txt, newlen);
    newtxt[newlen] = '\0';
    p = nb_quote(newtxt);
    if (p != NULL)
	sprintf((char *)buf, "%d:insert=%d %ld \"%s\"\n", bufno, cmdno, off, p);
    vim_free(p);
    vim_free(newtxt);
    nbdebug(("EVT: %s", buf));
    nb_send((char *)buf, "netbeans_inserted");
    vim_free(buf);
}

/*
 * Tell netbeans some bytes have been removed.
 */
    void
netbeans_removed(
    buf_T	*bufp,
    linenr_T	linenr,
    colnr_T	col,
    long	len)
{
    char_u	buf[128];
    int		bufno;
    nbbuf_T	*nbbuf;
    pos_T	pos;
    long	off;

    nbbuf = nb_bufp2nbbuf_fire(bufp, &bufno);
    if (nbbuf == NULL)
	return;

    if (len < 0)
    {
	nbdebug(("Negative len %ld in netbeans_removed()!", len));
	return;
    }

    nbbuf->modified = 1;

    pos.lnum = linenr;
    pos.col = col;

    off = pos2off(bufp, &pos);

    sprintf((char *)buf, "%d:remove=%d %ld %ld\n", bufno, cmdno, off, len);
    nbdebug(("EVT: %s", buf));
    nb_send((char *)buf, "netbeans_removed");
}

/*
 * Send netbeans an unmodufied command.
 */
    void
netbeans_unmodified(buf_T *bufp)
{
    char_u	buf[128];
    int		bufno;
    nbbuf_T	*nbbuf;

    nbbuf = nb_bufp2nbbuf_fire(bufp, &bufno);
    if (nbbuf == NULL)
	return;

    nbbuf->modified = 0;

    sprintf((char *)buf, "%d:unmodified=%d\n", bufno, cmdno);
    nbdebug(("EVT: %s", buf));
    nb_send((char *)buf, "netbeans_unmodified");
}

/*
 * Send a keypress event back to netbeans. This usualy simulates some
 * kind of function key press.
 */
    void
netbeans_keycommand(int key)
{
    char	buf[2*MAXPATHLEN];
    int		bufno;
    char	keyName[60];
    long	off;

    if (!haveConnection)
	return;

    /* convert key to netbeans name */
    netbeans_keyname(key, keyName);

    bufno = nb_getbufno(curbuf);

    if (bufno == -1)
    {
	nbdebug(("got keycommand for non-NetBeans buffer, opening...\n"));
	sprintf(buf, "0:fileOpened=%d \"%s\" %s %s\n", 0, curbuf->b_ffname,
		"T",  /* open in NetBeans */
		"F"); /* modified */
	nbdebug(("EVT: %s", buf));
	nb_send(buf, "netbeans_keycommand");

	postpone_keycommand(key);
	return;
    }

    /* sync the cursor position */
    off = pos2off(curbuf, &curwin->w_cursor);
    sprintf(buf, "%d:newDotAndMark=%d %ld %ld\n", bufno, cmdno, off, off);
    nbdebug(("EVT: %s", buf));
    nb_send(buf, "netbeans_keycommand");

    /* now send keyCommand event */
    sprintf(buf, "%d:keyCommand=%d \"%s\"\n", bufno, cmdno, keyName);
    nbdebug(("EVT: %s", buf));
    nb_send(buf, "netbeans_keycommand");

    /* New: do both at once and include the lnum/col. */
    sprintf(buf, "%d:keyAtPos=%d \"%s\" %ld %ld/%ld\n", bufno, cmdno, keyName,
		off, (long)curwin->w_cursor.lnum, (long)curwin->w_cursor.col);
    nbdebug(("EVT: %s", buf));
    nb_send(buf, "netbeans_keycommand");
}


/*
 * Send a save event to netbeans.
 */
    void
netbeans_saved(buf_T *bufp)
{
    char_u	buf[64];
    int		bufno;
    nbbuf_T	*nbbuf;

    nbbuf = nb_bufp2nbbuf_fire(bufp, &bufno);
    if (nbbuf == NULL)
	return;

    nbbuf->modified = 0;

    sprintf((char *)buf, "%d:save=%d\n", bufno, cmdno);
    nbdebug(("EVT: %s", buf));
    nb_send((char *)buf, "netbeans_saved");
}


/*
 * Send remove command to netbeans (this command has been turned off).
 */
    void
netbeans_deleted_all_lines(buf_T *bufp)
{
    char_u	buf[64];
    int		bufno;
    nbbuf_T	*nbbuf;

    nbbuf = nb_bufp2nbbuf_fire(bufp, &bufno);
    if (nbbuf == NULL)
	return;

    nbbuf->modified = 1;

    sprintf((char *)buf, "%d:remove=%d 0 -1\n", bufno, cmdno);
    nbdebug(("EVT(suppressed): %s", buf));
/*     nb_send(buf, "netbeans_deleted_all_lines"); */
}


/*
 * See if the lines are guarded. The top and bot parameters are from
 * u_savecommon(), these are the line above the change and the line below the
 * change.
 */
    int
netbeans_is_guarded(linenr_T top, linenr_T bot)
{
    signlist_T	*p;
    int		lnum;

    for (p = curbuf->b_signlist; p != NULL; p = p->next)
	if (p->id >= GUARDEDOFFSET)
	    for (lnum = top + 1; lnum < bot; lnum++)
		if (lnum == p->lnum)
		    return TRUE;

    return FALSE;
}

#if defined(FEAT_GUI_MOTIF) || defined(PROTO)
/*
 * We have multiple signs to draw at the same location. Draw the
 * multi-sign indicator instead. This is the Motif version.
 */
    void
netbeans_draw_multisign_indicator(int row)
{
    int i;
    int y;
    int x;

    x = 0;
    y = row * gui.char_height + 2;

    for (i = 0; i < gui.char_height - 3; i++)
	XDrawPoint(gui.dpy, gui.wid, gui.text_gc, x+2, y++);

    XDrawPoint(gui.dpy, gui.wid, gui.text_gc, x+0, y);
    XDrawPoint(gui.dpy, gui.wid, gui.text_gc, x+2, y);
    XDrawPoint(gui.dpy, gui.wid, gui.text_gc, x+4, y++);
    XDrawPoint(gui.dpy, gui.wid, gui.text_gc, x+1, y);
    XDrawPoint(gui.dpy, gui.wid, gui.text_gc, x+2, y);
    XDrawPoint(gui.dpy, gui.wid, gui.text_gc, x+3, y++);
    XDrawPoint(gui.dpy, gui.wid, gui.text_gc, x+2, y);
}
#endif /* FEAT_GUI_MOTIF */

#ifdef FEAT_GUI_GTK
/*
 * We have multiple signs to draw at the same location. Draw the
 * multi-sign indicator instead. This is the GTK/Gnome version.
 */
    void
netbeans_draw_multisign_indicator(int row)
{
    int i;
    int y;
    int x;
    GdkDrawable *drawable = gui.drawarea->window;

    x = 0;
    y = row * gui.char_height + 2;

    for (i = 0; i < gui.char_height - 3; i++)
	gdk_draw_point(drawable, gui.text_gc, x+2, y++);

    gdk_draw_point(drawable, gui.text_gc, x+0, y);
    gdk_draw_point(drawable, gui.text_gc, x+2, y);
    gdk_draw_point(drawable, gui.text_gc, x+4, y++);
    gdk_draw_point(drawable, gui.text_gc, x+1, y);
    gdk_draw_point(drawable, gui.text_gc, x+2, y);
    gdk_draw_point(drawable, gui.text_gc, x+3, y++);
    gdk_draw_point(drawable, gui.text_gc, x+2, y);
}
#endif /* FEAT_GUI_GTK */

/*
 * If the mouse is clicked in the gutter of a line with multiple
 * annotations, cycle through the set of signs.
 */
    void
netbeans_gutter_click(linenr_T lnum)
{
    signlist_T	*p;

    for (p = curbuf->b_signlist; p != NULL; p = p->next)
    {
	if (p->lnum == lnum && p->next && p->next->lnum == lnum)
	{
	    signlist_T *tail;

	    /* remove "p" from list, reinsert it at the tail of the sublist */
	    if (p->prev)
		p->prev->next = p->next;
	    else
		curbuf->b_signlist = p->next;
	    p->next->prev = p->prev;
	    /* now find end of sublist and insert p */
	    for (tail = p->next;
		  tail->next && tail->next->lnum == lnum
					    && tail->next->id < GUARDEDOFFSET;
		  tail = tail->next)
		;
	    /* tail now points to last entry with same lnum (except
	     * that "guarded" annotations are always last) */
	    p->next = tail->next;
	    if (tail->next)
		tail->next->prev = p;
	    p->prev = tail;
	    tail->next = p;
	    update_debug_sign(curbuf, lnum);
	    break;
	}
    }
}


/*
 * Add a sign of the reqested type at the requested location.
 *
 * Reverse engineering:
 * Apparently an annotation is defined the first time it is used in a buffer.
 * When the same annotation is used in two buffers, the second time we do not
 * need to define a new sign name but reuse the existing one.  But since the
 * ID number used in the second buffer starts counting at one again, a mapping
 * is made from the ID specifically for the buffer to the global sign name
 * (which is a number).
 *
 * globalsignmap[]	stores the signs that have been defined globally.
 * buf->signmapused[]	maps buffer-local annotation IDs to an index in
 *			globalsignmap[].
 */
/*ARGSUSED*/
    static void
addsigntype(
    nbbuf_T	*buf,
    int		typeNum,
    char_u	*typeName,
    char_u	*tooltip,
    char_u	*glyphFile,
    int		use_fg,
    int		fg,
    int		use_bg,
    int		bg)
{
    char fgbuf[32];
    char bgbuf[32];
    int i, j;

    for (i = 0; i < globalsignmapused; i++)
	if (STRCMP(typeName, globalsignmap[i]) == 0)
	    break;

    if (i == globalsignmapused) /* not found; add it to global map */
    {
	nbdebug(("DEFINEANNOTYPE(%d,%s,%s,%s,%d,%d)\n",
			    typeNum, typeName, tooltip, glyphFile, fg, bg));
	if (use_fg || use_bg)
	{
	    sprintf(fgbuf, "guifg=#%06x", fg & 0xFFFFFF);
	    sprintf(bgbuf, "guibg=#%06x", bg & 0xFFFFFF);

	    coloncmd(":highlight NB_%s %s %s", typeName, (use_fg) ? fgbuf : "",
		     (use_bg) ? bgbuf : "");
	    if (*glyphFile == NUL)
		/* no glyph, line highlighting only */
		coloncmd(":sign define %d linehl=NB_%s", i + 1, typeName);
	    else if (vim_strsize(glyphFile) <= 2)
		/* one- or two-character glyph name, use as text glyph with
		 * texthl */
		coloncmd(":sign define %d text=%s texthl=NB_%s", i + 1,
							 glyphFile, typeName);
	    else
		/* glyph, line highlighting */
		coloncmd(":sign define %d icon=%s linehl=NB_%s", i + 1,
							 glyphFile, typeName);
	}
	else
	    /* glyph, no line highlighting */
	    coloncmd(":sign define %d icon=%s", i + 1, glyphFile);

	if (STRCMP(typeName,"CurrentPC") == 0)
	    curPCtype = typeNum;

	if (globalsignmapused == globalsignmaplen)
	{
	    if (globalsignmaplen == 0) /* first allocation */
	    {
		globalsignmaplen = 20;
		globalsignmap = (char **)alloc_clear(globalsignmaplen*sizeof(char *));
	    }
	    else    /* grow it */
	    {
		int incr;
		int oldlen = globalsignmaplen;

		globalsignmaplen *= 2;
		incr = globalsignmaplen - oldlen;
		globalsignmap = (char **)vim_realloc(globalsignmap,
					   globalsignmaplen * sizeof(char *));
		memset(globalsignmap + oldlen, 0, incr * sizeof(char *));
	    }
	}

	globalsignmap[i] = (char *)typeName;
	globalsignmapused = i + 1;
    }

    /* check local map; should *not* be found! */
    for (j = 0; j < buf->signmapused; j++)
	if (buf->signmap[j] == i + 1)
	    return;

    /* add to local map */
    if (buf->signmapused == buf->signmaplen)
    {
	if (buf->signmaplen == 0) /* first allocation */
	{
	    buf->signmaplen = 5;
	    buf->signmap = (int *)alloc_clear(buf->signmaplen * sizeof(int *));
	}
	else    /* grow it */
	{
	    int incr;
	    int oldlen = buf->signmaplen;
	    buf->signmaplen *= 2;
	    incr = buf->signmaplen - oldlen;
	    buf->signmap = (int *)vim_realloc(buf->signmap,
					       buf->signmaplen*sizeof(int *));
	    memset(buf->signmap + oldlen, 0, incr * sizeof(int *));
	}
    }

    buf->signmap[buf->signmapused++] = i + 1;

}


/*
 * See if we have the requested sign type in the buffer.
 */
    static int
mapsigntype(nbbuf_T *buf, int localsigntype)
{
    if (--localsigntype >= 0 && localsigntype < buf->signmapused)
	return buf->signmap[localsigntype];

    return 0;
}


/*
 * Compute length of buffer, don't print anything.
 */
    static long
get_buf_size(buf_T *bufp)
{
    linenr_T	lnum;
    long	char_count = 0;
    int		eol_size;
    long	last_check = 100000L;

    if (bufp->b_ml.ml_flags & ML_EMPTY)
	return 0;
    else
    {
	if (get_fileformat(bufp) == EOL_DOS)
	    eol_size = 2;
	else
	    eol_size = 1;
	for (lnum = 1; lnum <= bufp->b_ml.ml_line_count; ++lnum)
	{
	    char_count += STRLEN(ml_get(lnum)) + eol_size;
	    /* Check for a CTRL-C every 100000 characters */
	    if (char_count > last_check)
	    {
		ui_breakcheck();
		if (got_int)
		    return char_count;
		last_check = char_count + 100000L;
	    }
	}
	/* Correction for when last line doesn't have an EOL. */
	if (!bufp->b_p_eol && bufp->b_p_bin)
	    char_count -= eol_size;
    }

    return char_count;
}

/*
 * Convert character offset to lnum,col
 */
    static pos_T *
off2pos(buf_T *buf, long offset)
{
    linenr_T	 lnum;
    static pos_T pos;

    pos.lnum = 0;
    pos.col = 0;
#ifdef FEAT_VIRTUALEDIT
    pos.coladd = 0;
#endif

    if (!(buf->b_ml.ml_flags & ML_EMPTY))
    {
	if ((lnum = ml_find_line_or_offset(buf, (linenr_T)0, &offset)) < 0)
	    return NULL;
	pos.lnum = lnum;
	pos.col = offset;
    }

    return &pos;
}

/*
 * Convert an argument in the form "1234" to an offset and compute the
 * lnum/col from it.  Convert an argument in the form "123/12" directly to a
 * lnum/col.
 * "argp" is advanced to after the argument.
 * Return a pointer to the position, NULL if something is wrong.
 */
    static pos_T *
get_off_or_lnum(buf_T *buf, char_u **argp)
{
    static pos_T	mypos;
    long		off;

    off = strtol((char *)*argp, (char **)argp, 10);
    if (**argp == '/')
    {
	mypos.lnum = (linenr_T)off;
	++*argp;
	mypos.col = strtol((char *)*argp, (char **)argp, 10);
#ifdef FEAT_VIRTUALEDIT
	mypos.coladd = 0;
#endif
	return &mypos;
    }
    return off2pos(buf, off);
}


/*
 * Convert lnum,col to character offset
 */
    static long
pos2off(buf_T *buf, pos_T *pos)
{
    long	 offset = 0;

    if (!(buf->b_ml.ml_flags & ML_EMPTY))
    {
	if ((offset = ml_find_line_or_offset(buf, pos->lnum, 0)) < 0)
	    return 0;
	offset += pos->col;
    }

    return offset;
}


#if defined(FEAT_GUI_MOTIF) || defined(PROTO)
/*
 *	This function fills in the XRectangle object with the current
 *	x,y coordinates and height, width so that an XtVaSetValues to
 *	the same shell of those resources will restore the window to its
 *	formar position and dimensions.
 *
 *	Note: This function may fail, in which case the XRectangle will
 *	be unchanged.  Be sure to have the XRectangle set with the
 *	proper values for a failed condition prior to calling this
 *	function.
 */
    void
shellRectangle(Widget shell, XRectangle *r)
{
	Window		rootw, shellw, child, parentw;
	int		absx, absy;
	XWindowAttributes	a;
	Window		*children;
	unsigned int	childrenCount;

	shellw = XtWindow(shell);
	if (shellw == 0)
		return;
	for (;;)
	{
	    XQueryTree(XtDisplay(shell), shellw, &rootw, &parentw,
					&children, &childrenCount);
	    XFree(children);
	    if (parentw == rootw)
		break;
	    shellw = parentw;
	}
	XGetWindowAttributes(XtDisplay(shell), shellw, &a);
	XTranslateCoordinates(XtDisplay(shell), shellw, a.root, 0, 0,
				&absx, &absy, &child);
	r->x = absx;
	r->y = absy;
	XtVaGetValues(shell, XmNheight, &r->height,
				XmNwidth, &r->width, NULL);
}
#endif

#endif /* defined(FEAT_NETBEANS_INTG) */
