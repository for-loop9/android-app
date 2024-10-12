
cmake_minimum_required(VERSION 3.22.1)
project(main_app_project)

set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC
	${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable -Wno-nullability-completeness")

set(CMAKE_SHARED_LINKER_FLAGS
	"${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

file(GLOB_RECURSE SOURCES
    "app/src/main/**/*.c"
    "app/src/main/**/*.cpp"
)

add_library(main_app SHARED ${SOURCES})

target_compile_definitions(main_app
	PRIVATE IG_ANDROID)

target_include_directories(main_app PRIVATE
	${ANDROID_NDK}/sources/android/native_app_glue
	app/src/main/ignite/src)

target_link_libraries(
	main_app
	android
	native_app_glue
	vulkan
	log)