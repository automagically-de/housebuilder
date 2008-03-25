#ifndef _VIEW_H
#define _VIEW_H

#include <gtk/gtk.h>

#include "gui.h"

typedef struct {
	GtkWidget *widget;
	gpointer user_data;
	HBGui *gui;
} HBView;

#endif /* _VIEW_H */
