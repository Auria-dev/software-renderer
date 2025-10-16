#include "textures.h"

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
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (tm->used[i] && tm->textures[i].data) {
            free(tm->textures[i].data);
            tm->textures[i].data = NULL;
            tm->used[i] = false;
        }
    }
}

void tm_parse_texture_file(const char* filename, int* width, int* height, int* channels, unsigned char** data) {
    printf("INFO: Stubbed texture load for '%s'\n", filename);
    *width = 1;
    *height = 1;
    *channels = 4;
    *data = (unsigned char*)malloc(4 * sizeof(unsigned char));
    if (*data) {
        (*data)[0] = 255; (*data)[1] = 0; (*data)[2] = 255; (*data)[3] = 255;
    }
}

int tm_create_texture(texture_manager_t* tm, int width, int height, int channels, unsigned char* data) {
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (!tm->used[i]) {
            tm->textures[i].width = width;
            tm->textures[i].height = height;
            tm->textures[i].channels = channels;
            tm->textures[i].data = data;
            tm->used[i] = true;
            printf("INFO: Created texture with ID %d\n", i);
            return i;
        }
    }
    printf("Warning: Texture manager is full. Could not create texture.\n");
    if(data) free(data);
    return -1;
}

void tm_delete_texture(texture_manager_t* tm, int id) {
    if (id >= 0 && id < MAX_TEXTURES && tm->used[id]) {
        if (tm->textures[id].data) {
            free(tm->textures[id].data);
            tm->textures[id].data = NULL;
        }
        tm->used[id] = false;
    }
}

texture_t* tm_get_texture(texture_manager_t* tm, int id) {
    if (id >= 0 && id < MAX_TEXTURES && tm->used[id]) {
        return &tm->textures[id];
    }
    return NULL;
}

u32 tm_sample_texture(void) {
    return 0;
}
