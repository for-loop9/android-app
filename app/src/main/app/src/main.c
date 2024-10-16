#include "rendering/renderer.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "external/cimgui/cimgui.h"
#include "external/mongoose/mongoose.h"

static float now_ms(void) {
	struct timespec res;
	clock_gettime(CLOCK_MONOTONIC, &res);
	return 1000.0 * res.tv_sec + (float) res.tv_nsec / 1e6;
}

void ev_handle(struct mg_connection* c, int ev, void* ev_data) {
	switch (ev) {
	case MG_EV_OPEN:
		*(uint64_t*) c->data = mg_millis() + 5000;
		break;
	case MG_EV_POLL:
		if (mg_millis() > *(uint64_t*) c->data && (c->is_connecting || c->is_resolving)) {
			mg_error(c, "connection timed out");
		}
		break;
	case MG_EV_WRITE:
		*(bool*) c->fn_data = false;
	break;
	case MG_EV_CONNECT:
		mg_send(c, "HELLO\n", 6);
		IG_LOG("connection successful, disconnecting.");
		break;
	case MG_EV_CLOSE:
		IG_LOG("connection closed.");
		break;
	case MG_EV_ERROR:
		IG_LOG("error: %s", (char*) ev_data);
		*(bool*) c->fn_data = false;
	break;
	}
}

void android_main(struct android_app* state) {
	ig_window* window = ig_window_create(state);
	ig_context* ctx = ig_context_create(window, 1, 1);
	ig_texture* sprite_sheet = ig_context_texture_create_from_file(ctx, "app/res/textures/sprite_sheet.png");
	renderer* renderer = renderer_create(ctx, window, sprite_sheet);
	ImTextureID imgui_tex = igImplVulkan_AddTexture(ctx->nearest_sampler, sprite_sheet->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	struct mg_mgr mgr;
	bool running = true;
	mg_mgr_init(&mgr);
	struct mg_connection* c = mg_connect(&mgr, "tcp://192.168.2.35:1234", ev_handle, &running);

	while (running) {
		mg_mgr_poll(&mgr, 1000);
	}

	mg_mgr_free(&mgr);

	int fps = 0;
	int fps_display = 0;
	float ms_start = now_ms();

	while (!ig_window_closed(window)) {
		fps++;
		ig_window_input(window);
		renderer_start_imgui_frame(renderer);

		// show fps
		ImDrawList* draw_list = igGetForegroundDrawList_ViewportPtr(NULL);
		igPushFont(renderer->fonts[RENDERER_FONT_BIG]);
		char fps_str[30] = {};
		sprintf(fps_str, "FPS = %d", fps_display);
		ImDrawList_AddText_Vec2(draw_list, (ImVec2) { 200, 200 }, igColorConvertFloat4ToU32((ImVec4) { 1, 0.6f, 0.4f, 1 }), fps_str, NULL);
		igPopFont();

		igShowDemoWindow(NULL);
		igBegin("tex", NULL, ImGuiWindowFlags_None);
		igImage(imgui_tex, (ImVec2) { 128 * 4, 128 * 4 }, (ImVec2) { 0, 0 }, (ImVec2) { 1, 1 }, (ImVec4) { 1, 1, 1, 1 }, (ImVec4) { 0, 0, 0, 1 });
		if (igButton("show keyboard", (ImVec2) { -1, 0 })) {
			ig_window_show_keyboard(window);
		}
		if (igButton("hide keyboard", (ImVec2) { -1, 0 })) {
			ig_window_hide_keyboard(window);
		}
		igEnd();

		if (ig_context_begin(ctx, window, 1, 1)) {
			renderer_flush(renderer, &(ig_vec4) { 0.0f, 0.0f, 0.0f, 1.0f });
			ig_context_end(ctx, window, 1, 1);
		}

		if ((now_ms() - ms_start) >= 1000) {
			// IG_LOG("fps: %d\n", fps);
			fps_display = fps;
			fps = 0;
			ms_start = now_ms();
		}
	}

	ig_context_finish(ctx);

	ig_context_texture_destroy(ctx, sprite_sheet);
	renderer_destroy(renderer);
	ig_context_destroy(ctx);
	ig_window_destroy(window);

	IG_LOG("main end");
}
