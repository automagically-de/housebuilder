#include <GL/gl.h>

#include "view3d.h"
#include "gl.h"
#include "trackball.h"

typedef struct {
	GtkWidget *da;
	gdouble aspect;
	gdouble zoom;
	gint drag_x;
	gint drag_y;
	gdouble offx;
	gdouble offy;
	GLfloat quat[4];
} View3DPrivate;

gboolean view3d_scroll_cb(GtkWidget *widget, GdkEventScroll *event,
	HBView *view);
gboolean view3d_configure_cb(GtkWidget *widget, GdkEventConfigure *event,
	HBView *view);
gboolean view3d_destroy_cb(GtkWidget *widget, GdkEvent *event, HBView *view);
gboolean view3d_expose_cb(GtkWidget *widget, GdkEventExpose *event,
	HBView *view);
gboolean view3d_button_press_cb(GtkWidget *widget, GdkEventButton *event,
	HBView *view);
gboolean view3d_motion_cb(GtkWidget *widget, GdkEventMotion *event,
	HBView *view);

HBView *view3d_new(void)
{
	HBView *view;
	View3DPrivate *priv;
	GdkGLConfig *glconfig;

	view = g_new0(HBView, 1);
	priv = g_new0(View3DPrivate, 1);
	priv->drag_x = -1;
	priv->drag_y = -1;
	priv->aspect = 1;
	priv->zoom = 1;
	trackball(priv->quat, 0.0, 0.0, 0.0, 0.0);
	view->user_data = priv;

	glconfig = gl_get_config();
	if(glconfig == NULL) {
		view->widget = gtk_label_new("OpenGL could not be initialized");
		g_warning("failed to initialize OpenGL");
		return view;
	}

	/* create GL area */
	priv->da = gtk_drawing_area_new();
	gtk_widget_set_gl_capability(priv->da, glconfig, NULL, TRUE,
		GDK_GL_RGBA_TYPE);

	if(priv->da == NULL) {
		view->widget = gtk_label_new("OpenGL widget could not be created");
		g_warning("failed to create GtkGLExt widget");
		return view;
	}

	gtk_widget_set_events(priv->da,
		GDK_EXPOSURE_MASK |
		GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
		GDK_SCROLL_MASK |
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK);

	g_signal_connect(G_OBJECT(priv->da), "scroll_event",
		G_CALLBACK(view3d_scroll_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "expose_event",
		G_CALLBACK(view3d_expose_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "motion_notify_event",
		G_CALLBACK(view3d_motion_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "button_press_event",
		G_CALLBACK(view3d_button_press_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "configure_event",
		G_CALLBACK(view3d_configure_cb), view);
	g_signal_connect(G_OBJECT(priv->da), "destroy_event",
		G_CALLBACK(view3d_destroy_cb), view);

	view->widget = priv->da;

	return view;
}

void view3d_redraw(HBView *view)
{
	View3DPrivate *priv = (View3DPrivate *)view->user_data;

	gtk_widget_queue_draw_area(priv->da, 0, 0,
		priv->da->allocation.width, priv->da->allocation.height);
}

/* drawing area callback functions *******************************************/

gboolean view3d_scroll_cb(GtkWidget *widget, GdkEventScroll *event,
	HBView *view)
{
	View3DPrivate *priv = (View3DPrivate *)view->user_data;

	if(event->direction == GDK_SCROLL_DOWN) {
		if(priv->zoom < 100)
			priv->zoom *= 1.5;
	} else {
		if(priv->zoom > 1)
			priv->zoom /= 1.5;
	}
	view3d_redraw(view);
	return TRUE;
}

gboolean view3d_configure_cb(GtkWidget *widget, GdkEventConfigure *event,
	HBView *view)
{
	View3DPrivate *priv = (View3DPrivate *)view->user_data;
	GdkGLDrawable *gldrawable;
	GdkGLContext *glcontext;

	gldrawable = gtk_widget_get_gl_drawable(widget);
	glcontext = gtk_widget_get_gl_context(widget);

	if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return TRUE;

	glViewport(0,0, widget->allocation.width, widget->allocation.height);
	priv->aspect = (gfloat)widget->allocation.width /
		(gfloat)widget->allocation.height;
	gdk_gl_drawable_gl_end(gldrawable);
	return TRUE;
}

gboolean view3d_destroy_cb(GtkWidget *widget, GdkEvent *event, HBView *view)
{
	return TRUE;
}

gboolean view3d_expose_cb(GtkWidget *widget, GdkEventExpose *event,
	HBView *view)
{
	HBHouse *house;
	View3DPrivate *priv = (View3DPrivate *)view->user_data;
	GdkGLDrawable *gldrawable;
	GdkGLContext *glcontext;

	if(event->count > 0) return TRUE;

	gldrawable = gtk_widget_get_gl_drawable(widget);
	glcontext = gtk_widget_get_gl_context(widget);

	if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
		return TRUE;

	house = gui_get_house(view->gui);
	gl_draw(house, priv->zoom, priv->aspect, priv->quat,
		priv->offx, priv->offy);

	gdk_gl_drawable_swap_buffers(gldrawable);
	gdk_gl_drawable_gl_end(gldrawable);
	return TRUE;
}

gboolean view3d_button_press_cb(GtkWidget *widget, GdkEventButton *event,
	HBView *view)
{
	View3DPrivate *priv = (View3DPrivate *)view->user_data;

	if(event->button == 1) {
		priv->drag_x = event->x;
		priv->drag_y = event->y;
		return TRUE;
	}

	return FALSE;
}

gboolean view3d_motion_cb(GtkWidget *widget, GdkEventMotion *event,
	HBView *view)
{
	HBHouse *house;
	View3DPrivate *priv = (View3DPrivate *)view->user_data;
	gint x, y;
	GdkModifierType state;
	GdkRectangle area;

	house = gui_get_house(view->gui);

	if(event->is_hint)
		gdk_window_get_pointer(event->window, &x, &y, &state);
	else {
		x = event->x;
		y = event->y;
		state = event->state;
	}

	area.x = area.y = 0;
	area.width = widget->allocation.width;
	area.height = widget->allocation.height;

	if(state & GDK_BUTTON1_MASK) {
		/* left button pressed */
		if(state & GDK_SHIFT_MASK) {
			/* shift pressed, translate view */
			house->off_x += (gdouble)(x - priv->drag_x) / 10.0;
			house->off_z += (gdouble)(y - priv->drag_y) / 10.0;
		} else {
			/* rotate view */
			gfloat spin_quat[4];
			trackball(spin_quat,
				(2.0 * priv->drag_x - area.width) / area.width,
				(area.height - 2.0 * priv->drag_y) / area.height,
				(2.0 * x - area.width) / area.width,
				(area.height - 2.0 * y) / area.height);
			add_quats(spin_quat, priv->quat, priv->quat);
		}

		view3d_redraw(view);
	}

	priv->drag_x = x;
	priv->drag_y = y;
	return TRUE;
}

