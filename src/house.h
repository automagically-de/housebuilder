#ifndef _HOUSE_H
#define _HOUSE_H

#include <g3d/g3d.h>
#include <cairo/cairo.h>
#include <gtk/gtk.h>

#include "layers.h"

typedef struct _HBPartType HBPartType;
typedef struct _HBPart HBPart;

typedef gboolean (*HBPartTypeRender2DFunc)(HBPart *part,
	cairo_t *cairo, LayerID layerid);
typedef gboolean (*HBPartTypeRender3DFunc)(HBPart *part, G3DModel *model);
typedef gboolean (*HBPartTypeSelectFunc)(HBPart *part, gdouble x, gdouble y);
typedef GtkWidget (*HBPartTypePropertiesFunc)(HBPart *part, GtkWidget **wp);

struct _HBPartType {
	const gchar *title;
	HBPartTypeRender2DFunc render2d;
	HBPartTypeRender3DFunc render3d;
	HBPartTypeSelectFunc select;
	HBPartTypePropertiesFunc properties;
};

struct _HBPart {
	gboolean selected;
	gint floor;
	HBPartType *type;
	G3DObject *object;
	GSList *nodes;
	GSList *properties;
	gpointer data;
};

HBPart *part_new(HBPartType *type, gpointer data);
void part_free(HBPart *part);

typedef struct {
	gboolean dirty;
	GSList *parts;
	G3DModel *model;
	gint32 dlist;
	gdouble off_x;
	gdouble off_y;
	gdouble off_z;
	gdouble scale;
} HBHouse;

gboolean house_update_position_hints(HBHouse *house);
gboolean house_get_max_extension(HBHouse *house, gint32 floor, gdouble *mx,
	gdouble *my);

#endif /* _HOUSE_H */
