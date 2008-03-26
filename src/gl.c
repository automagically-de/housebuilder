#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <g3d/g3d.h>
#include <g3d/matrix.h>

#include "house.h"
#include "gl.h"
#include "trackball.h"

#define TRAP_GL_ERROR(text) \
	error = glGetError(); \
	if(error != GL_NO_ERROR) \
		g_printerr("[gl] %s: E: %d\n", text, error);

GdkGLConfig *gl_get_config(void)
{
	GdkGLConfig *glconfig;

	glconfig = gdk_gl_config_new_by_mode(
		GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);

	if(glconfig == NULL) {
		glconfig = gdk_gl_config_new_by_mode(
			GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH |
			GDK_GL_MODE_ALPHA | GDK_GL_MODE_DOUBLE);
		g_debug("gl_init: fallback to GDK_GL_MODE_ALPHA");
	}
	if(glconfig == NULL)
		return NULL;

	return glconfig;
}

void gl_init(void)
{
	GLenum error;
	GLfloat light0_pos[4] = { -50.0, 50.0, 0.0, 0.0 };
	GLfloat light0_col[4] = { 0.6, 0.6, 0.6, 1.0 };
	GLfloat light1_pos[4] = {  50.0, 50.0, 0.0, 0.0 };
	GLfloat light1_col[4] = { 0.4, 0.4, 0.4, 1.0 };
	GLfloat ambient_lc[4] = { 0.35, 0.35, 0.35, 1.0 };

	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	TRAP_GL_ERROR("gl_init - alpha, blend, depth");

	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_lc);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  light0_col);
	glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  light1_col);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light1_col);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);
	TRAP_GL_ERROR("gl_init - light stuff");

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	TRAP_GL_ERROR("gl_init - color material");

	glEnable(GL_TEXTURE_2D);
	TRAP_GL_ERROR("gl_init - enable texture");
}

gboolean gl_update_material(G3DMaterial *material)
{
	g_return_val_if_fail(material != NULL, FALSE);

	g_debug("material updated");

	glColor4f(material->r, material->g, material->b, material->a);
	glMaterialfv(GL_FRONT, GL_SPECULAR, material->specular);
	glMaterialf(GL_FRONT, GL_SHININESS, material->shininess * 10);
	return TRUE;
}

gboolean gl_draw_object(G3DObject *object)
{
	G3DFace *face;
	GSList *iface;
	gboolean req_end = FALSE;
	gint32 prev_vcnt = -1;
	gint32 i, index;
	static G3DMaterial *prev_mat = NULL;

	g_debug("gl_draw_object called");

	for(iface = object->faces; iface != NULL; iface = iface->next) {
		face = (G3DFace *)iface->data;

		/* update material */
		if(face->material != prev_mat) {
			if(req_end) {
				glEnd();
				req_end = FALSE;
				prev_vcnt = -1;
			}
			gl_update_material(face->material);
			prev_mat = face->material;
		}

		/* update polygon type */
		if(face->vertex_count != prev_vcnt) {
			if(req_end) {
				glEnd();
				req_end = FALSE;
			}
			prev_vcnt = face->vertex_count;
			if(prev_vcnt == 3) {
				glBegin(GL_TRIANGLES);
				req_end = TRUE;
			} else if(prev_vcnt == 4) {
				glBegin(GL_QUADS);
				req_end = TRUE;
			} else if(prev_vcnt > 4) {
				glBegin(GL_POLYGON);
				req_end = TRUE;
			}
		}

		/* vertices for one face */
		if(face->vertex_count >= 3) {
			for(i = 0; i < face->vertex_count; i ++) {
				if(face->normals)
					glNormal3f(
						face->normals[i * 3 + 0],
						face->normals[i * 3 + 1],
						face->normals[i * 3 + 2]);
				index = face->vertex_indices[i];
				glVertex3f(
					object->vertex_data[index * 3 + 0],
					object->vertex_data[index * 3 + 1],
					object->vertex_data[index * 3 + 2]);

#if DEBUG > 1
				g_debug("vertex (%f, %f, %f)",
					object->vertex_data[index * 3 + 0],
					object->vertex_data[index * 3 + 1],
					object->vertex_data[index * 3 + 2]);
#endif
			}
		} /* vertex count >= 3 */
	} /* faces */

	if(req_end)
		glEnd();

	return TRUE;
}

gboolean gl_draw_objects(GSList *objects)
{
	GSList *item;

	for(item = objects; item != NULL; item = item->next) {
		glPushMatrix();
		gl_draw_object((G3DObject *)item->data);
		glPopMatrix();
	}
	return TRUE;
}

gboolean gl_rebuild_list(HBHouse *house)
{
	if(house->dlist >= 0)
		glDeleteLists(house->dlist, 1);
	house->dlist = glGenLists(1);

	glNewList(house->dlist, GL_COMPILE);
	gl_draw_objects(house->model->objects);
	glEndList();
	return TRUE;
}

gboolean gl_draw(HBHouse *house, gdouble zoom, gdouble aspect, gfloat *quat,
	gdouble offx, gdouble offy)
{
	GLenum error;
	static gboolean initialized = FALSE;
	gfloat tmat[16];
	GLfloat m[4][4];
	static gint32 dlist_gnd = -1;
	gint32 x, y;

	if(!initialized) {
		gl_init();
		initialized = TRUE;
	}

	if(dlist_gnd == -1) {
		dlist_gnd = glGenLists(1);
		glNewList(dlist_gnd, GL_COMPILE);
		glColor4f(0.9, 0.9, 0.9, 0.7);
		glBegin(GL_LINES);
		for(x = -100.0; x <= 100.0; x += 5) {
			glVertex3f(x, 0.0, -100.0);
			glVertex3f(x, 0.0,  100.0);
		}
		for(y = -100.0; y <= 100.0; y += 5) {
			glVertex3f(-100.0, 0.0, y);
			glVertex3f( 100.0, 0.0, y);
		}
		glEnd();
		glEndList();
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(zoom, aspect, 0.1, 1000);

	g3d_matrix_identity(tmat);
	g3d_matrix_translate(offx, offy, 0.0, tmat);
	glMultMatrixf(tmat);
	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(1.0);
	glClearIndex(0.3);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
		GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -100);
	build_rotmatrix(m, quat);
	glMultMatrixf(&m[0][0]);

	glBindTexture(GL_TEXTURE_2D, 0);
	TRAP_GL_ERROR("glBindTexture");

	if(house == NULL)
		return FALSE;

	if(house->dirty || (house->dlist == -1)) {
		gl_rebuild_list(house);
		TRAP_GL_ERROR("gl_rebuild_list");
		house->dirty = FALSE;
	}

	glTranslated(-house->off_x, -house->off_y, -house->off_z);
	glScaled(house->scale, house->scale, house->scale);

	glCallList(dlist_gnd);
	glCallList(house->dlist);
	TRAP_GL_ERROR("glCallList");

	return TRUE;
}
