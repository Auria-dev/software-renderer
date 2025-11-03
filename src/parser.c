#include "parser.h"

char* trim_whitespace(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static u32 find_or_add_vertex(
    mesh_t* mesh,
    vertex_t new_vertex
) {
    for (int i = 0; i < mesh->vertex_count; i++) {
        if (memcmp(&mesh->vertices[i], &new_vertex, sizeof(vertex_t)) == 0) {
            return i; 
        }
    }

    u32 new_index = mesh->vertex_count;
    array_push(mesh->vertices, new_vertex);
    mesh->vertex_count++;
    
    return new_index;
}

void load_obj(const char* path, mesh_t* mesh, material_manager_t* m) {
    FILE* file = fopen(path, "r");
    if (!file) {
        printf("ERROR: Unable to open OBJ file: %s\n", path);
        return;
    }

    vec3* temp_positions = NULL;
    vec2* temp_texcoords = NULL;
    vec3* temp_normals = NULL;

    mesh->vertices = NULL;
    mesh->submeshes = NULL;
    mesh->vertex_count = 0;
    mesh->submesh_count = 0;
    
    material_lookup_t* material_lookups = NULL;
    int material_lookup_count = 0;
    int current_submesh_index = -1;

    char obj_dir[512] = "./";
    const char* last_slash = strrchr(path, '/');
    if (last_slash) {
        strncpy(obj_dir, path, last_slash - path + 1);
        obj_dir[last_slash - path + 1] = '\0';
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0) continue;

        // skip comments
        if (trimmed[0] == '#') continue;

        if (strncmp(trimmed, "v ", 2) == 0) {
            vec3 pos;
            sscanf(trimmed + 2, "%f %f %f", &pos.x, &pos.y, &pos.z);
            array_push(temp_positions, pos);
        } else if (strncmp(trimmed, "vt ", 3) == 0) {
            vec2 uv;
            sscanf(trimmed + 3, "%f %f", &uv.x, &uv.y);
            uv.y = 1.0f - uv.y;
            array_push(temp_texcoords, uv);
        } else if (strncmp(trimmed, "vn ", 3) == 0) {
            vec3 norm;
            sscanf(trimmed + 3, "%f %f %f", &norm.x, &norm.y, &norm.z);
            array_push(temp_normals, norm);
        } else if (strncmp(trimmed, "mtllib ", 7) == 0) {
            char mtl_filename[256];
            strncpy(mtl_filename, trimmed + 7, sizeof(mtl_filename) - 1);
            mtl_filename[sizeof(mtl_filename) - 1] = '\0';

            char mtl_path[512];
            snprintf(mtl_path, sizeof(mtl_path), "%s%s", obj_dir, mtl_filename);
            material_lookup_count = load_mtl(mtl_path, obj_dir, m, &material_lookups);
        } else if (strncmp(trimmed, "usemtl ", 7) == 0) {
            char mtl_name[128];
            strncpy(mtl_name, trimmed + 7, sizeof(mtl_name) - 1);
            mtl_name[sizeof(mtl_name) - 1] = '\0';
            
            int material_id = -1;
            for (int i = 0; i < material_lookup_count; i++) {
                if (strcmp(material_lookups[i].name, mtl_name) == 0) {
                    material_id = material_lookups[i].material_id;
                    break;
                }
            }
            
            current_submesh_index = -1;
            for (int i = 0; i < mesh->submesh_count; i++) {
                if (mesh->submeshes[i].material_id == material_id) {
                    current_submesh_index = i;
                    break;
                }
            }

            if (current_submesh_index == -1) {
                submesh_t new_sub = { .indices = NULL, .index_count = 0, .material_id = material_id };
                array_push(mesh->submeshes, new_sub);
                mesh->submesh_count++;
                current_submesh_index = mesh->submesh_count - 1;
            }

        } else if (strncmp(trimmed, "f ", 2) == 0) {
            if (current_submesh_index < 0) {
                 submesh_t default_sub = { .indices = NULL, .index_count = 0, .material_id = -1 };
                 array_push(mesh->submeshes, default_sub);
                 mesh->submesh_count++;
                 current_submesh_index = mesh->submesh_count - 1;
            }
            
            int p[3], t[3], n[3]; // position, texcoord, normal indices
            int matches = sscanf(trimmed + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", 
                                 &p[0], &t[0], &n[0], &p[1], &t[1], &n[1], &p[2], &t[2], &n[2]);

            if (matches != 9) {
                printf("WARNING: Unsupported face format: %s. Only v/vt/vn is supported.\n", trimmed);
                continue;
            }

            submesh_t* current_submesh = &mesh->submeshes[current_submesh_index];
            for (int i = 0; i < 3; i++) {
                vertex_t vertex = {0};
                vertex.position = temp_positions[p[i] - 1];
                vertex.texcoord = temp_texcoords[t[i] - 1];
                vertex.normal   = temp_normals[n[i] - 1];
                
                u32 index = find_or_add_vertex(mesh, vertex);
                
                array_push(current_submesh->indices, index);
                current_submesh->index_count++;
            }
        }
    }

    fclose(file);

    array_free(temp_positions);
    array_free(temp_texcoords);
    array_free(temp_normals);
    array_free(material_lookups);

    printf("INFO: Loaded OBJ: %d vertices, %d submeshes\n", mesh->vertex_count, mesh->submesh_count);
    for (int i = 0; i < mesh->submesh_count; i++) {
        printf("  - Submesh %d: material_id=%d, indices=%d\n", i, mesh->submeshes[i].material_id, mesh->submeshes[i].index_count);
    }
}

int load_mtl(const char* mtl_path, const char* obj_dir, material_manager_t* m, material_lookup_t** lookup_table_out) {
    FILE* file = fopen(mtl_path, "r");
    if (!file) {
        printf("ERROR: Cannot open MTL file: %s\n", mtl_path);
        return 0;
    }

    material_lookup_t* lookups = NULL;
    material_t* current_material = NULL;
    char line[1024];

    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0 || trimmed[0] == '#') continue;

        if (strncmp(trimmed, "newmtl ", 7) == 0) {
            material_lookup_t new_lookup;
            strncpy(new_lookup.name, trimmed + 7, sizeof(new_lookup.name) - 1);
            new_lookup.name[sizeof(new_lookup.name) - 1] = '\0';

            int new_mat_id = m_create_material(m, new_lookup.name);
            new_lookup.material_id = new_mat_id;
            
            array_push(lookups, new_lookup);
            current_material = m_get_material(m, new_mat_id);
            printf("DEBUG: load_mtl: material: Found new material '%s' with ID %d\n", new_lookup.name, new_mat_id);

        } else if (current_material && strncmp(trimmed, "Ka ", 3) == 0) {
            sscanf(trimmed + 3, "%f %f %f", 
                   &current_material->ambient.x, 
                   &current_material->ambient.y, 
                   &current_material->ambient.z);
            printf("- DEBUG: load_mtl: material: Found ambient color\n");
        } else if (current_material && strncmp(trimmed, "Kd ", 3) == 0) {
            sscanf(trimmed + 3, "%f %f %f", 
                   &current_material->diffuse.x, 
                   &current_material->diffuse.y, 
                   &current_material->diffuse.z);
            printf("- DEBUG: load_mtl: material: Found diffuse color\n");
        } else if (current_material && strncmp(trimmed, "Ks ", 3) == 0) {
            sscanf(trimmed + 3, "%f %f %f", 
                   &current_material->specular.x, 
                   &current_material->specular.y, 
                   &current_material->specular.z);
            printf("- DEBUG: load_mtl: material: Found specular color\n");
        } else if (current_material && strncmp(trimmed, "Ns ", 3) == 0) {
            sscanf(trimmed + 3, "%f", &current_material->shininess);
            printf("- DEBUG: load_mtl: material: Found shininess\n");
        } else if (current_material && strncmp(trimmed, "map_Kd ", 7) == 0) {
            printf("- DEBUG: load_mtl: material: Found diffuse texture map\n");

            char texture_filename[256];
            strncpy(texture_filename, trimmed + 7, sizeof(texture_filename) - 1);
            texture_filename[sizeof(texture_filename) - 1] = '\0';

            char texture_path[512];
            snprintf(texture_path, sizeof(texture_path), "%s%s", obj_dir, texture_filename);
            
            int width, height, channels;
            unsigned char* data = NULL;
            m_parse_texture_file(texture_path, &width, &height, &channels, &data);

            if (data) {
                int texture_id = m_create_texture(m, width, height, channels, data);
                current_material->diffuse_map_id = texture_id;
                if (texture_id != -1) m->texture_used[texture_id] = true;
                printf("- DEBUG: load_mtl: material: created texture id=%d\n", texture_id);
            } else {
                printf("- WARNING: load_mtl: material: Failed to load texture: %s\n", texture_path);
            }
        }
    }
    printf("\n");
    fclose(file);
    *lookup_table_out = lookups;
    return array_length(lookups);
}
