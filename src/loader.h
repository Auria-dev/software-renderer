#ifndef LAODER_H
#define LAODER_H

#include "mesh.h"
#include "c3m.h"
#include "texture.h"
#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void load_mesh(void);
void parse_obj_file(void);
void parse_mtl_file(void);

void parse_png_file(void);
void parse_jpg_file(void);
void parse_bmp_file(void);

#endif