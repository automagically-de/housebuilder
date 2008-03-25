bin_PROGRAMS = housebuilder

housebuilder_SOURCES = \
	main.c main.h

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


