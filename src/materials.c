#include "materials.h"

#include "stb_image.h"

material_manager_t m_init() {
    material_manager_t m = {0};
    for (int i = 0; i < MAX_TEXTURES; i++) {
        m.texture_used[i] = false;
        m.textures[i].data = NULL;
        m.textures[i].width = 0;
        m.textures[i].height = 0;
        m.textures[i].channels = 0;
    }

    for (int i = 0; i < MAX_MATERIALS; i++) {
        m.material_used[i] = false;
        m.materials[i] = (material_t){0};
    }
    return m;
}

void m_free(material_manager_t* m) {
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (m->texture_used[i] && m->textures[i].data) {
            free(m->textures[i].data);
            m->textures[i].data = NULL;
            m->texture_used[i] = false;
        }
    }

    for (int i = 0; i < MAX_MATERIALS; i++) {
        m->material_used[i] = false;
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
}

int m_create_texture(material_manager_t* m, int width, int height, int channels, unsigned char* data) {
    for (int i = 0; i < MAX_TEXTURES; i++) {
        if (!m->texture_used[i]) {
            m->textures[i].width = width;
            m->textures[i].height = height;
            m->textures[i].channels = channels;
            m->textures[i].data = data;
            m->texture_used[i] = true;
            return i;
        }
    }
    printf("WARNING: Texture manager is full. Could not create texture.\n");
    if(data) free(data);
    return -1;
}

void m_delete_texture(material_manager_t* m, int id) {
    if (id >= 0 && id < MAX_TEXTURES && m->texture_used[id]) {
        if (m->textures[id].data) {
            free(m->textures[id].data);
            m->textures[id].data = NULL;
        }
        m->texture_used[id] = false;
    }
}

texture_t* m_get_texture(material_manager_t* m, int id) {
    if (id >= 0 && id < MAX_TEXTURES && m->texture_used[id]) {
        return &m->textures[id];
    }
    return NULL;
}

int m_create_material(material_manager_t* m, const char* name) {
    for (int i = 0; i < MAX_MATERIALS; i++) {
        if (!m->material_used[i]) {
            m->material_used[i] = true;
            material_t* mat = &m->materials[i];

            // Set name
            strncpy(mat->name, name, sizeof(mat->name) - 1);
            mat->name[sizeof(mat->name) - 1] = '\0';

            // Set default MTL values
            mat->ambient = (vec3){0.1f, 0.1f, 0.1f};
            mat->diffuse = (vec3){0.8f, 0.8f, 0.8f};
            mat->specular = (vec3){0.0f, 0.0f, 0.0f};
            mat->shininess = 32.0f;
            mat->diffuse_map_id = -1;

            printf("INFO: Created material '%s' with ID %d\n", name, i);
            return i;
        }
    }
    printf("WARNING: Material manager is full. Could not create material.\n");
    return -1;
}

void m_delete_material(material_manager_t* m, int id) {
    if (id >= 0 && id < MAX_MATERIALS && m->material_used[id]) {
        m->material_used[id] = false;
        // Note: We don't delete the texture here. Textures are shared.
    }
}

material_t* m_get_material(material_manager_t* m, int id) {
    if (id >= 0 && id < MAX_MATERIALS && m->material_used[id]) {
        return &m->materials[id];
    }
    return NULL;
}