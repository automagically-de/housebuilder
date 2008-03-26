#ifndef _ACTIONS_H
#define _ACTIONS_H

#include "tools.h"
#include "view2d.h"

/* GtkActionEntry:
 * name, stock id, label, accelerator, tooltip, callback */

static GtkActionEntry action_entries[] =
{
	{ "FileMenuAction", NULL, "_File" },
	{ "QuitAction", GTK_STOCK_QUIT,
		"_Quit", "<control>Q",
		"Quit",
		G_CALLBACK(gtk_main_quit) }
};
static guint32 action_num_entries = G_N_ELEMENTS(action_entries);

static GtkActionEntry action_view2d_entries[] =
{
	{ "UpperFloorAction", GTK_STOCK_GO_UP,
		"_Upper Floor", "+",
		"Upper Floor",
		G_CALLBACK(view2d_upper_floor_cb) },
	{ "LowerFloorAction", GTK_STOCK_GO_DOWN,
		"_Lower Floor", "-",
		"Lower Floor",
		G_CALLBACK(view2d_lower_floor_cb) }
};
static guint32 action_view2d_num_entries = G_N_ELEMENTS(action_view2d_entries);

/* GtkRadioActionEntry:
 * name, stock id, label, accelerator, tooltip, value */

static GtkRadioActionEntry action_radio_entries[] =
{
	{ "ToolSelectAction", GTK_STOCK_INDEX, "_Select", NULL,
		"tools_select", TOOL_SELECT },
	{ "ToolDeleteAction", GTK_STOCK_DELETE, "_Delete", NULL,
		"tools_delete", TOOL_DELETE }
};
static guint32 action_num_radio_entries = G_N_ELEMENTS(action_radio_entries);

gboolean actions_radio_cb(GtkRadioButton *rb, gpointer user_data);

#endif /* _ACTIONS_H */
