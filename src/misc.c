#include "misc.h"
#include <math.h>

#define G(i) (gdouble)(i)

gdouble misc_angle(gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
	gdouble at;

	if(x1 == x2)
	{
		if(y1 > y2)
			return G_PI * 1.5;
		else
			return G_PI / 2.0;
	}

	at = atan((G(y2) - G(y1)) / (G(x2) - G(x1)));
	if(x2 < x1)
		return G_PI + at;
	else
		return at;
}

gdouble misc_delta(gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
	gdouble a, b;

	a = ABS(G(x2) - G(x1));
	b = ABS(G(y2) - G(y1));
	return sqrt(a * a + b * b);
}

gdouble misc_delta_p(gdouble x1, gdouble y1, gdouble x2, gdouble y2,
	gdouble rx, gdouble ry)
{
	gdouble nx, ny, n0x, n0y, nd, dx, dy;

	nx = -(y2 - y1);
	ny = (x2 - x1);
	nd = sqrt(nx * nx + ny * ny);
	n0x = nx / nd;
	n0y = ny / nd;

#if 0
	g_debug("MISC: n (%.2f, %.2f), n0=(%.2f, %.2f), nd=%.2f",
		nx, ny, n0x, n0y, nd);
#endif
	dx = rx - x1;
	dy = ry - y1;

	return ABS(dx * n0x + dy * n0y);
}
