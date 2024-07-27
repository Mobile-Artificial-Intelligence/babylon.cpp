#ifndef BABYLON_H
#define BABYLON_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
   #define EXPORT __declspec(dllexport)
#else
   #define EXPORT __attribute__((visibility("default"))) __attribute__((used))
#endif

EXPORT int babylon_g2p_init(const char* model_path, const char* language, int use_punctuation);

EXPORT char* babylon_g2p(const char* text);

EXPORT void babylon_g2p_free();

EXPORT int babylon_tts_init(const char* model_path);

EXPORT void babylon_tts(const char* text, const char* output_path);

EXPORT void babylon_tts_free();

#ifdef __cplusplus
}
#endif

#endif // BABYLON_H