/* workshop.c */
extern void workshop_init __ARGS((void));
extern void workshop_postinit __ARGS((void));
extern void ex_wsverb __ARGS((exarg_T *eap));
extern char *workshop_get_editor_name __ARGS((void));
extern char *workshop_get_editor_version __ARGS((void));
extern void workshop_load_file __ARGS((char *filename, int line, char *frameid));
extern void workshop_reload_file __ARGS((char *filename, int line));
extern void workshop_show_file __ARGS((char *filename));
extern void workshop_goto_line __ARGS((char *filename, int lineno));
extern void workshop_front_file __ARGS((char *filename));
extern void workshop_save_file __ARGS((char *filename));
extern void workshop_save_files __ARGS((void));
extern void workshop_quit __ARGS((void));
extern void workshop_minimize __ARGS((void));
extern void workshop_maximize __ARGS((void));
extern void workshop_add_mark_type __ARGS((int idx, char *colorspec, char *sign));
extern void workshop_set_mark __ARGS((char *filename, int lineno, int markId, int idx));
extern void workshop_change_mark_type __ARGS((char *filename, int markId, int idx));
extern void workshop_goto_mark __ARGS((char *filename, int markId, char *message));
extern void workshop_delete_mark __ARGS((char *filename, int markId));
extern int workshop_get_mark_lineno __ARGS((char *filename, int markId));
extern void workshop_moved_marks __ARGS((char *filename));
extern int workshop_get_font_height __ARGS((void));
extern void workshop_footer_message __ARGS((char *message, int severity));
extern void workshop_menu_begin __ARGS((char *label));
extern void workshop_submenu_begin __ARGS((char *label));
extern void workshop_submenu_end __ARGS((void));
extern void workshop_menu_item __ARGS((char *label, char *verb, char *accelerator, char *acceleratorText, char *name, char *filepos, char *sensitive));
extern void workshop_menu_end __ARGS((void));
extern void workshop_toolbar_begin __ARGS((void));
extern void workshop_toolbar_end __ARGS((void));
extern void workshop_toolbar_button __ARGS((char *label, char *verb, char *senseVerb, char *filepos, char *help, char *sense, char *file, char *left));
extern void workshop_frame_sensitivities __ARGS((VerbSense *vs));
extern void workshop_set_option __ARGS((char *option, char *value));
extern void workshop_balloon_mode __ARGS((Boolean on));
extern void workshop_balloon_delay __ARGS((int delay));
extern void workshop_show_balloon_tip __ARGS((char *tip));
extern void workshop_hotkeys __ARGS((Boolean on));
extern int workshop_get_positions __ARGS((void *clientData, char **filename, int *curLine, int *curCol, int *selStartLine, int *selStartCol, int *selEndLine, int *selEndCol, int *selLength, char **selection));
extern char *workshop_test_getcurrentfile __ARGS((void));
extern int workshop_test_getcursorrow __ARGS((void));
extern int workshop_test_getcursorcol __ARGS((void));
extern char *workshop_test_getcursorrowtext __ARGS((void));
extern char *workshop_test_getselectedtext __ARGS((void));
extern void workshop_save_sensitivity __ARGS((char *filename));
extern void findYourself __ARGS((char *argv0));
/* vim: set ft=c : */
