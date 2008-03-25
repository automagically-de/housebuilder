#include "node.h"

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

