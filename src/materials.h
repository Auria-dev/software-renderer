#ifndef MATERIALS_H
#define MATERIALS_H

#include "c3m.h"
#include <string.h>
#include <stdio.h>

#define MAX_TEXTURES 128
#define MAX_MATERIALS 128

typedef struct {
  int width, height;
  int channels;
  unsigned char* data;
} texture_t;

typedef struct {
    char name[128];
    vec3 ambient;       // Ka
    vec3 diffuse;       // Kd
    vec3 specular;      // Ks
    float shininess;    // Ns
    int diffuse_map_id; // map_Kd (this is an ID into the textures array)

    u32 color;  // debug material color
} material_t;

typedef struct {
  texture_t textures[MAX_TEXTURES];
  bool texture_used[MAX_TEXTURES];

  material_t materials[MAX_MATERIALS];
  bool material_used[MAX_MATERIALS];
} material_manager_t;

material_manager_t m_init();
void m_free(material_manager_t* m);

void m_parse_texture_file(const char* filename, int* width, int* height, int* channels, unsigned char** data);
int m_create_texture(material_manager_t* m, int width, int height, int channels, unsigned char* data);
void m_delete_texture(material_manager_t* m, int id);
texture_t* m_get_texture(material_manager_t* m, int id);

int m_create_material(material_manager_t* m, const char* name);
void m_delete_material(material_manager_t* m, int id);
material_t* m_get_material(material_manager_t* m, int id);

#endif // MATERIALS_H