#include "house.h"
#include "node.h"
#include "misc.h"

static gdouble objects_max_radius(GSList *objects);
static void objects_max_extension(GSList *objects,
	gdouble *min_x, gdouble *min_y, gdouble *min_z,
	gdouble *max_x,	gdouble *max_y, gdouble *max_z);

/* HBPart stuff **************************************************************/

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

#define THRESHOLD 2.0

gboolean part_select_line(HBPart *part, gdouble x, gdouble y)
{
	gdouble x1, y1, x2, y2, ax, ay, bx, by;

	if(g_slist_length(part->nodes) < 2)
		return FALSE;

	node_get_xy(part, 0, &x1, &y1);
	node_get_xy(part, 1, &x2, &y2);

	if(x1 < x2) {
		ax = x1;
		ay = y1;
		bx = x2;
		by = y2;
	} else {
		ax = x2;
		ay = y2;
		bx = x1;
		by = y1;
	}

	g_debug("select line: (%.2f, %.2f), (%.2f, %.2f), (%.2f, %.2f)",
		ax, ay, bx, by, x, y);

	if(((ax - THRESHOLD) > x) || (x > (bx + THRESHOLD)) ||
		((MIN(ay, by) - THRESHOLD) > y) || (y > (MAX(ay, by) + THRESHOLD)))
		return FALSE;

	if(misc_delta_p(ax, ay, bx, by, x, y) > THRESHOLD)
		return FALSE;

	return TRUE;
}

gint32 part_select_node(HBPart *part, gdouble x, gdouble y)
{
	gint32 i, n;
	gdouble nx, ny;

	if(part == NULL)
		return -1;

	n = g_slist_length(part->nodes);
	for(i = 0; i < n; i ++) {
		node_get_xy(part, i, &nx, &ny);
		if(misc_delta(nx, ny, x, y) < 5.0)
			return i;
	}
	return -1;
}

/* HBHouse stuff *************************************************************/

gboolean house_update_position_hints(HBHouse *house)
{
	gdouble min_x = 10.0e99, min_y = 10.0e99, min_z = 10.0e99;
	gdouble max_x = -9.9e99, max_y = -9.9e99, max_z = -9.9e99;

	if(g_slist_length(house->model->objects) == 0) {
		house->off_x = 0.0;
		house->off_y = 0.0;
		house->off_z = 0.0;
		house->scale = 1.0;
	}

	/* get maximum extension */
	objects_max_extension(house->model->objects,
		&min_x, &min_y, &min_z, &max_x, &max_y, &max_z);

	house->off_x = max_x - ((max_x - min_x) / 2.0);
	house->off_y = max_y - ((max_y - min_y) / 2.0);
	house->off_z = max_z - ((max_z - min_z) / 2.0);

	/* get scale factor */
	house->scale = 10.0 / objects_max_radius(house->model->objects);

#if DEBUG > 2
	g_debug("position hints: (%.2f, %.2f, %.2f), %.2f",
		house->off_x, house->off_y, house->off_z, house->scale);
#endif
	return TRUE;
}

HBPart *house_select_part(HBHouse *house, gint32 floor, gdouble x, gdouble y)
{
	HBPart *part;
	GSList *pitem;

	/* FIXME: floor handling */
	for(pitem = house->parts; pitem != NULL; pitem = pitem->next) {
		part = (HBPart *)pitem->data;
		if(part->type->select) {
			if(part->type->select(part, x, y))
				return part;
		} else {
			/* fallback to line selection if unset */
			if(part_select_line(part, x, y))
				return part;
		}
	}
	return NULL;
}

gboolean house_render_part_3d(HBHouse *house, HBPart *part,
	HBTextureLoader *loader)
{
	gboolean retval = FALSE;

	if((house == NULL) || (part == NULL))
		return FALSE;

	/* remove old object */
	if(part->object) {
		house->model->objects = g_slist_remove(house->model->objects,
			part->object);
		g3d_object_free(part->object);
		part->object = NULL;
	}

	/* render part */
	if(part->type->render3d) {
		/* render 3d stuff */
		retval = part->type->render3d(part, house->model->context, loader);
		/* add part to model */
		if(part->object != NULL) {
			house->model->objects = g_slist_append(house->model->objects,
				part->object);
		}
		/* inform the renderer */
		house_update_position_hints(house);
		house->dirty = TRUE; /* rebuild GL list */
	}
	return retval;
}

/* helper functions **********************************************************/

static gdouble objects_max_radius(GSList *objects)
{
	G3DObject *object;
	GSList *oitem;
	gdouble radius, max_rad = 0.0;

	oitem = objects;
	while(oitem)
	{
		object = (G3DObject *)oitem->data;
		radius = g3d_object_radius(object);
		if(radius > max_rad)
			max_rad = radius;

		radius = objects_max_radius(object->objects);
		if(radius > max_rad)
			max_rad = radius;

		oitem = oitem->next;
	}

	return max_rad;
}

static void objects_max_extension(GSList *objects,
	gdouble *min_x, gdouble *min_y, gdouble *min_z,
	gdouble *max_x,	gdouble *max_y, gdouble *max_z)
{
	GSList *oitem;
	G3DObject *object;
	guint32 i;

	oitem = objects;
	while(oitem)
	{
		object = (G3DObject *)oitem->data;
		for(i = 0; i < object->vertex_count; i ++)
		{
			if(object->vertex_data[i * 3 + 0] < *min_x)
				*min_x = object->vertex_data[i * 3 + 0];
			if(object->vertex_data[i * 3 + 1] < *min_y)
				*min_y = object->vertex_data[i * 3 + 1];
			if(object->vertex_data[i * 3 + 2] < *min_z)
				*min_z = object->vertex_data[i * 3 + 2];

			if(object->vertex_data[i * 3 + 0] > *max_x)
				*max_x = object->vertex_data[i * 3 + 0];
			if(object->vertex_data[i * 3 + 1] > *max_y)
				*max_y = object->vertex_data[i * 3 + 1];
			if(object->vertex_data[i * 3 + 2] > *max_z)
				*max_z = object->vertex_data[i * 3 + 2];
		}

		objects_max_extension(object->objects,
			min_x, min_y, min_z, max_x, max_y, max_z);

		oitem = oitem->next;
	}
}

gboolean house_get_max_extension(HBHouse *house, gint32 floor, gdouble *mx,
	gdouble *my)
{
	GSList *pitem;
	HBPart *part;
	gint32 i, n;
	gdouble x, y;

	/* FIXME: floor handling */
	for(pitem = house->parts; pitem != NULL; pitem = pitem->next) {
		part = (HBPart *)pitem->data;
		n = g_slist_length(part->nodes);
		for(i = 0; i < n; i ++) {
			node_get_xy(part, i, &x, &y);
			if(x > *mx)
				*mx = x;
			if(y > *my)
				*my = y;
		}
	}
	return TRUE;
}
