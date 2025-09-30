#ifndef TEXTURE_H
#define TEXTURE_H

#include "c3m.h"

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

int create_texture(int width, int height, int channels, unsigned char* data);
void delete_texture(int id);
texture_t* get_texture(int id);
u32 sample_texture(void);

#endif
