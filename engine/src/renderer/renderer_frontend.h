#pragma once
 
#include "renderer_types.inl"
#include "core/kmemory.h"
#include "core/logger.h"
#include "platform/platform.h"
 
struct static_mesh_data;
struct platform_state;
 
b8 renderer_initialize(const char* application_name, struct platform_state* plat_state);
void renderer_shutdown();
 
void renderer_on_resized(u16 width, u16 height);
 
b8 renderer_begin_frame(f32 delta_time);
b8 renderer_end_frame(f32 delta_time);
b8 renderer_draw_frame(render_packet* packet);

// Mesh functions
mesh* renderer_create_mesh(const vertex* vertices, u32 vertex_count);
void renderer_destroy_mesh(mesh* m);
void renderer_draw_mesh(mesh* m);

// Text functions
font* renderer_create_font(const char* font_path, u32 font_size);
void renderer_destroy_font(font* f);
void renderer_draw_text(font* f, const char* text, vec2 position, vec4 color, f32 scale);
