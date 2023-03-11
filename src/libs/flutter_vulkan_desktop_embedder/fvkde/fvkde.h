#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef EUREKA_FLUTTER_EMBEDDER_EXPORT
#ifdef _WIN32
#define FVDKE_API __declspec(dllexport)
#else
#define FVDKE_API __attribute__((visibility("default")))
#endif
#else
#ifdef _WIN32
#define FVDKE_API __declspec(dllimport)
#else
//#define EUREKA_API __attribute__((visibility("default"))) // don't know whaty happens on linux
#endif
#endif
   

typedef struct
{
    const char* asset_path;
    const char* icudtl_path;
    const char* aot_path;
} FVKDE_INIT_PARAMS;

typedef enum {
    eOk,
    eFail
} FVKDE_STATUS;

typedef void* FVKDE_HANDLE;


FVDKE_API FVKDE_STATUS fvkde_init(const FVKDE_INIT_PARAMS* params, FVKDE_HANDLE* out);
FVDKE_API FVKDE_STATUS fvkde_finalize(FVKDE_HANDLE embedder);
FVDKE_API FVKDE_STATUS fvkde_run(FVKDE_HANDLE embedder);


#if defined(__cplusplus)
}
#endif