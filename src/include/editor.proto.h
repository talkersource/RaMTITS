#ifndef _EDITOR_C
  #define EXTERN extern
#else
  #define EXTERN
#endif

EXTERN void nuts_editor(User,char *);
EXTERN void ramtits_editor(User,char *);
EXTERN void redraw_edit(User,Editor);
EXTERN void editor_done(User);
EXTERN Editor get_editor(User);
EXTERN Editor create_editor(void);
EXTERN void destruct_editor(Editor);
#undef EXTERN
