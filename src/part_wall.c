#include "house.h"
#include "property.h"

gboolean part_wall_render2d(HBPart *part, cairo_t *cairo, LayerID layerid);
gboolean part_wall_render3d(HBPart *part, G3DModel *model);

HBPartType part_wall = {
	"wall",
	part_wall_render2d,
	part_wall_render3d,
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

	return part;
}

gboolean part_wall_render2d(HBPart *part, cairo_t *cairo, LayerID layerid)
{

	return TRUE;
}

gboolean part_wall_render3d(HBPart *part, G3DModel *model)
{
	return TRUE;
}
