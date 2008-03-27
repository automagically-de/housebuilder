#include <gtk/gtk.h>
#include <cairo/cairo.h>

#include "house.h"
#include "node.h"
#include "view2d.h"
#include "tools.h"

#include "part_wall.h"

typedef struct {
	gfloat scale;
	GtkWidget *da;
	HBPart *preview;
	gint32 drag_x;
	gint32 drag_y;
	GtkWidget *ruler_h;
	GtkWidget *ruler_v;
	HBToolID selected_tool;
	HBPart *selected_part;
	gint32 selected_floor;
} View2DPrivate;

static gboolean expose_event_cb (GtkWidget *widget, GdkEventExpose *event,
	gpointer data);
static gboolean button_press_cb(GtkWidget *widget, GdkEventButton *event,
	gpointer data);
static gboolean button_release_cb(GtkWidget *widget, GdkEventButton *event,
	gpointer data);
static gboolean scroll_cb(GtkWidget *widget, GdkEventScroll *event,
	gpointer data);
static gboolean motion_notify_cb(GtkWidget *widget, GdkEventMotion *event,
	gpointer data);
static gboolean configure_cb(GtkWidget *widget, GdkEventConfigure *event,
	gpointer data);
static gboolean view2d_scroll_da_cb(GtkAdjustment *adj, HBView *view);

HBView *view2d_new(void)
{
	GtkWidget *table, *sw;
	HBView *view;
	View2DPrivate *priv;

	view = g_new0(HBView, 1);
	priv = g_new0(View2DPrivate, 1);
	priv->scale = 1.0;
	priv->drag_x = priv->drag_y = -1;
	priv->selected_tool = TOOL_SELECT;
	view->user_data = priv;

	/* table */
	table = gtk_table_new(2, 2, FALSE);

	/* scrolled window */
	sw = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_table_attach_defaults(GTK_TABLE(table), sw, 1, 2, 1, 2);

	/* drawing area */
	priv->da = gtk_layout_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(sw), priv->da);
	gtk_widget_set_events(priv->da,
		GDK_EXPOSURE_MASK |
		GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK |
		GDK_POINTER_MOTION_MASK |
		GDK_SCROLL_MASK);
	g_signal_connect(G_OBJECT(priv->da), "expose-event",
		G_CALLBACK(expose_event_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "scroll-event",
		G_CALLBACK(scroll_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "button-press-event",
		G_CALLBACK(button_press_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "button-release-event",
		G_CALLBACK(button_release_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "motion-notify-event",
		G_CALLBACK(motion_notify_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "configure-event",
		G_CALLBACK(configure_cb), view);
	g_signal_connect(
		G_OBJECT(gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(sw))),
		"value-changed",
		G_CALLBACK(view2d_scroll_da_cb), view);
	g_signal_connect(
		G_OBJECT(gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sw))),
		"value-changed",
		G_CALLBACK(view2d_scroll_da_cb), view);

	/* rulers */
	priv->ruler_h = gtk_hruler_new();
	gtk_ruler_set_range(GTK_RULER(priv->ruler_h), 0, 800, 0, 800);
	gtk_table_attach(GTK_TABLE(table), priv->ruler_h, 1, 2, 0, 1,
		GTK_EXPAND|GTK_SHRINK|GTK_FILL, GTK_FILL, 0, 0);

	priv->ruler_v = gtk_vruler_new();
	gtk_ruler_set_range(GTK_RULER(priv->ruler_v), 0, 600, 0, 600);
	gtk_table_attach(GTK_TABLE(table), priv->ruler_v, 0, 1, 1, 2,
		GTK_FILL, GTK_EXPAND|GTK_SHRINK|GTK_FILL, 0, 0);

	view->widget = table;

	return view;
}

void view2d_redraw(HBView *view)
{
	View2DPrivate *priv = (View2DPrivate *)view->user_data;

	gtk_widget_queue_draw_area(priv->da, 0, 0,
		priv->da->allocation.width, priv->da->allocation.height);
}

/* helper functions **********************************************************/

static void rulers_update(HBView *view)
{
	View2DPrivate *priv = (View2DPrivate *)view->user_data;
	gint32 x, y;
	gdouble ox, oy;
	GdkModifierType state;

	gdk_window_get_pointer(priv->da->window, &x, &y, &state);

	ox = gtk_adjustment_get_value(gtk_layout_get_hadjustment(
		GTK_LAYOUT(priv->da))) / priv->scale;
	oy = gtk_adjustment_get_value(gtk_layout_get_vadjustment(
		GTK_LAYOUT(priv->da))) / priv->scale;

	gtk_ruler_set_range(GTK_RULER(priv->ruler_h),
		ox, ox + priv->da->allocation.width / priv->scale,
		ox + x / priv->scale,
		ox + priv->da->allocation.width / priv->scale);

	gtk_ruler_set_range(GTK_RULER(priv->ruler_v),
		oy, oy + priv->da->allocation.height / priv->scale,
		oy + y / priv->scale,
		oy + priv->da->allocation.height / priv->scale);
}

static void set_layout_size(HBView *view)
{
	View2DPrivate *priv = (View2DPrivate *)view->user_data;
	gdouble wx, wy, mx = 0.0, my = 0.0;

	/* size of visible area */
	wx = priv->da->allocation.width;
	wy = priv->da->allocation.height;

	/* maximum extension of parts */
	house_get_max_extension(gui_get_house(view->gui),
		priv->selected_floor, &mx, &my);
	if(priv->selected_floor > 0)
		house_get_max_extension(gui_get_house(view->gui),
			priv->selected_floor - 1, &mx, &my);

	mx = mx * priv->scale + 10;
	my = my * priv->scale + 10;

#if DEBUG > 2
	g_debug("set_layout_size: window: %.2f, %.2f; house: %.2f, %.2f",
		wx, wy, mx, my);
#endif

	gtk_layout_set_size(GTK_LAYOUT(priv->da),
		(gint32)MAX(wx, mx), (gint32)MAX(wy, my));
}

/* drawing area callbacks ****************************************************/

static gboolean expose_event_cb (GtkWidget *widget, GdkEventExpose *event,
	gpointer data)
{
	HBView *view = (HBView *)data;
	HBPart *part;
	View2DPrivate *priv = (View2DPrivate *)view->user_data;
	GSList *item;
	cairo_t *cairo;
	gint32 x, y;
	gdouble ox, oy, wx, wy;

	cairo = gdk_cairo_create(GTK_LAYOUT(widget)->bin_window);
	cairo_scale(cairo, priv->scale, priv->scale);

	/* background */
	cairo_set_source_rgba(cairo, 1.0, 1.0, 1.0, 1.0); /* white */
	cairo_paint(cairo);

	/* grid */
	cairo_set_source_rgba(cairo, 0.3, 0.3, 0.3, 1.0);

	ox = gtk_adjustment_get_value(gtk_layout_get_hadjustment(
		GTK_LAYOUT(priv->da))) / priv->scale;
	oy = gtk_adjustment_get_value(gtk_layout_get_vadjustment(
		GTK_LAYOUT(priv->da))) / priv->scale;
	wx = priv->da->allocation.width / priv->scale;
	wy = priv->da->allocation.height / priv->scale;

	for(y = oy / 10; y <= (oy + wy) / 10; y ++) {
		if((y % 10) == 0)
			cairo_set_line_width(cairo, 1.0);
		else
			cairo_set_line_width(cairo, 0.4);
		cairo_move_to(cairo, ox, y * 10);
		cairo_line_to(cairo, (ox + wx), y * 10);
		cairo_stroke(cairo);
	}
	for(x = ox / 10; x <= (ox + wx) / 10; x ++) {
		if((x % 10) == 0)
			cairo_set_line_width(cairo, 1.0);
		else
			cairo_set_line_width(cairo, 0.4);
		cairo_move_to(cairo, x * 10, oy);
		cairo_line_to(cairo, x * 10, (oy + wy));
		cairo_stroke(cairo);
	}

	/* parts */
	for(item = gui_get_house(view->gui)->parts; item; item = item->next) {
		part = (HBPart *)item->data;

		if(part->type->render2d) {
			cairo_save(cairo);
			part->type->render2d(part, cairo, LAYER_BASE);
			cairo_restore(cairo);
		}
	}

	/* preview */
	if(priv->preview && priv->preview->type->render2d)
		priv->preview->type->render2d(priv->preview, cairo, LAYER_PREVIEW);

	cairo_destroy(cairo);

	return TRUE;
}

static gboolean button_press_cb(GtkWidget *widget, GdkEventButton *event,
	gpointer data)
{
	HBView *view = (HBView *)data;
	View2DPrivate *priv = (View2DPrivate *)view->user_data;

	priv->drag_x = (event->x / priv->scale);
	priv->drag_y = (event->y / priv->scale);

	/* FIXME: tool selection */
	if(priv->selected_tool == TOOL_ADD_WALL) {
		priv->preview = part_wall_new();
		node_set_xy(priv->preview, 0, priv->drag_x, priv->drag_y);
		node_set_xy(priv->preview, 1, priv->drag_x, priv->drag_y);
		view2d_redraw(view);
	}
	return TRUE;
}

static gboolean button_release_cb(GtkWidget *widget, GdkEventButton *event,
	gpointer data)
{
	HBView *view = (HBView *)data;
	HBHouse *house;
	HBPart *part;
	View2DPrivate *priv = (View2DPrivate *)view->user_data;

	house = gui_get_house(view->gui);

	/* FIXME: tool selection */
	if(priv->selected_tool == TOOL_ADD_WALL) {
		part = part_wall_new();
		node_set_xy(part, 0, priv->drag_x, priv->drag_y);
		node_set_xy(part, 1,
			(event->x / priv->scale),
			(event->y / priv->scale));
		house->parts = g_slist_append(house->parts, part);

		if(part->type->render3d) {
			part->type->render3d(part, house->model);
			house_update_position_hints(house);
			house->dirty = TRUE; /* rebuild GL list */
		}

		set_layout_size(view);
	}

	if(priv->preview) {
		part_free(priv->preview);
		priv->preview = NULL;
		view2d_redraw(view);
	}

	priv->drag_x = -1;
	priv->drag_y = -1;
	return TRUE;
}

static gboolean scroll_cb(GtkWidget *widget, GdkEventScroll *event,
	gpointer data)
{
	HBView *view = (HBView *)data;
	View2DPrivate *priv = (View2DPrivate *)view->user_data;

	switch(event->direction) {
		case GDK_SCROLL_DOWN:
			if(priv->scale > 0.05)
				priv->scale /= 1.5;
			break;
		case GDK_SCROLL_UP:
			if(priv->scale < 20.0)
				priv->scale *= 1.5;
			break;
		default:
			break;
	}

	set_layout_size(view);
	rulers_update(view);
	view2d_redraw(view);

	return TRUE;
}

static gboolean motion_notify_cb(GtkWidget *widget, GdkEventMotion *event,
	gpointer data)
{
	HBView *view = (HBView *)data;
	View2DPrivate *priv = (View2DPrivate *)view->user_data;

	if(priv->drag_x >= 0) {
		if(priv->preview) {
			node_set_xy(priv->preview, 1,
				(event->x / priv->scale),
				(event->y / priv->scale));
			view2d_redraw(view);
		}
	}

	rulers_update(view);

	return TRUE;
}

static gboolean configure_cb(GtkWidget *widget, GdkEventConfigure *event,
	gpointer data)
{
	set_layout_size((HBView *)data);
	rulers_update((HBView *)data);
	return TRUE;
}

static gboolean view2d_scroll_da_cb(GtkAdjustment *adj, HBView *view)
{
#if DEBUG > 2
	g_debug("view2d_scroll_da_cb called");
#endif
	rulers_update(view);
	view2d_redraw(view);
	return TRUE;
}

/* callbacks for menu/toolbar actions ****************************************/

void view2d_upper_floor_cb(GtkWidget *widget, HBView *view)
{
	View2DPrivate *priv = (View2DPrivate *)view->user_data;
}

void view2d_lower_floor_cb(GtkWidget *widget, HBView *view)
{
	View2DPrivate *priv = (View2DPrivate *)view->user_data;
}

void view2d_select_tool_cb(GtkAction *action, GtkRadioAction *current,
	HBView *view)
{
	View2DPrivate *priv = (View2DPrivate *)view->user_data;

	priv->selected_tool = gtk_radio_action_get_current_value(current);
}

