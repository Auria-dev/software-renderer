#include "materials.h"

#include "stb_image.h"

material_manager_t m_init() {
    material_manager_t m = {0};
    for (int i = 0; i < MAX_TEXTURES; i++) {
        m.used[i] = false;
        m.textures[i].data = NULL;
        m.textures[i].width = 0;
        m.textures[i].height = 0;
        m.textures[i].channels = 0;
    }
    return m;
}

void m_free(material_manager_t* m) {
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (m->used[i] && m->textures[i].data) {
            free(m->textures[i].data);
            m->textures[i].data = NULL;
            m->used[i] = false;
        }
    }
}

void m_parse_texture_file(const char* filename, int* width, int* height, int* channels, unsigned char** data) {
    int req_channels = 4;
    int orig_channels = 0;
    unsigned char* img = stbi_load(filename, width, height, &orig_channels, req_channels);

    if (!img) {
        fprintf(stderr, "ERROR: failed to load texture '%s': %s\n", filename, stbi_failure_reason());
        *width = 0;
        *height = 0;
        *channels = 0;
        *data = NULL;
        return;
    }

    *channels = req_channels;
    *data = img;

    printf("INFO: loaded texture '%s' (%dx%d) orig_channels=%d -> returned_channels=%d\n",
           filename, *width, *height, orig_channels, *channels);
}


int m_create_texture(material_manager_t* m, int width, int height, int channels, unsigned char* data) {
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (!m->used[i]) {
            m->textures[i].width = width;
            m->textures[i].height = height;
            m->textures[i].channels = channels;
            m->textures[i].data = data;
            m->used[i] = true;
            printf("INFO: Created texture with ID %d\n", i);
            return i;
        }
    }
    printf("WARNING: Texture manager is full. Could not create texture.\n");
    if(data) free(data);
    return -1;
}

void m_delete_texture(material_manager_t* m, int id) {
    if (id >= 0 && id < MAX_TEXTURES && m->used[id]) {
        if (m->textures[id].data) {
            free(m->textures[id].data);
            m->textures[id].data = NULL;
        }
        m->used[id] = false;
    }
}

texture_t* m_get_texture(material_manager_t* m, int id) {
    if (id >= 0 && id < MAX_TEXTURES && m->used[id]) {
        return &m->textures[id];
    }
    return NULL;
}

u32 m_sample_texture(void) {
    return 0;
}
