#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef EUREKA_FLUTTER_EMBEDDER_EXPORT
#ifdef _WIN32
#define EUREKA_API __declspec(dllexport)
#else
#define EUREKA_API __attribute__((visibility("default")))
#endif
#else
#ifdef _WIN32
#define EUREKA_API __declspec(dllimport)
#else
//#define EUREKA_API __attribute__((visibility("default"))) // don't know whaty happens on linux
#endif
#endif
   

typedef struct
{
    const char* asset_path;
    const char* icudtl_path;
    const char* aot_path;
} EmbedderInitParams;

typedef enum {
    eOk,
    eFail
} FlutterEmbedderStatus;

typedef void* EMBEDDER_HANDLE;

EUREKA_API FlutterEmbedderStatus FlutterEmbedderInit(const EmbedderInitParams* params, EMBEDDER_HANDLE* out);
EUREKA_API FlutterEmbedderStatus FlutterEmbedderFinalize(EMBEDDER_HANDLE embedder);
EUREKA_API FlutterEmbedderStatus FlutterEmbedderRun(EMBEDDER_HANDLE embedder);
#if defined(__cplusplus)
}
#endif