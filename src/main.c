#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

#include "main.h"
#include "gui.h"

int main(int argc, char *argv[])
{
	HBGui *gui;

	gui = gui_init(&argc, &argv);
	gui_run(gui);

	return EXIT_SUCCESS;
}
