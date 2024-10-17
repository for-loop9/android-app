#ifndef RENDERER_H
#define RENDERER_H

#include <math/ig_mat4.h>
#include <ignite.h>
#include <graphics/ig_dbuffer.h>
#include "sprite_renderer.h"
#include <stdalign.h>

typedef struct ImFont ImFont;

typedef enum {
	RENDERER_FONT_BIG = 0,
	RENDERER_FONT_BIG_BOLD,
	RENDERER_FONT_MED,
	RENDERER_FONT_MED_BOLD,
	RENDERER_FONT_SMALL,
	RENDERER_FONT_SMALL_BOLD,
	RENDERER_FONT_SMALL_MONO,
	RENDERER_FONT_SMALL_MONO_BOLD,
} renderer_font;

#define MAX_LIGHTS 10

typedef struct light {
	alignas(8) ig_vec2 pos;
	alignas(16) ig_vec3 col;
	alignas(4) float intensity;
} light;

typedef struct renderer {
	struct {
		alignas(16) ig_mat4 projection;
		alignas(8) ig_vec2 cam_pos;

		light lights[MAX_LIGHTS];
		alignas(4) int num_lights;
	} global;

	ig_context* context;
	ig_dbuffer* global_buffer;
	ImFont* fonts[8];

	sprite_renderer* sprite_renderer;
} renderer;

typedef struct game game;

renderer* renderer_create(ig_context* context, ig_window* window, ig_texture* sprite_sheet);
void renderer_start_imgui_frame(renderer* renderer);
void renderer_push_sprite(renderer* renderer, const sprite_instance* sprite_instance);
void renderer_push_light(renderer* renderer, const light* light);
void renderer_flush(renderer* renderer, const ig_vec4* clear_color);
void renderer_destroy(renderer* renderer);

#endif