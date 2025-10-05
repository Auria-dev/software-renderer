#include "texture.h"

texture_manager_t tm_init() {
    texture_manager_t tm = {0};
    for (int i = 0; i < MAX_TEXTURES; i++) {
        tm.used[i] = false;
        tm.textures[i].data = NULL;
        tm.textures[i].width = 0;
        tm.textures[i].height = 0;
        tm.textures[i].channels = 0;
    }
    return tm;
}

void tm_free(texture_manager_t* tm) {

}

void tm_parse_texture_file(const char* filename, int* width, int* height, int* channels, unsigned char** data) {

}

int tm_create_texture(int width, int height, int channels, unsigned char* data) {
    return -1;
}

void tm_delete_texture(int id) {

}

texture_t* tm_get_texture(int id) {
    return NULL;
}

u32 tm_sample_texture(void) {
    return 0;
}

