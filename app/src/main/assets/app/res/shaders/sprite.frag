#version 460 core

layout (location = 0) out vec4 color;

layout (set = 1, binding = 0) uniform sampler2D sprite_sheet;

layout (location = 0) in vec4 out_color;
layout (location = 1) in vec2 out_uv;
layout (location = 2) in vec2 out_cam_pos;
layout (location = 3) in vec2 out_pos;

struct light {
	vec2 pos;
	vec3 col;
	float intensity;
};

layout (set = 0, binding = 1) uniform Global {
	mat4 projection;
	vec2 cam_pos;

	light lights[10];
	int num_lights;
} global;

void main() {
	vec4 sprite_sheet_col = texture(sprite_sheet, out_uv);
	color = out_color * sprite_sheet_col;

	vec3 light_factor = vec3(0, 0, 0);
	for (int i = 0; i < global.num_lights; i++) {
		light_factor += ((1 / (length(out_pos - global.lights[i].pos) / (1 + global.lights[i].intensity))) * global.lights[i].col);// + ((1 / (length(out_pos - vec2(out_cam_pos.x + 96 + 32, out_cam_pos.y)) / 15)) * vec3(0, 0.5, 1));
	}
	color.rgb *= light_factor;
}