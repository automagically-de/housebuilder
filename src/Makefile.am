bin_PROGRAMS = housebuilder

housebuilder_SOURCES = \
	main.c main.h \
	gui.c gui.h \
	view.h \
	view2d.c view2d.h \
	view3d.c view3d.h \
	house.c house.h \
	node.c node.h \
	part_wall.c part_wall.h \
	layers.h \
	property.c property.h \
	misc.c misc.h

housebuilder_CFLAGS = \
	-I$(top_srcdir) -I$(top_builddir) \
	-Wall -ansi -pedantic \
	-DDEBUG=@DEBUG@ \
	@DBG_CFLAGS@ \
	@GLIB_CFLAGS@ @GTK_CFLAGS@ \
	@CAIRO_CFLAGS@ @RSVG_CFLAGS@ \
	@GTKGLEXT_CFLAGS@ @X_CFLAGS@ \
	@LIBG3D_CFLAGS@

housebuilder_LDFLAGS = \
	@GLIB_LIBS@ @GTK_LIBS@ \
	@CAIRO_LIBS@ @RSVG_LIBS@ \
	@GTKGLEXT_LIBS@ @X_LIBS@ \
	@GL_LIBS@ @LIBG3D_LIBS@


