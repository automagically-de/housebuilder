#ifndef _VIEW2D_H
#define _VIEW2D_H

#include "view.h"
#include "gui.h"

HBView *view2d_new(void);
void view2d_redraw(HBView *view);

/* callbacks */
void view2d_upper_floor_cb(GtkWidget *widget, HBView *view);
void view2d_lower_floor_cb(GtkWidget *widget, HBView *view);

#endif /* _VIEW2D_H */
