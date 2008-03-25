#include "house.h"

HBPart *part_new(HBPartType *type, gpointer data)
{
	HBPart *part;

	part = g_new0(HBPart, 1);
	part->type = type;
	part->data = data;

	return part;
}

void part_free(HBPart *part)
{
	/* FIXME: delete properties & nodes */
	if(part->data)
		g_free(part->data);

	g_free(part);
}
