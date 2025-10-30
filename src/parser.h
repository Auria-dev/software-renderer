#ifndef PARSER_H
#define PARSER_H

#include "mesh.h"
#include "c3m.h"
#include "materials.h"
#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char name[128];
    int material_id;
} material_lookup_t;

void load_obj(const char* path, mesh_t* mesh, material_manager_t* m);
int load_mtl(const char* mtl_path, const char* obj_dir, material_manager_t* m, material_lookup_t** lookup_table_out);

#endif // PARSER_H