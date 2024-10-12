#ifndef RENDERER_H
#define RENDERER_H

#include <math/ig_mat4.h>
#include <ignite.h>
#include <graphics/ig_dbuffer.h>
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

typedef struct renderer {
	ig_context* context;
	ImFont* fonts[8];
} renderer;

typedef struct game game;

renderer* renderer_create(ig_context* context, ig_window* window);
void renderer_start_imgui_frame(renderer* renderer);
void renderer_flush(renderer* renderer, const ig_vec4* clear_color);
void renderer_destroy(renderer* renderer);

#endif