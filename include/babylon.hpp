#ifndef BABYLON_HPP
#define BABYLON_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <onnxruntime_cxx_api.h>

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
      Session(const std::string& model_path, const std::string language = "en_us", const bool use_punctuation = false);
      ~Session();

      std::vector<std::string> g2p(const std::string& text);
      std::vector<int64_t> g2p_tokens(const std::string& text);

    private:
      std::string lang;
      bool punctuation;
      Ort::Session* session;
      SequenceTokenizer* text_tokenizer;
      SequenceTokenizer* phoneme_tokenizer;

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

#endif // BABYLON_HPP