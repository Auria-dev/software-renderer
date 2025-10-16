#ifndef LAODER_H
#define LAODER_H

#include "mesh.h"
#include "c3m.h"
#include "textures.h"
#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char name[128];
    int texture_id;
} material_lookup_t;

mesh_t load_mesh(const char *filepath);
void parse_obj_file(void);
void parse_mtl_file(void);
void load_obj(const char* path, mesh_t* mesh, texture_manager_t* tm);
int load_mtl(const char* mtl_path, const char* obj_dir, texture_manager_t* tm, material_lookup_t** lookup_table_out);

texture_t parse_png_file(const char *filepath);
texture_t parse_jpg_file(const char *filepath);
texture_t parse_bmp_file(const char *filepath);

#endif
