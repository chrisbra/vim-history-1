" vi:set ts=8 sts=8 sw=8 tw=0:
"
" Menu Translations:	Japanese (for Unix)
" Translated By:	Muraoka Taro  <koron@tka.att.ne.jp>
" Last Change:		13-Jul-2001.

" Quit when menu translations have already been done.
if exists("did_menu_trans")
  finish
endif
let did_menu_trans = 1

scriptencoding euc-jp

" Help menu
menutrans &Help			�إ��(&H)
menutrans &Overview<Tab><F1>	��ά(&O)<Tab><F1>
menutrans &User\ Manual		�桼���ޥ˥奢��(&U)
menutrans &How-to\ links	&How-to���
menutrans &Credits		���쥸�å�(&C)
menutrans Co&pying		�������(&P)
menutrans &Version		�С���������(&V)
" menutrans &About		&About

" File menu
menutrans &File				�ե�����(&F)
menutrans &Open\.\.\.<Tab>:e		����(&O)\.\.\.<Tab>:e
menutrans Sp&lit-Open\.\.\.<Tab>:sp	ʬ�䤷�Ƴ���(&L)\.\.\.<Tab>:sp
menutrans &New<Tab>:enew		��������(&N)<Tab>:enew
menutrans &Close<Tab>:q			�Ĥ���(&C)<Tab>:q
menutrans &Save<Tab>:w			��¸(&S)<Tab>:w
menutrans Save\ &As\.\.\.<Tab>:sav	̾�����դ�����¸(&A)\.\.\.<Tab>:sav
menutrans Split\ &Diff\ with\.\.\.	��ʬɽ��(&D)\.\.\.
menutrans Split\ Patched\ &By\.\.\.	�ѥå���̤�ɽ��(&B)\.\.\.
menutrans &Print			����(&P)
menutrans Sa&ve-Exit<Tab>:wqa		��¸���ƽ�λ(&V)<Tab>:wqa
menutrans E&xit<Tab>:qa			��λ(&X)<Tab>:qa

" Edit menu
menutrans &Edit				�Խ�(&E)
menutrans &Undo<Tab>u			���ä�(&U)<Tab>u
menutrans &Redo<Tab>^R			�⤦���٤��(&R)<Tab>^R
menutrans Rep&eat<Tab>\.		�����֤�(&E)<Tab>\.
menutrans Cu&t<Tab>"+x			�ڤ���(&T)<Tab>"+x
menutrans &Copy<Tab>"+y			���ԡ�(&C)<Tab>"+y
menutrans &Paste<Tab>"+P		Ž���դ�(&P)<Tab>"+P
menutrans Put\ &Before<Tab>[p		����Ž��(&B)<Tab>[p
menutrans Put\ &After<Tab>]p		���Ž��(&A)<Tab>]p
menutrans &Delete<Tab>x			�ä�(&D)<Tab>x
menutrans &Select\ all<Tab>ggVG		��������(&S)<Tab>ggvG
menutrans &Find\.\.\.			����(&F)\.\.\.
menutrans Find\ and\ Rep&lace\.\.\.	�ִ�(&L)\.\.\.
"menutrans Options\.\.\.			���ץ����(&O)\.\.\.
menutrans Settings\ &Window		���ꥦ����ɥ�(&W)

" Edit/Global Settings
menutrans &Global\ Settings		��������(&G)
menutrans Toggle\ Pattern\ &Highlight<Tab>:set\ hls!
	\	�ѥ�����Ĵ����(&H)<Tab>:set\ hls!
menutrans Toggle\ &Ignore-case<Tab>:set\ ic!
	\	�羮ʸ����������(&I)<Tab>:set\ ic!
menutrans Toggle\ &Showmatch<Tab>:set\ sm!
	\	�ޥå�ɽ������(&S)<Tab>:set\ sm!
menutrans &Context\ lines		����������չԿ�(&C)
menutrans &Virtual\ Edit		�����Խ�(&V)
menutrans Never				̵��
menutrans Block\ Selection		�֥��å������
menutrans Insert\ mode			�����⡼�ɻ�
menutrans Block\ and\ Insert		�֥��å�/�����⡼�ɻ�
menutrans Always			���
menutrans Toggle\ Insert\ &Mode<Tab>:set\ im!
	\	����(�鿴��)�⡼������(&M)<Tab>:set\ im!
menutrans Search\ &Path\.\.\.		�����ѥ�(&P)\.\.\.
menutrans Ta&g\ Files\.\.\.		�����ե�����(&G)\.\.\.
"
" GUI options
menutrans Toggle\ &Toolbar		�ġ���С�ɽ������(&T)
menutrans Toggle\ &Bottom\ Scrollbar	����������С�(��)ɽ������(&B)
menutrans Toggle\ &Left\ Scrollbar	����������С�(��)ɽ������(&L)
menutrans Toggle\ &Right\ Scrollbar	����������С�(��)ɽ������(&R)

" Edit/File Settings

" Boolean options
menutrans F&ile\ Settings		�ե���������(&I)
menutrans Toggle\ Line\ &Numbering<Tab>:set\ nu!
	\	���ֹ�ɽ������(&N)<Tab>:set\ nu!
menutrans Toggle\ &List\ Mode<Tab>:set\ list!
	\ �ꥹ�ȥ⡼������(&L)<Tab>:set\ list!
menutrans Toggle\ Line\ &Wrap<Tab>:set\ wrap!
	\	�����֤�����(&W)<Tab>:set\ wrap!
menutrans Toggle\ W&rap\ at\ word<Tab>:set\ lbr!
	\	ñ�����֤�����(&R)<Tab>:set\ lbr!
menutrans Toggle\ &expand-tab<Tab>:set\ et!
	\	����Ÿ������(&E)<Tab>:set\ et!
menutrans Toggle\ &auto-indent<Tab>:set\ ai!
	\	��ư����������(&A)<Tab>:set\ ai!
menutrans Toggle\ &C-indenting<Tab>:set\ cin!
	\	C�������������(&C)<Tab>:set\ cin!

" other options
menutrans &Shiftwidth			���ե���(&S)
menutrans Soft\ &Tabstop		���եȥ�����������(&T)
menutrans Te&xt\ Width\.\.\.		�ƥ�������(&X)\.\.\.
menutrans &File\ Format\.\.\.		���Ե�������(&F)\.\.\.
menutrans C&olor\ Scheme		���ơ�������(&O)
menutrans &Keymap			�����ޥå�(&K)
menutrans None				�ʤ�

" Programming menu
menutrans &Tools			�ġ���(&T)
menutrans &Jump\ to\ this\ tag<Tab>g^]	����������(&J)<Tab>g^]
menutrans Jump\ &back<Tab>^T		���(&B)<Tab>^T
menutrans Build\ &Tags\ File		�����ե��������(&T)
menutrans &Make<Tab>:make		�ᥤ��(&M)<Tab>:make
menutrans &List\ Errors<Tab>:cl		���顼�ꥹ��(&L)<Tab>:cl
menutrans L&ist\ Messages<Tab>:cl!	��å������ꥹ��(&I)<Tab>:cl!
menutrans &Next\ Error<Tab>:cn		���Υ��顼��(&N)<Tab>:cn
menutrans &Previous\ Error<Tab>:cp	���Υ��顼��(&P)<Tab>:cp
menutrans &Older\ List<Tab>:cold	�Ť��ꥹ��(&O)<Tab>:cold
menutrans N&ewer\ List<Tab>:cnew	�������ꥹ��(&E)<Tab>:cnew
menutrans Error\ &Window		���顼������ɥ�(&W)
menutrans &Update<Tab>:cwin		����(&U)<Tab>:cwin
menutrans &Open<Tab>:copen		����(&O)<Tab>:copen
menutrans &Close<Tab>:cclose		�Ĥ���(&C)<Tab>:cclose
menutrans &Convert\ to\ HEX<Tab>:%!xxd	HEX���Ѵ�(&C)<Tab>:%!xxd
menutrans Conve&rt\ back<Tab>:%!xxd\ -r	HEX������Ѵ�(&R)<Tab>%!xxd\ -r
menutrans &Set\ Compiler		����ѥ�������(&S)

" Tools.Fold Menu
menutrans &Folding			�޾���(&F)
" open close folds
menutrans &Enable/Disable\ folds<Tab>zi	ͭ��/̵������(&E)<Tab>zi
menutrans &View\ Cursor\ Line<Tab>zv	��������Ԥ�ɽ��(&V)<Tab>zv
menutrans Vie&w\ Cursor\ Line\ only<Tab>zMzx	��������Ԥ�����ɽ��(&W)<Tab>zMzx
menutrans C&lose\ more\ folds<Tab>zm	�޾��ߤ��Ĥ���(&L)<Tab>zm
menutrans &Close\ all\ folds<Tab>zM	���޾��ߤ��Ĥ���(&C)<Tab>zM
menutrans O&pen\ more\ folds<Tab>zr	�޾��ߤ򳫤�(&P)<Tab>zr
menutrans &Open\ all\ folds<Tab>zR	���޾��ߤ򳫤�(&O)<Tab>zR
" fold method
menutrans Fold\ Met&hod			�޾�����ˡ(&H)
menutrans M&anual			��ư(&A)
menutrans I&ndent			����ǥ��(&N)
menutrans E&xpression			��ɾ��(&X)
menutrans S&yntax			���󥿥å���(&Y)
menutrans &Diff				��ʬ(&D)
menutrans Ma&rker			�ޡ�����(&R)
" create and delete folds
menutrans Create\ &Fold<Tab>zf		�޾��ߺ���(&F)<Tab>zf
menutrans &Delete\ Fold<Tab>zd		�޾��ߺ��(&D)<Tab>zd
menutrans Delete\ &All\ Folds<Tab>zD	���޾��ߺ��(&A)<Tab>zD
" moving around in folds
menutrans Fold\ column\ &width		�޾��ߥ������(&W)

menutrans &Update		����(&U)
menutrans &Get\ Block		�֥��å����(&G)
menutrans &Put\ Block		�֥��å�Ŭ��(&P)

" Names for buffer menu.
menutrans &Buffers		�Хåե�(&B)
menutrans &Refresh\ menu	��˥塼���ɹ�(&R)
menutrans &Delete		���(&D)
menutrans &Alternate		΢������(&A)
menutrans &Next			���ΥХåե�(&N)
menutrans &Previous		���ΥХåե�(&P)
menutrans [No\ File]		[̵��]
let g:menutrans_no_file = "[̵��]"

" Window menu
menutrans &Window			������ɥ�(&W)
menutrans &New<Tab>^Wn			��������(&N)<Tab>^Wn
menutrans S&plit<Tab>^Ws		ʬ��(&P)<Tab>^Ws
menutrans Sp&lit\ To\ #<Tab>^W^^	΢�Хåե���ʬ��(&L)<Tab>^W^^
menutrans Split\ &Vertically<Tab>^Wv	��ľʬ��(&V)<Tab>^Wv
menutrans Split\ File\ E&xplorer	�ե����륨�����ץ�����(&X)
menutrans &Close<Tab>^Wc		�Ĥ���(&C)<Tab>^Wc
menutrans Move\ &To			��ư(&T)
menutrans &Top<Tab>^WK			��Ƭ(&T)
menutrans &Bottom<Tab>^WJ		����(&B)
menutrans &Left\ side<Tab>^WH		��(&L)
menutrans &Right\ side<Tab>^WL		��(&R)
menutrans Close\ &Other(s)<Tab>^Wo	¾���Ĥ���(&O)<Tab>^Wo
menutrans Ne&xt<Tab>^Ww			����(&X)<Tab>^Ww
menutrans P&revious<Tab>^WW		����(&R)<Tab>^WW
menutrans &Equal\ Size<Tab>^W=	Ʊ���⤵��(&E)<Tab>^W=
menutrans &Max\ Height<Tab>^W_		������(&M)<Tab>^W
menutrans M&in\ Height<Tab>^W1_		�Ǿ����(&i)<Tab>^W1_
menutrans Max\ &Width<Tab>^W\|		��������(&W)<Tab>^W\|
menutrans Min\ Widt&h<Tab>^W1\|		�Ǿ�����(&H)<Tab>^W1\|
menutrans Rotate\ &Up<Tab>^WR		��˥����ơ������(&U)<Tab>^WR
menutrans Rotate\ &Down<Tab>^Wr		���˥����ơ������(&D)<Tab>^Wr
menutrans Select\ Fo&nt\.\.\.		�ե��������(&N)\.\.\.

" The popup menu
menutrans &Undo			���ä�(&U)
menutrans Cu&t			�ڤ���(&T)
menutrans &Copy			���ԡ�(&C)
menutrans &Paste		Ž���դ�(&P)
menutrans &Delete		���(&D)
menutrans Select\ Blockwise	����֥��å�����
menutrans Select\ &Word		ñ������(&W)
menutrans Select\ &Line		������(&L)
menutrans Select\ &Block	�֥��å�����(&B)
menutrans Select\ &All		���٤�����(&A)

" The GUI toolbar (for Win32 or GTK)
if has("toolbar")
  if exists("*Do_toolbar_tmenu")
    delfun Do_toolbar_tmenu
  endif
  fun Do_toolbar_tmenu()
    tmenu ToolBar.Open		�ե�����򳫤�
    tmenu ToolBar.Save		���ߤΥե��������¸
    tmenu ToolBar.SaveAll	���٤ƤΥե��������¸
    tmenu ToolBar.Print		����
    tmenu ToolBar.Undo		���ä�
    tmenu ToolBar.Redo		�⤦���٤��
    tmenu ToolBar.Cut		����åץܡ��ɤ��ڤ���
    tmenu ToolBar.Copy		����åץܡ��ɤإ��ԡ�
    tmenu ToolBar.Paste		����åץܡ��ɤ���Ž���դ�
    tmenu ToolBar.Find		����...
    tmenu ToolBar.FindNext	���򸡺�
    tmenu ToolBar.FindPrev	���򸡺�
    tmenu ToolBar.Replace	�ִ�...
    if 0	" disabled; These are in the Windows menu
      tmenu ToolBar.New		����������ɥ�����
      tmenu ToolBar.WinSplit	������ɥ�ʬ��
      tmenu ToolBar.WinMax	������ɥ����粽
      tmenu ToolBar.WinMin	������ɥ��Ǿ���
      tmenu ToolBar.WinClose	������ɥ����Ĥ���
    endif
    tmenu ToolBar.LoadSesn	���å�����ɹ�
    tmenu ToolBar.SaveSesn	���å������¸
    tmenu ToolBar.RunScript	Vim������ץȼ¹�
    tmenu ToolBar.Make		�ץ��������Ȥ�Make
    tmenu ToolBar.Shell		������򳫤�
    tmenu ToolBar.RunCtags	tags����
    tmenu ToolBar.TagJump	����������
    tmenu ToolBar.Help		Vim�إ��
    tmenu ToolBar.FindHelp	Vim�إ�׸���
  endfun
endif

" Syntax menu
menutrans &Syntax		���󥿥å���(&S)
menutrans Set\ '&syntax'\ only	'syntax'��������(&S)
menutrans Set\ '&filetype'\ too	'filetype'������(&F)
menutrans &Off			̵����(&O)
menutrans &Manual		��ư����(&M)
menutrans A&utomatic		��ư����(&U)
menutrans on/off\ for\ &This\ file
	\	����/��������(&T)
menutrans Co&lor\ test		���顼�ƥ���(&L)
menutrans &Highlight\ test	�ϥ��饤�ȥƥ���(&H)
menutrans &Convert\ to\ HTML	HTML�إ���С���(&C)