#ifndef _GUI_H
#define _GUI_H

typedef struct _HBGui HBGui;

HBGui *gui_init(int *argcp, char ***argvp);
gboolean gui_run(HBGui *gui);

#endif /* _GUI_H */
