#ifndef _NODE_H
#define _NODE_H

#include "house.h"

typedef struct _HBNode HBNode;

gboolean node_add_n(HBPart *part, guint32 n);
gboolean node_set_xy(HBPart *part, guint32 n, gdouble x, gdouble y);
gboolean node_get_xy(HBPart *part, guint32 n, gdouble *x, gdouble *y);
gboolean node_get_center(HBPart *part, gdouble *cx, gdouble *cy);
gdouble node_get_angle(HBPart *part);
gdouble node_get_delta(HBPart *part);

#endif /* _NODE_H */
