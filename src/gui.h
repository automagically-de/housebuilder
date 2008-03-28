#ifndef _GUI_H
#define _GUI_H

#include "house.h"
#include "texture.h"

typedef struct _HBGui HBGui;

HBGui *gui_init(int *argcp, char ***argvp);
gboolean gui_run(HBGui *gui);

HBHouse *gui_get_house(HBGui *gui);
HBTextureLoader *gui_get_texture_loader(HBGui *gui);
G3DContext *gui_get_g3d_context(HBGui *gui);

#endif /* _GUI_H */
