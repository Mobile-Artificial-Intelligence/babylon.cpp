#ifndef BABYLON_H
#define BABYLON_H

#ifdef __cplusplus
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <onnxruntime_cxx_api.h>

extern "C" {
#endif

#ifdef WIN32
   #define BABYLON_EXPORT __declspec(dllexport)
#else
   #define BABYLON_EXPORT __attribute__((visibility("default"))) __attribute__((used))
#endif

typedef struct {
   const char* language;
   const unsigned char use_dictionaries;
   const unsigned char use_punctuation;
} babylon_g2p_options_t;

BABYLON_EXPORT int babylon_g2p_init(const char* model_path, babylon_g2p_options_t options);

BABYLON_EXPORT char* babylon_g2p(const char* text);

BABYLON_EXPORT int* babylon_g2p_tokens(const char* text);

BABYLON_EXPORT void babylon_g2p_free(void);

BABYLON_EXPORT int babylon_tts_init(const char* model_path);

BABYLON_EXPORT void babylon_tts(const char* text, const char* output_path);

BABYLON_EXPORT void babylon_tts_free(void);

#ifdef __cplusplus
}

namespace DeepPhonemizer {
  class SequenceTokenizer {
    public:
      SequenceTokenizer(const std::vector<std::string>& symbols, const std::vector<std::string>& languages, int char_repeats, bool lowercase = true, bool append_start_end = true);
      std::vector<int64_t> operator()(const std::string& sentence, const std::string& language) const;
      std::vector<std::string> decode(const std::vector<int64_t>& sequence) const;
      std::vector<int64_t> clean(const std::vector<int64_t>& sequence) const;
      int64_t get_token(const std::string& token) const;
  
    private:
      std::vector<std::string> tokens;
      int char_repeats;
      bool lowercase;
      bool append_start_end;
      int pad_index;
      int end_index;
      std::string pad_token;
      std::string end_token;
      std::unordered_set<std::string> special_tokens;
  };

  class Session {
    public:
      Session(const std::string& model_path, const std::string language = "en_us", const bool use_dictionaries = true, const bool use_punctuation = false);
      ~Session();

      std::vector<std::string> g2p(const std::string& text);
      std::vector<int64_t> g2p_tokens(const std::string& text);

    private:
      std::string language;
      bool use_dictionaries;
      bool use_punctuation;
      Ort::Session* session;
      SequenceTokenizer* text_tokenizer;
      SequenceTokenizer* phoneme_tokenizer;
      std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> dictionaries;

      std::vector<int64_t> g2p_tokens_internal(const std::string& text);
  };

  std::vector<std::string> clean_text(const std::string& text);
}

namespace Vits {
  class SequenceTokenizer {
    public:
      SequenceTokenizer(const std::vector<std::string>& phonemes, const std::vector<int>& phoneme_ids);
      std::vector<int64_t> operator()(const std::vector<std::string>& phonemes) const;

    private:
      std::unordered_map<std::string, int> token_to_idx;
  };

  class Session {
    public:
      Session(const std::string& model_path);
      ~Session();

      void tts(const std::vector<std::string>& phonemes, const std::string& output_path);

    private:
      int sample_rate;
      std::vector<float> scales;

      Ort::Session* session;
      SequenceTokenizer* phoneme_tokenizer;
  };
}
#endif

#endif // BABYLON_H