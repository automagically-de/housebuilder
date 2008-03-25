#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include "gui.h"
#include "house.h"
#include "view2d.h"
#include "view3d.h"

struct _HBGui {
	GtkWidget *window;
	HBHouse *house;

	HBView *view2d;
	HBView *view3d;
};

HBGui *gui_init(int *argcp, char ***argvp)
{
	GtkWidget *window, *nb, *vbox, *label;
	HBGui *gui;

	gtk_init(argcp, argvp);

	gui = g_new0(HBGui, 1);

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

	nb = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(vbox), nb, TRUE, TRUE, 0);

	/* add views */
	gui->view2d = view2d_new();
	gui->view3d = view3d_new();
	gui->view2d->gui = gui->view3d->gui = gui;

	label = gtk_label_new_with_mnemonic("_2D view");
	gtk_notebook_append_page(GTK_NOTEBOOK(nb), gui->view2d->widget, label);

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
