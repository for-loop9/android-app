#include "renderer.h"
#include <graphics/ig_buffer.h>
#include <stdlib.h>
#include <string.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../external/cimgui/cimgui.h"

void dark_theme(ImGuiStyle* style) {
	style->FrameRounding = 4;
	style->WindowRounding = 4;
	style->FrameBorderSize = 1;
	style->GrabRounding = 2;
	style->ScrollbarRounding = 0;

	style->Colors[ImGuiCol_Button] = (ImVec4) { 0.345f, 0.345f, 0.345f, 1 };
	style->Colors[ImGuiCol_ButtonHovered] = (ImVec4) { 0.345f * 1.2f, 0.345f * 1.2f, 0.345f * 1.2f, 1 };
	style->Colors[ImGuiCol_ButtonActive] = (ImVec4) { 0.345f * 0.8f, 0.345f * 0.8f, 0.345f * 0.8f, 1 };
	style->Colors[ImGuiCol_SliderGrab] = (ImVec4) { 0.392f, 0.392f, 0.392f, 1 };
	style->Colors[ImGuiCol_SliderGrabActive] = (ImVec4) { 0.392f * 1.3f, 0.392f * 1.3f, 0.392f * 1.3f, 1 };
	style->Colors[ImGuiCol_WindowBg] = (ImVec4) { 0.22f, 0.22f, 0.22f, 1 };
	style->Colors[ImGuiCol_Border] = (ImVec4) { 0.047f, 0.047f, 0.047f, 1 };
	style->Colors[ImGuiCol_TabDimmedSelected] = (ImVec4) { 0.239f, 0.239f, 0.239f, 1 };
	style->Colors[ImGuiCol_TabSelected] = (ImVec4) { 0.239f, 0.239f, 0.239f, 1 };
	style->Colors[ImGuiCol_TabHovered] = (ImVec4) { 0.239f * 1.4f, 0.239f * 1.4f, 0.239f * 1.4f, 1 };
	style->Colors[ImGuiCol_TabDimmedSelectedOverline] = (ImVec4) {};
	style->Colors[ImGuiCol_TitleBg] = style->Colors[ImGuiCol_TabDimmed] = style->Colors[ImGuiCol_Tab] = (ImVec4) { 0.157f, 0.157f, 0.157f, 1 };
	style->Colors[ImGuiCol_TitleBgActive] = (ImVec4) { 0.157f, 0.157f, 0.157f, 1 };
	style->Colors[ImGuiCol_Header] = (ImVec4) { 0.22f, 0.22f, 0.22f, 1 };
	style->Colors[ImGuiCol_HeaderHovered] = (ImVec4) { 0.271f, 0.271f, 0.271f, 1 };
	style->Colors[ImGuiCol_HeaderActive] = (ImVec4) { 0.149f, 0.353f, 0.541f, 1 };

	style->Colors[ImGuiCol_FrameBg] = (ImVec4) { 0.165f, 0.165f, 0.165f, 1 };
	style->Colors[ImGuiCol_FrameBgHovered] = (ImVec4) { 0.22f, 0.22f, 0.22f, 1 };
	style->Colors[ImGuiCol_FrameBgActive] = (ImVec4) { 0.165f * 0.8f, 0.165f * 0.8f, 0.165f * 0.8f, 1 };
	style->Colors[ImGuiCol_CheckMark] = (ImVec4) { 0.845f, 0.845f, 0.845f, 1 };
}

void imgui_init(ig_context* context, ig_window* window, renderer* renderer) {
	ImGui_ImplVulkan_InitInfo vk_imgui_init_info = {
		.Instance = context->instance,
		.PhysicalDevice = context->gpu,
		.Device = context->device,
		.QueueFamily = context->queue_family,
		.Queue = context->queue,
		.DescriptorPool = context->descriptor_pool,
		.RenderPass = context->default_frame.render_pass,
		.MinImageCount = context->fif,
		.ImageCount = context->fif,
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.Subpass = 0
	};
	
	igCreateContext(NULL);
	ImGuiIO* io = igGetIO();
	io->IniFilename = NULL;
	io->LogFilename = NULL;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// igStyleColorsLight(NULL);
	dark_theme(igGetStyle());
	igImplAndroid_Init(window->native_handle->window);
	igImplVulkan_Init(&vk_imgui_init_info);

	#define ADD_FONT(cd, path, sz)\
	{\
	void* out_data;\
	size_t out_sz;\
	ig_read_file(context, path, &out_data, &out_sz);\
	renderer->fonts[cd] = ImFontAtlas_AddFontFromMemoryTTF(io->Fonts, out_data, out_sz, sz, NULL, NULL);\
	}

	ADD_FONT(RENDERER_FONT_SMALL, "app/res/fonts/calibri-regular.ttf", 32);
	ADD_FONT(RENDERER_FONT_SMALL_BOLD, "app/res/fonts/calibri-bold.ttf", 32);
	ADD_FONT(RENDERER_FONT_SMALL_MONO, "app/res/fonts/jetbrains-regular.ttf", 32);
	ADD_FONT(RENDERER_FONT_SMALL_MONO_BOLD, "app/res/fonts/jetbrains-bold.ttf", 32);
	ADD_FONT(RENDERER_FONT_MED, "app/res/fonts/calibri-regular.ttf", 40);
	ADD_FONT(RENDERER_FONT_MED_BOLD, "app/res/fonts/calibri-bold.ttf", 40);
	ADD_FONT(RENDERER_FONT_BIG, "app/res/fonts/calibri-regular.ttf", 60);
	ADD_FONT(RENDERER_FONT_BIG_BOLD, "app/res/fonts/calibri-bold.ttf", 60);

	igImplVulkan_CreateFontsTexture();
}

void imgui_render(_ig_frame* frame) {
	igRender();
	ImDrawData* draw_data = igGetDrawData();
	igImplVulkan_RenderDrawData(draw_data, frame->cmd_buffer, VK_NULL_HANDLE);
}

void imgui_destroy() {
	igImplVulkan_Shutdown();
    igImplAndroid_Shutdown();
    igDestroyContext(NULL);
}

renderer* renderer_create(ig_context* context, ig_window* window, ig_texture* sprite_sheet) {
	renderer* r = malloc(sizeof(renderer));
	r->context = context;

	r->global.cam_pos = (ig_vec2) { .x = 0, .y = 0 };
	r->global.num_lights = 0;
	memset(r->global.lights, 0, sizeof(r->global.lights));
	r->global_buffer = ig_context_dbuffer_create(context, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, NULL, sizeof(r->global));

	for (int i = 0; i < context->fif; i++) {
		vkUpdateDescriptorSets(context->device, 1, &(VkWriteDescriptorSet) {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = NULL,
			.dstSet = context->global_set[i],
			.dstBinding = 1,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = NULL,
			.pBufferInfo = &(VkDescriptorBufferInfo) {
				.buffer = r->global_buffer[i].buffer,
				.offset = 0,
				.range = VK_WHOLE_SIZE
			},
			.pTexelBufferView = NULL
		}, 0, NULL);
	}

	imgui_init(context, window, r);
	r->sprite_renderer = sprite_renderer_create(context, sprite_sheet, 2048);

	return r;
}

void renderer_start_imgui_frame(renderer* renderer) {
	igImplVulkan_NewFrame();
	igImplAndroid_NewFrame();
	igNewFrame();
}

void renderer_push_sprite(renderer* renderer, const sprite_instance* sprite_instance) {
	sprite_renderer_push(renderer->sprite_renderer, sprite_instance);
}

void renderer_push_light(renderer* renderer, const light* light) {
	renderer->global.lights[renderer->global.num_lights++] = *light;
}

void renderer_flush(renderer* renderer, const ig_vec4* clear_color) {
	_ig_frame* frame = renderer->context->frames + renderer->context->frame_idx;
	ig_mat4_ortho(&renderer->global.projection, 0, renderer->context->default_frame.resolution.x, renderer->context->default_frame.resolution.y, 0, 1, -1);
	memcpy(renderer->global_buffer[renderer->context->frame_idx].data, &renderer->global, sizeof(renderer->global));

	vkCmdBeginRenderPass(frame->cmd_buffer, &(VkRenderPassBeginInfo) {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.pNext = NULL,
		.renderPass = renderer->context->default_frame.render_pass,
		.framebuffer = renderer->context->default_frame.framebuffer,
		.renderArea = {
			.offset = { .x = 0, .y = 0 },
			.extent = {
				.width = renderer->context->default_frame.resolution.x,
				.height = renderer->context->default_frame.resolution.y
			}
		},
		.clearValueCount = 2,
		.pClearValues = (VkClearValue[]) {
			{ .color = { .float32 = { clear_color->x, clear_color->y, clear_color->z, clear_color->w } } },
			{ .depthStencil = { .depth = 0, .stencil = 0 }}
		}
	}, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindVertexBuffers(frame->cmd_buffer, 0, 1, &renderer->context->quad_buffer->buffer, (VkDeviceSize[]) { 0 });
	sprite_renderer_flush(renderer->sprite_renderer, renderer->context, frame);
	imgui_render(frame);
	vkCmdEndRenderPass(frame->cmd_buffer);

	renderer->global.num_lights = 0;
}

void renderer_destroy(renderer* renderer) {
	sprite_renderer_destroy(renderer->sprite_renderer, renderer->context);
	imgui_destroy();
	ig_context_dbuffer_destroy(renderer->context, renderer->global_buffer);
	free(renderer);
}
