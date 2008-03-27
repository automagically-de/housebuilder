#include <g3d/g3d.h>
#include <g3d/primitive.h>
#include <g3d/matrix.h>

#include "house.h"
#include "node.h"
#include "property.h"

gboolean part_wall_render2d(HBPart *part, cairo_t *cairo, LayerID layerid);
gboolean part_wall_render3d(HBPart *part, G3DModel *model);

HBPartType part_wall = {
	"wall",
	part_wall_render2d,
	part_wall_render3d,
	NULL,
	property_default_properties_handler
};

typedef struct {
	gdouble thickness;
} PartWall;

HBPart *part_wall_new(void)
{
	HBPart *part;
	PartWall *wall;
	PropertyPrivate *ppriv;

	wall = g_new0(PartWall, 1);
	wall->thickness = 2.0;

	part = part_new(&part_wall, wall);
	ppriv = property_new_double(2.0, 0.1, 4.0, 0.1);
	property_add(part->properties, "thickness", ppriv, &(wall->thickness));

	node_add_n(part, 2);

	return part;
}

gboolean part_wall_render2d(HBPart *part, cairo_t *cairo, LayerID layerid)
{
	PartWall *wall = (PartWall *)part->data;
	gdouble cx, cy, angle, delta;

	node_get_center(part, &cx, &cy);
	angle = node_get_angle(part);
	delta = node_get_delta(part);

	cairo_translate(cairo, cx, cy);
	cairo_rotate(cairo, angle);

	switch(layerid) {
		case LAYER_BASE:
			cairo_set_line_width(cairo, 1.0);
			cairo_rectangle(cairo,
				- delta / 2.0, - wall->thickness / 2.0,
				delta, wall->thickness);
			cairo_set_source_rgba(cairo, 1.0, 0.8, 0.8, 1.0);
			cairo_fill_preserve(cairo);
			cairo_set_source_rgba(cairo, 0.0, 0.0, 0.0, 1.0);
			cairo_stroke(cairo);
			break;
		case LAYER_PREVIEW:
			cairo_set_source_rgba(cairo, 0.0, 0.0, 0.0, 0.7);
			cairo_set_line_width(cairo, 1.0);
			cairo_rectangle(cairo,
				- delta / 2.0, - wall->thickness / 2.0,
				delta, wall->thickness);
			cairo_stroke(cairo);
			break;
		case LAYER_SELECT:
			cairo_set_source_rgba(cairo, 1.0, 0.2, 0.2, 0.7);
			cairo_set_line_width(cairo, 1.5);
			cairo_rectangle(cairo,
				- delta / 2.0, - wall->thickness / 2.0,
				delta, wall->thickness);
			cairo_stroke(cairo);
			break;
		default:
			break;
	}
	return TRUE;
}

gboolean part_wall_render3d(HBPart *part, G3DModel *model)
{
	static G3DMaterial *mat = NULL;
	PartWall *wall = (PartWall *)part->data;
	gdouble cx, cy, angle, delta;
	gfloat matrix[16];

	node_get_center(part, &cx, &cy);
	angle = node_get_angle(part);
	delta = node_get_delta(part);

	if(mat == NULL) {
		/* FIXME */
		mat = g3d_material_new();
		mat->r = 0.8;
		mat->g = 0.2;
		mat->b = 0.0;
		mat->a = 1.0;
	}

	part->object = g3d_primitive_box(delta, 20.0, wall->thickness, mat);

	g3d_matrix_identity(matrix);
	g3d_matrix_rotate(-angle, 0.0, 1.0, 0.0, matrix);
	g3d_object_transform(part->object, matrix);
	g3d_object_transform_normals(part->object, matrix);

	g3d_matrix_identity(matrix);
	g3d_matrix_translate(cx, 10.0, cy, matrix);
	g3d_object_transform(part->object, matrix);

	return TRUE;
}
