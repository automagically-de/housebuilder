#ifndef _NODE_H
#define _NODE_H

#include "house.h"

typedef struct _HBNode HBNode;

gboolean node_add_n(HBPart *part, guint32 n);
gboolean node_set_xy(HBPart *part, guint32 n, gdouble x, gdouble y);

#endif /* _NODE_H */
