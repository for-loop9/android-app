#include "ig_window.h"
#include <stdlib.h>
#include <stdio.h>
#include <jni.h>
#include "../util/ig_log.h"

#include <vulkan/vulkan.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "../../../app/src/external/cimgui/cimgui.h"

#ifndef IG_ANDROID
void framebuffer_resize_callback(GLFWwindow* window, int width, int height) {
	user_data* ud = glfwGetWindowUserPointer(window);
	ud->window->dim.x = width;
	ud->window->dim.y = height;
	ud->window->resize_requested = true;
}

ig_window* ig_window_create_asp(float asp_ratio, const char* title, const GLFWimage* icons, int icons_count) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	ig_window* r = (ig_window*) malloc(sizeof(ig_window));
	r->resize_requested = false;
	r->ud.window = r;
	const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    int window_width = vidmode->width * 0.75f;
    int window_height = window_width / asp_ratio;

    if (window_height > vidmode->height * 0.75f) {
        window_height = vidmode->height * 0.75f;
        window_width = window_height * asp_ratio;
    }

	r->dim.x = window_width;
	r->dim.y = window_height;
	r->last_dim.x = window_width;
	r->last_dim.y = window_height;

	r->native_handle = glfwCreateWindow(r->dim.x, r->dim.y, title, NULL, NULL);
	if (icons) glfwSetWindowIcon(r->native_handle, icons_count, icons);
	glfwSetWindowPos(r->native_handle, (vidmode->width / 2) - (r->dim.x / 2), (vidmode->height / 2) - (r->dim.y / 2));
	glfwSetFramebufferSizeCallback(r->native_handle, framebuffer_resize_callback);
	glfwSetWindowUserPointer(r->native_handle, &r->ud);

	return r;
}

ig_window* ig_window_create(const ig_ivec2* dim, const char* title, int full_screen, int monitor, const GLFWimage* icons, int icons_count) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	ig_window* r = (ig_window*) malloc(sizeof(ig_window));
	r->ud.window = r;
	r->resize_requested = false;
	int monitor_count;
	GLFWmonitor** monitors = glfwGetMonitors(&monitor_count);
	const GLFWvidmode* vidmode = glfwGetVideoMode(monitors[monitor]);
	r->dim = full_screen ? (ig_ivec2) { .x = vidmode->width, .y = vidmode->height } : *dim;
	r->last_dim.x = r->dim.x;
	r->last_dim.y = r->dim.y;

	r->native_handle = glfwCreateWindow(r->dim.x, r->dim.y, title, full_screen ? monitors[monitor] : NULL, NULL);
	if (icons) glfwSetWindowIcon(r->native_handle, icons_count, icons);
	glfwSetFramebufferSizeCallback(r->native_handle, framebuffer_resize_callback);
	if (!full_screen)
		glfwSetWindowPos(r->native_handle, (vidmode->width / 2) - (r->dim.x / 2), (vidmode->height / 2) - (r->dim.y / 2));
	glfwSetWindowUserPointer(r->native_handle, &r->ud);

	return r;
}
#endif

#ifdef IG_ANDROID
void handle_app_cmd(struct android_app* app, int32_t app_cmd) {
	ig_window* window = (ig_window*) app->userData;
	switch (app_cmd) {
	case APP_CMD_WINDOW_RESIZED:
		window->resize_requested = true;
		window->dim.x = ANativeWindow_getWidth(app->window);
		window->dim.y = ANativeWindow_getHeight(app->window);
		break;
	case APP_CMD_SAVE_STATE:
		break;
	case APP_CMD_INIT_WINDOW:
		IG_LOG("init, sdk verison %d", app->activity->sdkVersion);

		window->dim.x = ANativeWindow_getWidth(app->window);
		window->dim.y = ANativeWindow_getHeight(app->window);

		jobject acc_obj = app->activity->clazz;
		JNIEnv* env = app->activity->env;
		JavaVM* jvm = app->activity->vm;
		jint result = (*jvm)->AttachCurrentThread(jvm, &env, NULL);
		jclass clazz = (*env)->GetObjectClass(env, acc_obj);
		jmethodID get_window_method = (*env)->GetMethodID(env, clazz, "getWindow", "()Landroid/view/Window;");
		jobject window_obj = (*env)->CallObjectMethod(env, acc_obj, get_window_method);
		jclass window_class = (*env)->GetObjectClass(env, window_obj);
		jmethodID get_decore_view_method = (*env)->GetMethodID(env, window_class, "getDecorView", "()Landroid/view/View;");
		jobject decor_obj = (*env)->CallObjectMethod(env, window_obj, get_decore_view_method);
		jclass view_class = (*env)->GetObjectClass(env, decor_obj);
		jmethodID get_root_window_insets_method = (*env)->GetMethodID(env, view_class, "getRootWindowInsets", "()Landroid/view/WindowInsets;");
		jobject insets_obj = (*env)->CallObjectMethod(env, decor_obj, get_root_window_insets_method);
		jclass insets_class = (*env)->GetObjectClass(env, insets_obj);
		jmethodID get_cutout_method = (*env)->GetMethodID(env, insets_class, "getDisplayCutout", "()Landroid/view/DisplayCutout;");
		jobject cutout_obj = (*env)->CallObjectMethod(env, insets_obj, get_cutout_method);

		IG_LOG(cutout_obj ? "cutout found" : "no cutout");

		if (cutout_obj) {
			jclass cutout_class = (*env)->GetObjectClass(env, cutout_obj);
			jmethodID get_rect_top_method = (*env)->GetMethodID(env, cutout_class, "getBoundingRectTop", "()Landroid/graphics/Rect;");
			jobject rect_obj = (*env)->CallObjectMethod(env, cutout_obj, get_rect_top_method);
			jclass rect_class = (*env)->GetObjectClass(env, rect_obj);
			jmethodID get_ctr_x = (*env)->GetMethodID(env, rect_class, "centerX", "()I");
			jint center_x = (*env)->CallIntMethod(env, rect_obj, get_ctr_x);

			// IG_LOG("ctr x = %d", center_x);

			(*env)->DeleteLocalRef(env, rect_class);
			(*env)->DeleteLocalRef(env, rect_obj);
			(*env)->DeleteLocalRef(env, cutout_class);
		}

		(*env)->DeleteLocalRef(env, cutout_obj);
		(*env)->DeleteLocalRef(env, insets_class);
		(*env)->DeleteLocalRef(env, insets_obj);
		(*env)->DeleteLocalRef(env, view_class);
		(*env)->DeleteLocalRef(env, decor_obj);
		(*env)->DeleteLocalRef(env, window_class);
		(*env)->DeleteLocalRef(env, window_obj);
		(*env)->DeleteLocalRef(env, clazz);

		(*jvm)->DetachCurrentThread(jvm);

		window->started = true;
		break;
	case APP_CMD_TERM_WINDOW:
		IG_LOG("destroy");
		((ig_window*) app->userData)->running = false;
		break;
	}
}

int32_t handle_input_event(struct android_app* app, AInputEvent* input_event) {
    return igImplAndroid_HandleInputEvent(input_event);
}

ig_window* ig_window_create(struct android_app* app) {
	ig_window* r = (ig_window*) malloc(sizeof(ig_window));
	r->running = true;
	r->started = false;
	r->resize_requested = false;
	r->native_handle = app;

	app->onAppCmd = handle_app_cmd;
	app->userData = r;
	app->onInputEvent = handle_input_event;

	while (!r->started) {
		ig_window_input(r);
	}

	return r;
}
#endif

void ig_window_input(ig_window* window) {
	#ifndef IG_ANDROID
	glfwPollEvents();
	#else

	int events;
	struct android_poll_source* source;
	if (ALooper_pollOnce(0, NULL, &events, (void**) &source) >= 0) {
		if (source != NULL) {
			source->process(window->native_handle, source);
		}
	}
	#endif
}

int ig_window_closed(ig_window* window) {
	#ifndef IG_ANDROID
	return glfwWindowShouldClose(window->native_handle);
	#else
	return !window->running;
	#endif
}

#ifndef IG_ANDROID
void ig_window_set_fullscreen(ig_window* window, bool fullscreen) {
	const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (fullscreen) {
		window->last_dim.x = window->dim.x;
		window->last_dim.y = window->dim.y;
		glfwSetWindowMonitor(window->native_handle, glfwGetPrimaryMonitor(), 0, 0, vidmode->width, vidmode->height, GLFW_DONT_CARE);
	} else {
		glfwSetWindowMonitor(window->native_handle, NULL, (vidmode->width / 2) - (window->last_dim.x / 2), (vidmode->height / 2) - (window->last_dim.y / 2), window->last_dim.x, window->last_dim.y, GLFW_DONT_CARE);
	}
}

int ig_window_keyboard_key_down(ig_window* window, int key) {
	return glfwGetKey(window->native_handle, key) == GLFW_PRESS;
}

int ig_window_mouse_button_down(ig_window* window, int button) {
	return glfwGetMouseButton(window->native_handle, button);
}
#else
void ig_window_show_keyboard(ig_window* window) {
	jobject acc_obj = window->native_handle->activity->clazz;
	JNIEnv* env = window->native_handle->activity->env;
	JavaVM* jvm = window->native_handle->activity->vm;
	jint result = (*jvm)->AttachCurrentThread(jvm, &env, NULL);

	jclass activity_class = (*env)->GetObjectClass(env, acc_obj);
	jmethodID get_system_service_id = (*env)->GetMethodID(env, activity_class, "getSystemService", "(Ljava/lang/Class;)Ljava/lang/Object;");
	jclass imm_class = (*env)->FindClass(env, "android/view/inputmethod/InputMethodManager");
	jobject imm_obj = (*env)->CallObjectMethod(env, acc_obj, get_system_service_id, imm_class);
	jmethodID show_keyboard_id = (*env)->GetMethodID(env, imm_class, "showSoftInput", "(Landroid/view/View;I)Z");

	jmethodID get_window_method = (*env)->GetMethodID(env, activity_class, "getWindow", "()Landroid/view/Window;");
	jobject window_obj = (*env)->CallObjectMethod(env, acc_obj, get_window_method);
	jclass window_class = (*env)->GetObjectClass(env, window_obj);
	jmethodID get_decore_view_method = (*env)->GetMethodID(env, window_class, "getDecorView", "()Landroid/view/View;");
	jobject decor_obj = (*env)->CallObjectMethod(env, window_obj, get_decore_view_method);
	jint flags = (*env)->GetStaticIntField(env, imm_class, (*env)->GetStaticFieldID(env, imm_class, "SHOW_IMPLICIT", "I"));
	(*env)->CallBooleanMethod(env, imm_obj, show_keyboard_id, decor_obj, flags);

	(*env)->DeleteLocalRef(env, decor_obj);
	(*env)->DeleteLocalRef(env, window_class);
	(*env)->DeleteLocalRef(env, window_obj);
	(*env)->DeleteLocalRef(env, imm_obj);
	(*env)->DeleteLocalRef(env, imm_class);
	(*env)->DeleteLocalRef(env, activity_class);
	(*jvm)->DetachCurrentThread(jvm);
}

void ig_window_hide_keyboard(ig_window* window) {
	jobject acc_obj = window->native_handle->activity->clazz;
	JNIEnv* env = window->native_handle->activity->env;
	JavaVM* jvm = window->native_handle->activity->vm;
	jint result = (*jvm)->AttachCurrentThread(jvm, &env, NULL);

	jclass activity_class = (*env)->GetObjectClass(env, acc_obj);
	jmethodID get_system_service_id = (*env)->GetMethodID(env, activity_class, "getSystemService", "(Ljava/lang/Class;)Ljava/lang/Object;");
	jclass imm_class = (*env)->FindClass(env, "android/view/inputmethod/InputMethodManager");
	jobject imm_obj = (*env)->CallObjectMethod(env, acc_obj, get_system_service_id, imm_class);
	jmethodID hide_keyboard_id = (*env)->GetMethodID(env, imm_class, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");

	jmethodID get_window_method = (*env)->GetMethodID(env, activity_class, "getWindow", "()Landroid/view/Window;");
	jobject window_obj = (*env)->CallObjectMethod(env, acc_obj, get_window_method);
	jclass window_class = (*env)->GetObjectClass(env, window_obj);
	jmethodID get_decore_view_method = (*env)->GetMethodID(env, window_class, "getDecorView", "()Landroid/view/View;");
	jobject decor_obj = (*env)->CallObjectMethod(env, window_obj, get_decore_view_method);
	jclass view_class = (*env)->GetObjectClass(env, decor_obj);
	jmethodID get_token_method = (*env)->GetMethodID(env, view_class, "getWindowToken", "()Landroid/os/IBinder;");
	jobject token_obj = (*env)->CallObjectMethod(env, decor_obj, get_token_method);
	jint flags = (*env)->GetStaticIntField(env, imm_class, (*env)->GetStaticFieldID(env, imm_class, "HIDE_IMPLICIT_ONLY", "I"));
	(*env)->CallBooleanMethod(env, imm_obj, hide_keyboard_id, token_obj, flags);

	(*env)->DeleteLocalRef(env, token_obj);
	(*env)->DeleteLocalRef(env, view_class);
	(*env)->DeleteLocalRef(env, decor_obj);
	(*env)->DeleteLocalRef(env, window_class);
	(*env)->DeleteLocalRef(env, window_obj);
	(*env)->DeleteLocalRef(env, imm_obj);
	(*env)->DeleteLocalRef(env, imm_class);
	(*env)->DeleteLocalRef(env, activity_class);
	(*jvm)->DetachCurrentThread(jvm);
}
#endif

void ig_window_destroy(ig_window* window) {
	#ifndef IG_ANDROID
	glfwDestroyWindow(window->native_handle);
	glfwTerminate();
	#else
	ANativeActivity_finish(window->native_handle->activity);
	while (!window->native_handle->destroyRequested) {
		ig_window_input(window);
	}
	#endif
	free(window);
}
