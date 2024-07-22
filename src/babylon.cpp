#include "babylon.hpp"
#include <onnxruntime_cxx_api.h>
#include <iostream>
#include <algorithm>

LanguageTokenizer::LanguageTokenizer(const std::vector<std::string>& languages) {
    for (size_t i = 0; i < languages.size(); ++i) {
        lang_index[languages[i]] = i;
        index_lang[i] = languages[i];
    }
}

int LanguageTokenizer::operator()(const std::string& lang) const {
    auto it = lang_index.find(lang);
    if (it == lang_index.end()) {
        throw std::invalid_argument("Language not supported: " + lang);
    }
    return it->second;
}

std::string LanguageTokenizer::decode(int index) const {
    auto it = index_lang.find(index);
    if (it == index_lang.end()) {
        throw std::invalid_argument("Index not supported: " + std::to_string(index));
    }
    return it->second;
}

SequenceTokenizer::SequenceTokenizer(const std::vector<std::string>& symbols, const std::vector<std::string>& languages, int char_repeats, bool lowercase, bool append_start_end)
    : char_repeats(char_repeats), lowercase(lowercase), append_start_end(append_start_end), pad_token("_"), end_token("<end>") {
    
    pad_index = 0;
    token_to_idx[pad_token] = pad_index;
    special_tokens.insert(pad_token);
    
    for (const auto& lang : languages) {
        std::string lang_token = make_start_token(lang);
        token_to_idx[lang_token] = token_to_idx.size();
        special_tokens.insert(lang_token);
    }
    
    token_to_idx[end_token] = token_to_idx.size();
    end_index = token_to_idx[end_token];
    
    for (const auto& symbol : symbols) {
        token_to_idx[symbol] = token_to_idx.size();
    }
    
    for (const auto& pair : token_to_idx) {
        idx_to_token[pair.second] = pair.first;
    }
}

std::vector<int64_t> SequenceTokenizer::operator()(const std::string& sentence, const std::string& language) const {
    std::string processed_sentence = sentence;
    if (lowercase) {
        std::transform(processed_sentence.begin(), processed_sentence.end(), processed_sentence.begin(), ::tolower);
    }

    std::vector<int64_t> sequence;
    for (char c : processed_sentence) {
        std::string symbol(1, c);
        auto it = token_to_idx.find(symbol);
        if (it != token_to_idx.end()) {
            for (int i = 0; i < char_repeats; ++i) {
                sequence.push_back(it->second);
            }
        }
    }

    if (append_start_end) {
        sequence.insert(sequence.begin(), get_start_index(language));
        sequence.push_back(end_index);
    }

    return sequence;
}

std::vector<std::string> SequenceTokenizer::decode(const std::vector<int64_t>& sequence, bool remove_special_tokens) const {
    std::vector<int64_t> processed_sequence;
    if (append_start_end) {
        processed_sequence.push_back(sequence.front());
        for (size_t i = 1; i < sequence.size() - 1; i += char_repeats) {
            processed_sequence.push_back(sequence[i]);
        }
        processed_sequence.push_back(sequence.back());
    } else {
        for (size_t i = 0; i < sequence.size(); i += char_repeats) {
            processed_sequence.push_back(sequence[i]);
        }
    }

    std::vector<std::string> decoded;
    for (int64_t token : processed_sequence) {
        if (remove_special_tokens && special_tokens.count(idx_to_token.at(token))) {
            continue;
        }
        decoded.push_back(idx_to_token.at(token));
    }

    return decoded;
}

int SequenceTokenizer::get_start_index(const std::string& language) const {
    std::string lang_token = make_start_token(language);
    return token_to_idx.at(lang_token);
}

std::string SequenceTokenizer::make_start_token(const std::string& language) const {
    return "<" + language + ">";
}

Babylon::Babylon(const std::string& model_path) {
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "Babylon");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    
    session = new Ort::Session(env, model_path.c_str(), session_options);

    // Static configuration
    std::vector<std::string> languages = {"de", "en_us"};
    std::vector<std::string> text_symbols = {
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", 
        "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", 
        "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
        "ä", "ö", "ü", "Ä", "Ö", "Ü", "ß"
    };
    std::vector<std::string> phoneme_symbols = {
        "a", "b", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", 
        "o", "p", "r", "s", "t", "u", "v", "w", "x", "y", "z", "æ", "ç", 
        "ð", "ø", "ŋ", "œ", "ɐ", "ɑ", "ɔ", "ə", "ɛ", "ɝ", "ɹ", "ɡ", "ɪ", 
        "ʁ", "ʃ", "ʊ", "ʌ", "ʏ", "ʒ", "ʔ", "ˈ", "ˌ", "ː", "̃", "̍", "̥", "̩", 
        "̯", "͡", "θ"
    };
    int char_repeats = 1;
    bool lowercase = true;

    lang_tokenizer = new LanguageTokenizer(languages);
    text_tokenizer = new SequenceTokenizer(text_symbols, languages, char_repeats, lowercase);
    phoneme_tokenizer = new SequenceTokenizer(phoneme_symbols, languages, 1, false);
}

Babylon::~Babylon() {
    delete (Ort::Session*)session;
    delete lang_tokenizer;
    delete text_tokenizer;
    delete phoneme_tokenizer;
}

std::vector<std::string> Babylon::GraphemeToPhoneme(const std::string& text, const std::string& language) {
    Ort::Session* ort_session = (Ort::Session*)session;
    Ort::AllocatorWithDefaultOptions allocator;

    // Convert input text to tensor
    std::vector<Ort::Value> input_tensors;
    std::vector<int64_t> input_ids = text_tokenizer->operator()(text, language);
    std::vector<int64_t> input_shape = {1, static_cast<int64_t>(input_ids.size())};
    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    input_tensors.push_back(Ort::Value::CreateTensor<int64_t>(memory_info, input_ids.data(), input_ids.size(), input_shape.data(), input_shape.size()));

    std::array<const char *, 1> input_names = {"text"};
    std::array<const char *, 1> output_names = {"output"};
    auto output_tensors = ort_session->Run(Ort::RunOptions{nullptr}, input_names.data(), input_tensors.data(), 1, output_names.data(), 1);

    // Process the output tensor
    int64_t* output_ids = output_tensors.front().GetTensorMutableData<int64_t>();
    std::vector<int64_t> output_shape = output_tensors.front().GetTensorTypeAndShapeInfo().GetShape();

    std::vector<int64_t> output_ids_vector(output_ids, output_ids + output_shape[1]);

    // Convert output IDs to phonemes
    std::vector<std::string> phonemes = phoneme_tokenizer->decode(output_ids_vector);

    return phonemes;
}
