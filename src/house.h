#ifndef _HOUSE_H
#define _HOUSE_H

#include <g3d/g3d.h>
#include <cairo/cairo.h>
#include <gtk/gtk.h>

#include "layers.h"
#include "texture.h"

typedef struct _HBHouse HBHouse;

/* HBPart stuff **************************************************************/

typedef struct _HBPartType HBPartType;
typedef struct _HBPart HBPart;

typedef gboolean (*HBPartTypeRender2DFunc)(HBPart *part,
	cairo_t *cairo, LayerID layerid);
typedef gboolean (*HBPartTypeRender3DFunc)(HBPart *part, G3DContext *context,
	HBTextureLoader *loader);
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
gboolean part_select_line(HBPart *part, gdouble x, gdouble y);
gint32 part_select_node(HBPart *part, gdouble x, gdouble y);

/* HBFloor stuff *************************************************************/

typedef struct {
	gint32 n;
	gdouble height;
	GSList *properties;
	GSList *parts;
	GSList *object;
} HBFloor;

HBFloor *floor_new(HBHouse *house);

/* HBHouse stuff *************************************************************/

struct _HBHouse {
	gboolean dirty;
	GSList *floors;
	G3DModel *model;
	gint32 dlist;
	gdouble off_x;
	gdouble off_y;
	gdouble off_z;
	gdouble scale;
};

gboolean house_update_position_hints(HBHouse *house);
gboolean house_get_max_extension(HBHouse *house, gint32 n_floor, gdouble *mx,
	gdouble *my);
HBPart *house_select_part(HBHouse *house, gint32 n_floor, gdouble x,
	gdouble y);
gboolean house_render_part_3d(HBHouse *house, HBPart *part,
	HBTextureLoader *loader);

#endif /* _HOUSE_H */
