/* buffer.c */
extern int open_buffer __ARGS((int read_stdin, exarg_T *eap));
extern int buf_valid __ARGS((buf_T *buf));
extern void close_buffer __ARGS((win_T *win, buf_T *buf, int action));
extern void buf_clear_file __ARGS((buf_T *buf));
extern void buf_freeall __ARGS((buf_T *buf, int del_buf, int wipe_buf));
extern void goto_buffer __ARGS((exarg_T *eap, int start, int dir, int count));
extern void handle_swap_exists __ARGS((buf_T *old_curbuf));
extern char_u *do_bufdel __ARGS((int command, char_u *arg, int addr_count, int start_bnr, int end_bnr, int forceit));
extern int do_buffer __ARGS((int action, int start, int dir, int count, int forceit));
extern void set_curbuf __ARGS((buf_T *buf, int action));
extern void enter_buffer __ARGS((buf_T *buf));
extern buf_T *buflist_new __ARGS((char_u *ffname, char_u *sfname, linenr_T lnum, int flags));
extern void free_buf_options __ARGS((buf_T *buf, int free_p_ff));
extern int buflist_getfile __ARGS((int n, linenr_T lnum, int options, int forceit));
extern void buflist_getfpos __ARGS((void));
extern buf_T *buflist_findname __ARGS((char_u *ffname));
extern int buflist_findpat __ARGS((char_u *pattern, char_u *pattern_end, int unlisted, int diffmode));
extern int ExpandBufnames __ARGS((char_u *pat, int *num_file, char_u ***file, int options));
extern buf_T *buflist_findnr __ARGS((int nr));
extern char_u *buflist_nr2name __ARGS((int n, int fullname, int helptail));
extern void get_winopts __ARGS((buf_T *buf));
extern pos_T *buflist_findfpos __ARGS((buf_T *buf));
extern linenr_T buflist_findlnum __ARGS((buf_T *buf));
extern void buflist_list __ARGS((exarg_T *eap));
extern int buflist_name_nr __ARGS((int fnum, char_u **fname, linenr_T *lnum));
extern int setfname __ARGS((buf_T *buf, char_u *ffname, char_u *sfname, int message));
extern void buf_name_changed __ARGS((buf_T *buf));
extern buf_T *setaltfname __ARGS((char_u *ffname, char_u *sfname, linenr_T lnum));
extern char_u *getaltfname __ARGS((int errmsg));
extern int buflist_add __ARGS((char_u *fname, int flags));
extern void buflist_slash_adjust __ARGS((void));
extern void buflist_altfpos __ARGS((void));
extern int otherfile __ARGS((char_u *ffname));
extern void buf_setino __ARGS((buf_T *buf));
extern void fileinfo __ARGS((int fullname, int shorthelp, int dont_truncate));
extern void col_print __ARGS((char_u *buf, int col, int vcol));
extern void maketitle __ARGS((void));
extern void resettitle __ARGS((void));
extern int build_stl_str_hl __ARGS((win_T *wp, char_u *out, size_t outlen, char_u *fmt, int fillchar, int maxwidth, struct stl_hlrec *hl));
extern void get_rel_pos __ARGS((win_T *wp, char_u *str));
extern int append_arg_number __ARGS((win_T *wp, char_u *buf, int add_file, int maxlen));
extern char_u *fix_fname __ARGS((char_u *fname));
extern void fname_expand __ARGS((buf_T *buf, char_u **ffname, char_u **sfname));
extern char_u *alist_name __ARGS((aentry_T *aep));
extern void do_arg_all __ARGS((int count, int forceit));
extern void ex_buffer_all __ARGS((exarg_T *eap));
extern void do_modelines __ARGS((void));
extern int read_viminfo_bufferlist __ARGS((vir_T *virp, int writing));
extern void write_viminfo_bufferlist __ARGS((FILE *fp));
extern char *buf_spname __ARGS((buf_T *buf));
extern void buf_addsign __ARGS((buf_T *buf, int id, linenr_T lnum, int typenr));
extern int buf_change_sign_type __ARGS((buf_T *buf, int markId, int typenr));
extern int_u buf_getsigntype __ARGS((buf_T *buf, linenr_T lnum, int type));
extern linenr_T buf_delsign __ARGS((buf_T *buf, int id));
extern int buf_findsign __ARGS((buf_T *buf, int id));
extern int buf_findsign_id __ARGS((buf_T *buf, linenr_T lnum));
extern int buf_findsigntype_id __ARGS((buf_T *buf, linenr_T lnum, int typenr));
extern int buf_signcount __ARGS((buf_T *buf, linenr_T lnum));
extern void buf_delete_all_signs __ARGS((void));
extern void sign_list_placed __ARGS((buf_T *rbuf));
extern void sign_mark_adjust __ARGS((linenr_T line1, linenr_T line2, long amount, long amount_after));
extern void set_buflisted __ARGS((int on));
extern int buf_contents_changed __ARGS((buf_T *buf));
extern void wipe_buffer __ARGS((buf_T *buf, int aucmd));
/* vim: set ft=c : */
