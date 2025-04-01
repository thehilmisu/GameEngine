#include "model.h"
#include "core/logger.h"
#include "core/file_operations.h"
#include "core/kstring.h"
#include "renderer/renderer_frontend.h"
#include "resources/texture.h"
#include "containers/darray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Temporary structures for OBJ loading
typedef struct {
    f32 x, y, z;
} obj_vertex;

typedef struct {
    f32 u, v;
} obj_texcoord;

typedef struct {
    f32 x, y, z;
} obj_normal;

typedef struct {
    u32 v_index;  // Vertex index
    u32 t_index;  // Texture coordinate index
    u32 n_index;  // Normal index
} obj_face_vertex;

typedef struct {
    obj_face_vertex vertices[3]; // Triangle face
} obj_face;

// Global model manager state
static u32 next_model_id = 0;
// static model** models = NULL;
// static u32 model_count = 0;
// static u32 model_capacity = 0;

// Forward declarations for internal functions
// static void register_model(model* m);
// static void unregister_model(model* m);
static char* extract_filename(const char* path);

model* model_load_obj(const char* file_path) {
    INFO("Loading OBJ model: %s", file_path);
    
    FILE* file = fopen(file_path, "r");
    if (!file) {
        ERROR("Failed to open OBJ file: %s", file_path);
        return NULL;
    }
    
    // Create darrays for temporary storage
    void* vertices = darray_create(obj_vertex);
    void* texcoords = darray_create(obj_texcoord);
    void* normals = darray_create(obj_normal);
    void* faces = darray_create(obj_face);
    
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        // Process based on line type
        if (line[0] == 'v' && line[1] == ' ') {
            // Vertex
            obj_vertex v;
            if (sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z) == 3) {
                darray_push(vertices, v);
            }
        } else if (line[0] == 'v' && line[1] == 't' && line[2] == ' ') {
            // Texture coordinate
            obj_texcoord t;
            if (sscanf(line, "vt %f %f", &t.u, &t.v) == 2) {
                darray_push(texcoords, t);
            }
        } else if (line[0] == 'v' && line[1] == 'n' && line[2] == ' ') {
            // Normal
            obj_normal n;
            if (sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z) == 3) {
                darray_push(normals, n);
            }
        } else if (line[0] == 'f' && line[1] == ' ') {
            // Face - support both triangles and quads
            int v1, t1, n1, v2, t2, n2, v3, t3, n3, v4, t4, n4;
            int matches = sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3, &v4, &t4, &n4);
            
            if (matches == 9) {
                // Triangle
                obj_face f;
                // OBJ indices are 1-based, convert to 0-based
                f.vertices[0].v_index = v1 - 1;
                f.vertices[0].t_index = t1 - 1;
                f.vertices[0].n_index = n1 - 1;
                
                f.vertices[1].v_index = v2 - 1;
                f.vertices[1].t_index = t2 - 1;
                f.vertices[1].n_index = n2 - 1;
                
                f.vertices[2].v_index = v3 - 1;
                f.vertices[2].t_index = t3 - 1;
                f.vertices[2].n_index = n3 - 1;
                
                darray_push(faces, f);
            } else if (matches == 12) {
                // Quad - split into two triangles
                obj_face f1, f2;
                
                // First triangle (v1, v2, v3)
                f1.vertices[0].v_index = v1 - 1;
                f1.vertices[0].t_index = t1 - 1;
                f1.vertices[0].n_index = n1 - 1;
                
                f1.vertices[1].v_index = v2 - 1;
                f1.vertices[1].t_index = t2 - 1;
                f1.vertices[1].n_index = n2 - 1;
                
                f1.vertices[2].v_index = v3 - 1;
                f1.vertices[2].t_index = t3 - 1;
                f1.vertices[2].n_index = n3 - 1;
                
                // Second triangle (v1, v3, v4)
                f2.vertices[0].v_index = v1 - 1;
                f2.vertices[0].t_index = t1 - 1;
                f2.vertices[0].n_index = n1 - 1;
                
                f2.vertices[1].v_index = v3 - 1;
                f2.vertices[1].t_index = t3 - 1;
                f2.vertices[1].n_index = n3 - 1;
                
                f2.vertices[2].v_index = v4 - 1;
                f2.vertices[2].t_index = t4 - 1;
                f2.vertices[2].n_index = n4 - 1;
                
                darray_push(faces, f1);
                darray_push(faces, f2);
            } else {
                // Try alternative format: f v//n v//n v//n
                matches = sscanf(line, "f %d//%d %d//%d %d//%d",
                    &v1, &n1, &v2, &n2, &v3, &n3);
                
                if (matches == 6) {
                    obj_face f;
                    f.vertices[0].v_index = v1 - 1;
                    f.vertices[0].t_index = 0; // No texture coords
                    f.vertices[0].n_index = n1 - 1;
                    
                    f.vertices[1].v_index = v2 - 1;
                    f.vertices[1].t_index = 0;
                    f.vertices[1].n_index = n2 - 1;
                    
                    f.vertices[2].v_index = v3 - 1;
                    f.vertices[2].t_index = 0;
                    f.vertices[2].n_index = n3 - 1;
                    
                    darray_push(faces, f);
                } else {
                    // Try another format: f v v v
                    matches = sscanf(line, "f %d %d %d", &v1, &v2, &v3);
                    
                    if (matches == 3) {
                        obj_face f;
                        f.vertices[0].v_index = v1 - 1;
                        f.vertices[0].t_index = 0;
                        f.vertices[0].n_index = 0;
                        
                        f.vertices[1].v_index = v2 - 1;
                        f.vertices[1].t_index = 0;
                        f.vertices[1].n_index = 0;
                        
                        f.vertices[2].v_index = v3 - 1;
                        f.vertices[2].t_index = 0;
                        f.vertices[2].n_index = 0;
                        
                        darray_push(faces, f);
                    }
                }
            }
        }
    }
    
    fclose(file);
    
    u64 vertex_count = darray_length(vertices);
    u64 texcoord_count = darray_length(texcoords);
    u64 normal_count = darray_length(normals);
    u64 face_count = darray_length(faces);
    
    INFO("OBJ loaded: %llu vertices, %llu texcoords, %llu normals, %llu faces",
         vertex_count, texcoord_count, normal_count, face_count);
    
    if (face_count == 0) {
        ERROR("No faces found in OBJ file");
        darray_destroy(vertices);
        darray_destroy(texcoords);
        darray_destroy(normals);
        darray_destroy(faces);
        return NULL;
    }
    
    // Create the model
    model* m = kallocate(sizeof(model), MEMORY_TAG_MODEL);
    m->id = next_model_id++;
    m->vertex_count = (u32)(face_count * 3); // Each face has 3 vertices
    m->vertices = kallocate(sizeof(vertex) * m->vertex_count, MEMORY_TAG_MODEL);
    m->is_indexed = FALSE;
    m->texture = NULL;  // Initialize texture pointer to NULL
    
    // Extract filename for model name
    char* filename = extract_filename(file_path);
    strncpy(m->name, filename, sizeof(m->name) - 1);
    m->name[sizeof(m->name) - 1] = '\0'; // Ensure null termination
    
    // Try to load a texture for this model
    // First, check if a texture with the same name as the model exists
    char texture_path[512];
    snprintf(texture_path, sizeof(texture_path), "assets/textures/%s.png", filename);
    
    // Try to load the texture
    texture* tex = texture_load(texture_path);
    if (!tex) {
        // Try jpg format if png failed
        snprintf(texture_path, sizeof(texture_path), "assets/textures/%s.jpg", filename);
        tex = texture_load(texture_path);
        
        if (!tex) {
            // Fall back to a default texture or create a checkerboard pattern
            INFO("Creating default checkerboard texture for model %s", filename);
            tex = texture_create_default_checkerboard();
            
            if (!tex) {
                WARN("Could not create default texture for model %s. Model will use color data only.", filename);
            } else {
                INFO("Using default checkerboard texture for model %s", filename);
            }
        } else {
            INFO("Loaded texture %s for model %s", texture_path, filename);
        }
    } else {
        INFO("Loaded texture %s for model %s", texture_path, filename);
    }
    
    m->texture = tex;
    
    kfree(filename, strlen(filename) + 1, MEMORY_TAG_STRING);
    
    // Convert faces to vertices
    obj_vertex* obj_vertices = (obj_vertex*)vertices;
    obj_texcoord* obj_texcoords = (obj_texcoord*)texcoords;
    obj_normal* obj_normals = (obj_normal*)normals;
    obj_face* obj_faces = (obj_face*)faces;
    
    for (u32 i = 0; i < face_count; i++) {
        for (u32 j = 0; j < 3; j++) {
            u32 vertex_index = i * 3 + j;
            u32 v_idx = obj_faces[i].vertices[j].v_index;
            u32 t_idx = obj_faces[i].vertices[j].t_index;
            u32 n_idx = obj_faces[i].vertices[j].n_index;
            
            // Ensure indices are valid
            if (v_idx >= vertex_count) {
                WARN("Invalid vertex index: %u (max: %llu)", v_idx, vertex_count - 1);
                v_idx = 0;
            }
            
            // Position
            m->vertices[vertex_index].position.x = obj_vertices[v_idx].x;
            m->vertices[vertex_index].position.y = obj_vertices[v_idx].y;
            m->vertices[vertex_index].position.z = obj_vertices[v_idx].z;
            
            // Texture coordinates
            if (texcoord_count > 0 && t_idx < texcoord_count) {
                m->vertices[vertex_index].tex_coords.x = obj_texcoords[t_idx].u;
                m->vertices[vertex_index].tex_coords.y = obj_texcoords[t_idx].v;
            } else {
                // Default texture coordinates
                m->vertices[vertex_index].tex_coords.x = 0.0f;
                m->vertices[vertex_index].tex_coords.y = 0.0f;
            }
            
            // Color (default to white)
            m->vertices[vertex_index].color.x = 1.0f; // R
            m->vertices[vertex_index].color.y = 1.0f; // G
            m->vertices[vertex_index].color.z = 1.0f; // B
            m->vertices[vertex_index].color.w = 1.0f; // A
            
            // Normals
            if (normal_count > 0 && n_idx < normal_count) {
                // We could store these in the vertex structure if needed
                // For now, we're using flat shading
            }
        }
    }
    
    // Create mesh for rendering
    m->mesh = renderer_create_mesh(m->vertices, m->vertex_count);
   
    
    // Register the model
    // register_model(m);
    
    INFO("%s Model '%s' loaded successfully with ID %u", __FILE__, m->name, m->id);
    
    // Free temporary storage
    darray_destroy(vertices);
    darray_destroy(texcoords);
    darray_destroy(normals);
    darray_destroy(faces);
    
    return m;
}

void model_destroy(model* m) {
    if (!m) return;
    
    // Unregister from the model manager
    // unregister_model(m);
    
    // Destroy mesh
    if (m->mesh) {
        renderer_destroy_mesh(m->mesh);
    }
    
    // Destroy texture
    if (m->texture) {
        texture_destroy(m->texture);
    }
    
    // Free vertex data
    if (m->vertices) {
        kfree(m->vertices, sizeof(vertex) * m->vertex_count, MEMORY_TAG_MODEL);
    }
    
    // Free index data if indexed
    if (m->is_indexed && m->indices) {
        kfree(m->indices, sizeof(u32) * m->index_count, MEMORY_TAG_MODEL);
    }
    
    // Free model struct itself
    kfree(m, sizeof(model), MEMORY_TAG_MODEL);
}

void model_render(model* m, vec3 position, vec3 rotation, vec3 scale) {
    if (!m || !m->mesh) return;
    
    // We need to add this mesh to the render packet that will be used in the current frame
    // This is typically done by the game code by creating a mesh_command and adding it to
    // the render packet's mesh_commands array. 
    //
    // Since we don't have direct access to the render packet here, the game code
    // should call this function and then add the mesh to its render command list.
    
    // For now, we just return the mesh - the caller is responsible for adding it to the render packet
    // Example usage in game code:
    // model* my_model = model_load_obj("bin/assets/models/cube.obj");
    //model_render(my_model, position, rotation, scale);
    
    // Then in game's render function:
    // mesh_command cmd = {
    // .mesh = m->mesh,
    // .position = position,
    // .rotation = rotation,
    // .scale = scale
    // };
    // renderer_draw_mesh(cmd.mesh);
    //   darray_push(state->mesh_commands, cmd);
}

model* model_get_by_id(u32 model_id) {
    // for (u32 i = 0; i < model_count; i++) {
    //     if (models[i] && models[i]->id == model_id) {
    //         return models[i];
    //     }
    // }
    return NULL;
}

// Internal model manager functions
// static void register_model(model* m) {
//     if (!m) return;
    
//     // Initialize model array if not done yet
//     if (!models) {
//         model_capacity = 16; // Start with space for 16 models
//         models = kallocate(sizeof(model*) * model_capacity, MEMORY_TAG_ARRAY);
//         model_count = 0;
//     }
    
//     // Resize model array if needed
//     if (model_count >= model_capacity) {
//         u32 new_capacity = model_capacity * 2;
//         model** new_models = kallocate(sizeof(model*) * new_capacity, MEMORY_TAG_ARRAY);
        
//         // Copy over existing pointers
//         kcopy_memory(new_models, models, sizeof(model*) * model_capacity);
        
//         // Free old array and update
//         kfree(models, sizeof(model*) * model_capacity, MEMORY_TAG_ARRAY);
//         models = new_models;
//         model_capacity = new_capacity;
//     }
    
//     // Add the model
//     models[model_count++] = m;
// }

// static void unregister_model(model* m) {
//     if (!m || !models) return;
    
//     // Find the model in the array
//     for (u32 i = 0; i < model_count; i++) {
//         if (models[i] == m) {
//             // Replace this model with the last one in the array
//             if (i < model_count - 1) {
//                 models[i] = models[model_count - 1];
//             }
            
//             // Null out the last pointer and decrement count
//             models[model_count - 1] = NULL;
//             model_count--;
//             break;
//         }
//     }
// }

static char* extract_filename(const char* path) {
    if (!path) return NULL;
    
    // Find the last path separator
    const char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        last_slash = strrchr(path, '\\');
    }
    
    const char* filename = last_slash ? last_slash + 1 : path;
    
    // Find the extension and truncate
    char* result = string_duplicate(filename);
    char* dot = strrchr(result, '.');
    if (dot) {
        *dot = '\0';
    }
    
    return result;
} 