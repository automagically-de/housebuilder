#include "view3d.h"

typedef struct {
	int bla;
} View3DPrivate;

HBView *view3d_new(void)
{
	HBView *view;
	View3DPrivate *priv;

	view = g_new0(HBView, 1);
	priv = g_new0(View3DPrivate, 1);
	view->user_data = priv;

	/* FIXME: 3D view NYI */
	view->widget = gtk_label_new("3D View");

	return view;
}
