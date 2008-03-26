#ifndef _GL_H
#define _GL_H

#include <glib.h>
#include <gtk/gtkgl.h>

#include "house.h"

GdkGLConfig *gl_get_config(void);
gboolean gl_draw(HBHouse *house, gdouble zoom, gdouble aspect, gfloat *quat,
	gdouble offx, gdouble offy);

#endif /* _GL_H */
