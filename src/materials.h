#ifndef TEXTURES_H
#define TEXTURES_H

#include "c3m.h"
#include <string.h>
#include <stdio.h>

#define MAX_TEXTURES 64
typedef struct {
  int width, height;
  int channels;
  unsigned char* data;
} texture_t;

typedef struct {
  texture_t textures[MAX_TEXTURES];
  bool used[MAX_TEXTURES];
} material_manager_t;

material_manager_t m_init();
void m_free(material_manager_t* m);
void m_parse_texture_file(const char* filename, int* width, int* height, int* channels, unsigned char** data);
int m_create_texture(material_manager_t* m, int width, int height, int channels, unsigned char* data);
void m_delete_texture(material_manager_t* m, int id);
texture_t* m_get_texture(material_manager_t* m, int id);
u32 m_sample_texture(void);

#endif // TEXTURES_H