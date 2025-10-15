#ifndef TEXTURE_H
#define TEXTURE_H

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
} texture_manager_t;

texture_manager_t tm_init();
void tm_free(texture_manager_t* tm);
void tm_parse_texture_file(const char* filename, int* width, int* height, int* channels, unsigned char** data);
int tm_create_texture(texture_manager_t* tm, int width, int height, int channels, unsigned char* data);
void tm_delete_texture(texture_manager_t* tm, int id);
texture_t* tm_get_texture(texture_manager_t* tm, int id);
u32 tm_sample_texture(void);

#endif
