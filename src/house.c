#include "house.h"

static gdouble objects_max_radius(GSList *objects);
static void objects_max_extension(GSList *objects,
	gdouble *min_x, gdouble *min_y, gdouble *min_z,
	gdouble *max_x,	gdouble *max_y, gdouble *max_z);

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

	g_debug("position hints: (%.2f, %.2f, %.2f), %.2f",
		house->off_x, house->off_y, house->off_z, house->scale);

	return TRUE;
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

