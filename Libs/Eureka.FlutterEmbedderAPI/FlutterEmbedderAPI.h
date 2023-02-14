

#if defined(__cplusplus)
extern "C" {
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

FlutterEmbedderStatus FlutterEmbedderInit(const EmbedderInitParams* params, EMBEDDER_HANDLE* out);
FlutterEmbedderStatus FlutterEmbedderFinalize(EMBEDDER_HANDLE embedder);
FlutterEmbedderStatus FlutterEmbedderRun(EMBEDDER_HANDLE embedder);
#if defined(__cplusplus)
}
#endif