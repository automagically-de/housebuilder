#include "node.h"
#include "misc.h"

struct _HBNode {
	gdouble x;
	gdouble y;
	gdouble z;
};

gboolean node_add_n(HBPart *part, guint32 n)
{
	HBNode *node;
	guint32 i;

	for(i = 0; i < n; i ++) {
		node = g_new0(HBNode, 1);
		part->nodes = g_slist_append(part->nodes, node);
	}

	return TRUE;
}

gboolean node_set_xy(HBPart *part, guint32 n, gdouble x, gdouble y)
{
	HBNode *node;

	node = g_slist_nth_data(part->nodes, n);
	if(node == NULL)
		return FALSE;

	node->x = x;
	node->y = y;

	return TRUE;
}

gboolean node_get_xy(HBPart *part, guint32 n, gdouble *x, gdouble *y)
{
	HBNode *node;

	node = g_slist_nth_data(part->nodes, n);
	if(node == NULL)
		return FALSE;

	*x = node->x;
	*y = node->y;
	return TRUE;
}

gboolean node_get_center(HBPart *part, gdouble *cx, gdouble *cy)
{
	gdouble x1, x2, y1, y2;

	node_get_xy(part, 0, &x1, &y1);
	node_get_xy(part, 1, &x2, &y2);

	*cx = MIN(x1, x2) + ABS((gdouble)(x2 - x1) / 2.0);
	*cy = MIN(y1, y2) + ABS((gdouble)(y2 - y1) / 2.0);
	return TRUE;
}

gdouble node_get_angle(HBPart *part)
{
	gdouble x1, x2, y1, y2;

	node_get_xy(part, 0, &x1, &y1);
	node_get_xy(part, 1, &x2, &y2);

	return misc_angle(x1, y1, x2, y2);
}

gdouble node_get_delta(HBPart *part)
{
	gdouble x1, x2, y1, y2;

	node_get_xy(part, 0, &x1, &y1);
	node_get_xy(part, 1, &x2, &y2);

	return misc_delta(x1, y1, x2, y2);
}
