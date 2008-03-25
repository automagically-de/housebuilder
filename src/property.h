#ifndef _PROPERTY_H
#define _PROPERTY_H

#include <gtk/gtk.h>

typedef enum {
	PROPERTY_UNHANDLED = 0,
	PROPERTY_DOUBLE,
	PROPERTY_INTEGER,
	PROPERTY_COLOR,
	PROPERTY_CUSTOM = 255
} PropertyType;

typedef struct {
	PropertyType type;
	GtkWidget *widget;
	GtkWidget *signal_widget;
	gint32 signal_handler;
	gpointer custom;
} PropertyPrivate;

typedef struct {
	gpointer var;
	const gchar *title;
	PropertyPrivate *priv;
} Property;

GtkWidget *property_get_widget(PropertyPrivate *priv);
gboolean property_update_handler(PropertyPrivate *priv, gpointer var);
gboolean property_add(GSList *list, const gchar *title, PropertyPrivate *priv,
		gpointer var);

gchar *property_var_to_str(PropertyType type, gpointer var);
void property_str_to_var(PropertyType type, const gchar *str, gpointer var);
gchar *property_type_to_str(PropertyType type);
PropertyType property_str_to_type(const gchar *str);

GtkWidget *property_default_properties_handler(GSList *list, GtkWidget **wp);
Property *property_get_by_name(GSList *list, const gchar *title);

PropertyPrivate *property_new_double(gdouble defval, gdouble min, gdouble max,
	gdouble step);
PropertyPrivate *property_new_integer(gint32 defval, gint32 min, gint32 max);
PropertyPrivate *property_new_color(guint32 defval);
PropertyPrivate *property_new_custom(GtkWidget *widget, const gchar *signal,
	GCallback callback);

#define property_set_var(list, title, type, value) \
	do { \
		Property *p = property_get_by_name(list, title); \
		if(p != NULL) \
			*((type *)(p)->var) = value; \
	} while(0)

#define property_set_integer(o, title, value) \
	property_set_var(o, title, gint32, value)
#define property_set_double(o, title, value) \
	property_set_var(o, title, gdouble, value)
#define property_set_color(o, title, value) \
	property_set_var(o, title, guint32, value)

gboolean property_set_from_str(GSList *list, const gchar *typestr,
	const gchar *varname, const gchar *value);

#endif /* _PROPERTY_H */
