#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <g3d/g3d.h>

#include "gui.h"
#include "house.h"
#include "view2d.h"
#include "view3d.h"
#include "actions.h"

struct _HBGui {
	GtkWidget *window;
	HBHouse *house;

	HBView *view2d;
	HBView *view3d;

	G3DContext *context;
};

HBGui *gui_init(int *argcp, char ***argvp)
{
	GtkWidget *window, *nb, *vbox, *hbox, *label;
	GtkWidget *menubar, *toolbar;
	GtkUIManager *ui;
	GtkActionGroup *ag;
	GError *error = NULL;
	HBGui *gui;

	gtk_init(argcp, argvp);

	gui = g_new0(HBGui, 1);

	/* g3d initialization */
	gui->context = g3d_context_new();

	/* create main window */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gui->window = window;
	gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
	gtk_window_set_title(GTK_WINDOW(window), "HouseBuilder");
	gtk_window_set_icon_from_file(GTK_WINDOW(window),
		DATA_DIR "/icons/housebuilder48.png", NULL);
	g_signal_connect(G_OBJECT(window), "delete-event",
		G_CALLBACK(gtk_main_quit), NULL);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* create view widgets - pointer needed for ui.xml */
	gui->view2d = view2d_new();
	gui->view3d = view3d_new();
	gui->view2d->gui = gui->view3d->gui = gui;

	/* UI from xml */
	ui = gtk_ui_manager_new();
	ag = gtk_action_group_new("MainActions");
	gtk_action_group_set_translation_domain(ag, "housebuilder");
	gtk_action_group_add_actions(ag, action_entries, action_num_entries, gui);
	gtk_action_group_add_actions(ag, action_view2d_entries,
		action_view2d_num_entries, gui->view2d);
	gtk_action_group_add_radio_actions(ag,
		action_radio_entries, action_num_radio_entries,
		-1, G_CALLBACK(actions_radio_cb), gui);
	gtk_ui_manager_insert_action_group(ui, ag, 0);
	gtk_ui_manager_add_ui_from_file(ui,
		DATA_DIR "/ui.xml", &error);
	if(error != NULL) {
		g_warning("building menus failed: %s", error->message);
		g_error_free(error);
	}
	gtk_ui_manager_ensure_update(ui);
	gtk_window_add_accel_group(GTK_WINDOW(window),
		gtk_ui_manager_get_accel_group(ui));

	/* menu bar */
	menubar = gtk_ui_manager_get_widget(ui, "/MainMenu");
	gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

	/* add views */
	nb = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(vbox), nb, TRUE, TRUE, 0);

	/* 2D page stuff */
	vbox = gtk_vbox_new(FALSE, 0);

	/* add 2D toolbar */
	toolbar = gtk_ui_manager_get_widget(ui, "/View2DToolbar");
	gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, FALSE, 0);

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	toolbar = gtk_ui_manager_get_widget(ui, "/ToolsToolbar");
	gtk_toolbar_set_orientation(GTK_TOOLBAR(toolbar),
		GTK_ORIENTATION_VERTICAL);
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
	gtk_box_pack_start(GTK_BOX(hbox), toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gui->view2d->widget, TRUE, TRUE, 5);

	/* add 2D view */
	label = gtk_label_new_with_mnemonic("_2D view");
	gtk_notebook_append_page(GTK_NOTEBOOK(nb), vbox, label);

	/* add 3D view */
	label = gtk_label_new_with_mnemonic("_3D view");
	gtk_notebook_append_page(GTK_NOTEBOOK(nb), gui->view3d->widget, label);

	return gui;
}

gboolean gui_run(HBGui *gui)
{
	gtk_widget_show_all(gui->window);
	gtk_main();

	return TRUE;
}

HBHouse *gui_get_house(HBGui *gui)
{
	if(gui->house == NULL) {
		gui->house = g_new0(HBHouse, 1);
		gui->house->dlist = -1;

		gui->house->model = g_new0(G3DModel, 1);
		gui->house->model->filename = g_strdup("house");
		gui->house->model->context = gui->context;
	}
	return gui->house;
}
