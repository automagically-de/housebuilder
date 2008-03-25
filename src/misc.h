#ifndef _MISC_H
#define _MISC_H

#include <glib.h>

#define RAD2DEG(f) ((f) * 180.0 / G_PI)
#define DEG2RAD(f) ((f) * G_PI / 180.0)

#define MM2PT(f) ((f) * 72.0 / 2.54)

gdouble misc_angle(gdouble x1, gdouble y1, gdouble x2, gdouble y2);
gdouble misc_delta(gdouble x1, gdouble y1, gdouble x2, gdouble y2);
gdouble misc_delta_p(gdouble x1, gdouble y1, gdouble x2, gdouble y2,
	gdouble rx, gdouble ry);


#endif /* _MISC_H */
