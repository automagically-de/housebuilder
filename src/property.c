#include <stdlib.h>
#include <string.h>

#include "property.h"

static gboolean double_update_handler(PropertyPrivate *priv, gdouble *var);
static gboolean integer_update_handler(PropertyPrivate *priv, gint32 *var);
static gboolean color_update_handler(PropertyPrivate *priv, guint32 *var);
static gboolean custom_update_handler(PropertyPrivate *priv, gpointer var);

/*****************************************************************************/
/* generic                                                                   */
/*****************************************************************************/

GtkWidget *property_get_widget(PropertyPrivate *priv)
{
	return priv->widget;
}

Property *property_get_by_name(GSList *list, const gchar *title)
{
	GSList *item;
	Property *p;

	for(item = list; item != NULL; item = item->next)
	{
		p = (Property *)item->data;
		if(strcmp(p->title, title) == 0)
			return p;
	}
	return NULL;
}

gboolean property_update_handler(PropertyPrivate *priv, gpointer var)
{
	if(priv->signal_handler >= 0)
		g_signal_handler_disconnect(G_OBJECT(priv->widget),
			(gulong)priv->signal_handler);
	switch(priv->type)
	{
		case PROPERTY_DOUBLE:
			return double_update_handler(priv, (gdouble *)var);
		case PROPERTY_INTEGER:
			return integer_update_handler(priv, (gint32 *)var);
		case PROPERTY_COLOR:
			return color_update_handler(priv, (guint32 *)var);

		case PROPERTY_CUSTOM:
			return custom_update_handler(priv, var);
		default:
			return FALSE;
	}
}

gboolean property_add(GSList *list, const gchar *title, PropertyPrivate *priv,
	gpointer var)
{
	Property *prop;

	prop = g_new0(Property, 1);
	prop->title = title;
	prop->priv = priv;
	prop->var = var;
	list = g_slist_append(list, prop);
	return TRUE;
}

struct PropertyTypeConv {
	PropertyType id;
	const gchar *title;
};

static struct PropertyTypeConv p_types[] = {
	 { PROPERTY_DOUBLE, "double" },
	 { PROPERTY_INTEGER, "integer" },
	 { PROPERTY_COLOR, "color" },
	 { PROPERTY_CUSTOM, "custom" },

	 { PROPERTY_UNHANDLED, "unhandled" /* last in list */ }
};

gchar *property_type_to_str(PropertyType type)
{
	static GHashTable *type_to_str = NULL;
	gint32 i;
	gchar *str;

	if(type_to_str == NULL)
	{
		type_to_str = g_hash_table_new(g_int_hash, g_int_equal);
		i = 0;
		while(p_types[i].id != PROPERTY_UNHANDLED)
		{
			g_hash_table_insert(type_to_str, &(p_types[i].id),
				(gpointer)p_types[i].title);
			i ++;
		}
	}

	str = (gchar *)g_hash_table_lookup(type_to_str, &type);
	if(str != NULL)
		return g_strdup(str);
	else
		return g_strdup("unhandled");
}

PropertyType property_str_to_type(const gchar *str)
{
	static GHashTable *str_to_type = NULL;
	gint32 i;

	if(str_to_type == NULL)
	{
		str_to_type = g_hash_table_new(g_str_hash, g_str_equal);
		i = 0;
		while(p_types[i].id != PROPERTY_UNHANDLED)
		{
			g_hash_table_insert(str_to_type, (gchar *)p_types[i].title,
				GINT_TO_POINTER(p_types[i].id));
			i ++;
		}
	}

	return GPOINTER_TO_INT(g_hash_table_lookup(str_to_type, str));
}

gchar *property_var_to_str(PropertyType type, gpointer var)
{
	switch(type)
	{
		case PROPERTY_DOUBLE:
			return g_strdup_printf("%f", *((gdouble *)var));
		case PROPERTY_INTEGER:
			return g_strdup_printf("%i", *((gint32 *)var));
		case PROPERTY_COLOR:
			return g_strdup_printf("0x%08X", *((guint32 *)var));
		default:
			return g_strdup("");
	}
}

void property_str_to_var(PropertyType type, const gchar *str, gpointer var)
{
	switch(type)
	{
		case PROPERTY_DOUBLE:
			*((gdouble *)var) = atof(str);
			break;
		case PROPERTY_INTEGER:
			*((gint32 *)var) = atoi(str);
			break;
		case PROPERTY_COLOR:
			*((guint32 *)var) = strtol(str, NULL, 16);
			break;
		default:
			*((gint32 *)var) = 0;
			break;
	}
}

gboolean property_set_from_str(GSList *list, const gchar *typestr,
	const gchar *varname, const gchar *value)
{
	PropertyType pt;
	gint32 ival;
	guint32 cval;
	gdouble dval;

	pt = property_str_to_type(typestr);
	switch(pt)
	{
		case PROPERTY_INTEGER:
			property_str_to_var(pt, value, &ival);
			property_set_integer(list, varname, ival);
			break;
		case PROPERTY_DOUBLE:
			property_str_to_var(pt, value, &dval);
			property_set_double(list, varname, dval);
			break;
		case PROPERTY_COLOR:
			property_str_to_var(pt, value, &cval);
			property_set_color(list, varname, cval);
			break;
		default:
			break;
	}

	return TRUE;
}

GtkWidget *property_default_properties_handler(GSList *list, GtkWidget **wp)
{
	GtkWidget *w, *table;
	GSList *item;
	Property *prop;
	gint32 i, len;

	if(*wp == NULL)
	{
		len = g_slist_length(list);
		if(len <= 0)
			return NULL;

		table = gtk_table_new(len, 2, FALSE);
		*wp = table;

		for(item = list, i = 0; item != NULL; item = item->next, i ++)
		{
			prop = (Property *)item->data;
			w = gtk_label_new(prop->title);
			gtk_table_attach_defaults(GTK_TABLE(table), w, 0, 1, i, i + 1);
			w = property_get_widget(prop->priv);
			gtk_table_attach_defaults(GTK_TABLE(table), w, 1, 2, i, i + 1);
		}
		gtk_widget_show_all(table);
		g_object_ref(G_OBJECT(table));
	}

	for(item = list, i = 0; item != NULL; item = item->next, i ++)
	{
		prop = (Property *)item->data;
		property_update_handler(prop->priv, prop->var);
	}

	return *wp;
}

/*****************************************************************************/
/* custom                                                                    */
/*****************************************************************************/

typedef struct {
	const gchar *signal;
	GCallback callback;
} PropertyPrivateCustom;

PropertyPrivate *property_new_custom(GtkWidget *widget, const gchar *signal,
	GCallback callback)
{
	PropertyPrivate *priv;
	PropertyPrivateCustom *cpriv;

	priv = g_new0(PropertyPrivate, 1);
	priv->type = PROPERTY_CUSTOM;
	priv->signal_handler = -1;
	priv->widget = priv->signal_widget = widget;
	cpriv = g_new0(PropertyPrivateCustom, 1);
	cpriv->signal = signal;
	cpriv->callback = callback;
	priv->custom = cpriv;

	return priv;
}

static gboolean custom_update_handler(PropertyPrivate *priv, gpointer var)
{
	PropertyPrivateCustom *cpriv = (PropertyPrivateCustom *)priv->custom;

	priv->signal_handler = g_signal_connect(G_OBJECT(priv->signal_widget),
		cpriv->signal, cpriv->callback, var);

	return TRUE;
}

/*****************************************************************************/
/* double                                                                    */
/*****************************************************************************/

typedef struct {
	gdouble defval;
	gdouble min;
	gdouble max;
} PropertyPrivateDouble;

PropertyPrivate *property_new_double(gdouble defval, gdouble min, gdouble max,
	gdouble step)
{
	PropertyPrivate *priv;
	PropertyPrivateDouble *dpriv;

	priv = g_new0(PropertyPrivate, 1);
	priv->type = PROPERTY_DOUBLE;
	priv->signal_handler = -1;
	dpriv = g_new0(PropertyPrivateDouble, 1);
	dpriv->defval = defval;
	dpriv->min = min;
	dpriv->max = max;
	priv->custom = dpriv;
	priv->widget = gtk_spin_button_new_with_range(min, max, step);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(priv->widget), 3);
	priv->signal_widget = priv->widget;

	return priv;
}

static void double_spin_changed_cb(GtkSpinButton *spinbutton, gpointer data)
{
	*((gdouble *)data) = gtk_spin_button_get_value(spinbutton);
}

static gboolean double_update_handler(PropertyPrivate *priv, gdouble *var)
{
	PropertyPrivateDouble *dpriv = (PropertyPrivateDouble *)priv->custom;

	priv->signal_handler = g_signal_connect(G_OBJECT(priv->signal_widget),
		"value-changed", G_CALLBACK(double_spin_changed_cb),
		((var != NULL) ? var : &(dpriv->defval)));

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->signal_widget),
		((var != NULL) ? *var : dpriv->defval));

	return TRUE;
}

/*****************************************************************************/
/* integer                                                                   */
/*****************************************************************************/

typedef struct {
	gint32 defval;
	gint32 min;
	gint32 max;
} PropertyPrivateInteger;

PropertyPrivate *property_new_integer(gint32 defval, gint32 min, gint32 max)
{
	PropertyPrivate *priv;
	PropertyPrivateInteger *ipriv;

	priv = g_new0(PropertyPrivate, 1);
	priv->type = PROPERTY_INTEGER;
	priv->signal_handler = -1;
	ipriv = g_new0(PropertyPrivateInteger, 1);
	ipriv->defval = defval;
	ipriv->min = min;
	ipriv->max = max;
	priv->custom = ipriv;
	priv->widget = gtk_spin_button_new_with_range(min, max, 1);
	priv->signal_widget = priv->widget;

	return priv;
}

static void integer_spin_changed_cb(GtkSpinButton *spinbutton, gpointer data)
{
	*((gint32 *)data) = gtk_spin_button_get_value_as_int(spinbutton);
}

static gboolean integer_update_handler(PropertyPrivate *priv, gint32 *var)
{
	PropertyPrivateInteger *ipriv = (PropertyPrivateInteger *)priv->custom;

	priv->signal_handler = g_signal_connect(G_OBJECT(priv->signal_widget),
		"value-changed", G_CALLBACK(integer_spin_changed_cb),
		((var != NULL) ? var : &(ipriv->defval)));

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->signal_widget),
		((var != NULL) ? *var : ipriv->defval));

	return TRUE;
}

/*****************************************************************************/
/* color                                                                     */
/*****************************************************************************/

typedef struct {
	guint32 defval;
	GdkColor *color;
} PropertyPrivateColor;

static void color_uint32_to_gdk(GdkColor *gdkcolor, guint32 color)
{
	gdkcolor->red   = ((color & 0xFF000000) >> 24) << 8;
	gdkcolor->green = ((color & 0x00FF0000) >> 16) << 8;
	gdkcolor->blue  = ((color & 0x0000FF00) >> 8)  << 8;
}

static guint32 color_gdk_to_uint32(GdkColor *gdkcolor)
{
	return 0xFF +
		((gdkcolor->red   >> 8) << 24) +
		((gdkcolor->green >> 8) << 16) +
		((gdkcolor->blue  >> 8) << 8);
}

PropertyPrivate *property_new_color(guint32 defval)
{
	PropertyPrivate *priv;
	PropertyPrivateColor *cpriv;

	priv = g_new0(PropertyPrivate, 1);
	priv->type = PROPERTY_COLOR;
	priv->signal_handler = -1;
	cpriv = g_new0(PropertyPrivateColor, 1);
	cpriv->defval = defval;
	cpriv->color = g_new0(GdkColor, 1);
	color_uint32_to_gdk(cpriv->color, cpriv->defval);
	priv->custom = cpriv;
	priv->widget = gtk_color_button_new_with_color(cpriv->color);
	priv->signal_widget = priv->widget;

	return priv;
}

static void color_set_cb(GtkColorButton *widget, gpointer data)
{
	guint32 *var = (guint32 *)data;
	GdkColor color;

	gtk_color_button_get_color(widget, &color);
	*var = color_gdk_to_uint32(&color);
}

static gboolean color_update_handler(PropertyPrivate *priv, guint32 *var)
{
	PropertyPrivateColor *cpriv = (PropertyPrivateColor *)priv->custom;

	priv->signal_handler = g_signal_connect(G_OBJECT(priv->signal_widget),
		"color-set", G_CALLBACK(color_set_cb),
		((var != NULL) ? var : &(cpriv->defval)));

	color_uint32_to_gdk(cpriv->color, ((var != NULL) ? *var : cpriv->defval));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(priv->widget), cpriv->color);

	return TRUE;
}
