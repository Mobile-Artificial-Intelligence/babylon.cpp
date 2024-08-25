#include "babylon.hpp"
#include <onnxruntime_cxx_api.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

const std::array<const char *, 1> input_names = {"text"};
const std::array<const char *, 1> output_names = {"output"};

std::vector<float> softmax(const std::vector<float>& logits) {
    float max_logit = *std::max_element(logits.begin(), logits.end());
    std::vector<float> probabilities(logits.size());

    float sum = 0.0f;
    for (float logit : logits) {
        sum += std::exp(logit - max_logit);
    }

    for (size_t i = 0; i < logits.size(); ++i) {
        probabilities[i] = std::exp(logits[i] - max_logit) / sum;
    }

    return probabilities;
}

std::unordered_map<std::string, std::vector<std::string>> process_dictionary(const std::string& dictonary_str) {
    std::unordered_map<std::string, std::vector<std::string>> dictionary;
    std::istringstream dictionary_stream(dictonary_str);

    std::string line;
    while (std::getline(dictionary_stream, line)) {
        std::stringstream line_stream(line);
        std::string word;
        line_stream >> word;
        std::vector<std::string> phonemes;
        std::string phoneme;
        while (line_stream >> phoneme) {
            phonemes.push_back(phoneme);
        }
        dictionary[word] = phonemes;
    }

    return dictionary;
}

namespace DeepPhonemizer {
    SequenceTokenizer::SequenceTokenizer(const std::vector<std::string>& symbols, const std::vector<std::string>& languages, int char_repeats, bool lowercase, bool append_start_end)
        : char_repeats(char_repeats), lowercase(lowercase), append_start_end(append_start_end), pad_token(" "), end_token("<end>") {
        
        tokens.push_back(pad_token);
        special_tokens.insert(pad_token);

        for (const auto& lang : languages) {
            std::string lang_token = "<" + lang + ">";
            tokens.push_back(lang_token);
            special_tokens.insert(lang_token);
        }

        tokens.push_back(end_token);
        end_index = tokens.size() - 1;

        for (const auto& symbol : symbols) {
            tokens.push_back(symbol);
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
            auto index = get_token(symbol);
            if (index != -1) {
                for (int i = 0; i < char_repeats; ++i) {
                    sequence.push_back(index);
                }
            }
        }

        if (append_start_end) {
            auto index = get_token("<" + language + ">");
            sequence.insert(sequence.begin(), index);
            sequence.push_back(end_index);
        }

        // Pad the sequence to the maximum length (50)
        int max_length = 50;
        while (sequence.size() < max_length) {
            sequence.push_back(pad_index);
        }

        if (sequence.size() > max_length) {
            sequence.resize(max_length);
        }

        return sequence;
    }

    std::vector<std::string> SequenceTokenizer::decode(const std::vector<int64_t>& sequence) const {
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
            if (token == end_index) {
                break;
            }
            decoded.push_back(tokens[token]);
        }

        return decoded;
    }

    std::vector<int64_t> SequenceTokenizer::clean(const std::vector<int64_t>& sequence) const {
        std::vector<int64_t> processed_sequence = sequence;

        // remove all special tokens from the sequence
        for (auto token : special_tokens) {
            auto special_token_index = get_token(token);
            if (special_token_index != -1) {
                processed_sequence.erase(std::remove(processed_sequence.begin(), processed_sequence.end(), special_token_index), processed_sequence.end());
            }
        }
        
        // extract everything between the start and end tokens
        auto end = std::find(processed_sequence.begin(), processed_sequence.end(), end_index);
        if (end != processed_sequence.end()) {
            processed_sequence.erase(end, processed_sequence.end());
        }

        // Remove consecutive duplicate tokens
        auto last = std::unique(processed_sequence.begin(), processed_sequence.end());
        processed_sequence.erase(last, processed_sequence.end());
        
        return processed_sequence;
    }

    int64_t SequenceTokenizer::get_token(const std::string& token) const {
        auto it = std::find(tokens.begin(), tokens.end(), token);

        if (it != tokens.end()) {
            return std::distance(tokens.begin(), it);
        }

        return -1;
    }

    Session::Session(const std::string& model_path, const std::string language, const bool use_dictionaries, const bool use_punctuation) {
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "DeepPhonemizer");
        env.DisableTelemetryEvents();

        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        this->session = new Ort::Session(env, (const ORTCHAR_T *) model_path.c_str(), session_options);

        // Load metadata from the model
        Ort::ModelMetadata model_metadata = session->GetModelMetadata();
        Ort::AllocatorWithDefaultOptions allocator;

        std::string langs_str = model_metadata.LookupCustomMetadataMapAllocated("languages", allocator).get();

        std::vector<std::string> languages;
        std::stringstream languages_stream(langs_str);
        std::string language_buffer;
        while (languages_stream >> language_buffer) {
            languages.push_back(language_buffer);
        }

        std::string text_symbols_str = model_metadata.LookupCustomMetadataMapAllocated("text_symbols", allocator).get();

        std::vector<std::string> text_symbols;
        std::stringstream text_symbols_stream(text_symbols_str);
        std::string text_symbol_buffer;
        while (text_symbols_stream >> text_symbol_buffer) {
            text_symbols.push_back(text_symbol_buffer);
        }

        std::string phoneme_symbols_str = model_metadata.LookupCustomMetadataMapAllocated("phoneme_symbols", allocator).get();

        std::vector<std::string> phoneme_symbols;
        std::stringstream phoneme_symbols_stream(phoneme_symbols_str);
        std::string phoneme_symbol_buffer;
        while (phoneme_symbols_stream >> phoneme_symbol_buffer) {
            phoneme_symbols.push_back(phoneme_symbol_buffer);
        }

        if (use_dictionaries) {
            for (const auto& lang : languages) {
                std::string key = lang + "_dictionary";
                std::string dictonary_str = model_metadata.LookupCustomMetadataMapAllocated(key.c_str(), allocator).get();
                this->dictionaries[lang] = process_dictionary(dictonary_str);
            }
        }

        int char_repeats = model_metadata.LookupCustomMetadataMapAllocated("char_repeats", allocator).get()[0] - '0';

        bool lowercase = model_metadata.LookupCustomMetadataMapAllocated("lowercase", allocator).get()[0] == '1';

        if (std::find(languages.begin(), languages.end(), language) == languages.end()) {
            throw std::runtime_error("Language not supported.");
        }

        this->language = language;
        this->use_dictionaries = use_dictionaries;
        this->use_punctuation = use_punctuation;
        this->text_tokenizer = new SequenceTokenizer(text_symbols, languages, char_repeats, lowercase);
        this->phoneme_tokenizer = new SequenceTokenizer(phoneme_symbols, languages, 1, false);
    }

    Session::~Session() {
        delete session;
        delete text_tokenizer;
        delete phoneme_tokenizer;
    }

    std::vector<std::string> Session::g2p(const std::string& text) {
        // Convert input text to phonemes
        std::vector<int64_t> phoneme_tokens = g2p_tokens(text);

        // Decode the phoneme tokens
        return phoneme_tokenizer->decode(phoneme_tokens);
    }

    std::vector<int64_t> Session::g2p_tokens(const std::string& text) {
        // Clean the input text
        std::vector<std::string> words = clean_text(text);

        // Convert each word to phonemes
        std::vector<int64_t> phoneme_ids;
        for (const auto& word : words) {
            std::vector<int64_t> word_phoneme_ids = g2p_tokens_internal(word);

            std::vector<int64_t> cleaned_word_phoneme_ids = phoneme_tokenizer->clean(word_phoneme_ids);
            
            phoneme_ids.insert(phoneme_ids.end(), cleaned_word_phoneme_ids.begin(), cleaned_word_phoneme_ids.end());

            if (use_punctuation) {
                auto back_token = phoneme_tokenizer->get_token(std::string(1, word.back()));

                // Check if the word ends with punctuation
                if (std::ispunct(word.back()) && back_token != -1) {
                    phoneme_ids.push_back(back_token);
                }
            }

            phoneme_ids.push_back(0);
        }

        return phoneme_ids;
    }

    std::vector<int64_t> Session::g2p_tokens_internal(const std::string& text) {
        // Check if the input text is longer than one character
        std::string key_text = text;
        std::transform(key_text.begin(), key_text.end(), key_text.begin(), ::tolower);

        key_text.erase(std::remove_if(key_text.begin(), key_text.end(), ::ispunct), key_text.end());

        // First check if word is in the dictionary
        if (dictionaries[language].count(key_text) && use_dictionaries) {
            auto token_str = dictionaries[language].at(key_text);

            std::vector<int64_t> tokens;
            for (const auto& token : token_str) {
                tokens.push_back(phoneme_tokenizer->get_token(token));
            }

            return tokens;
        }

        // Convert input text to tensor
        std::vector<Ort::Value> input_tensors;
        std::vector<int64_t> input_ids = text_tokenizer->operator()(text, language);

        std::vector<int64_t> input_shape = {1, static_cast<int64_t>(input_ids.size())};
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        // Create input tensor
        input_tensors.push_back(Ort::Value::CreateTensor<int64_t>(
            memory_info, 
            input_ids.data(), 
            input_ids.size(), 
            input_shape.data(), 
            input_shape.size()
        ));

        // Run the model
        std::vector<Ort::Value> output_tensors = session->Run(
            Ort::RunOptions{nullptr}, 
            input_names.data(), 
            input_tensors.data(), 
            input_names.size(), 
            output_names.data(), 
            output_names.size()
        );

        // Check if output tensor is valid
        if (output_tensors.empty()) {
            throw std::runtime_error("No output tensor returned from the model.");
        }

        // Process the output tensor
        const float* output_data = output_tensors.front().GetTensorData<float>();
        std::vector<int64_t> output_shape = output_tensors.front().GetTensorTypeAndShapeInfo().GetShape();

        // Ensure the output shape is as expected: {1, 50, 53}
        if (output_shape.size() != 3 || output_shape[0] != 1 || output_shape[1] != 50 || output_shape[2] != 53) {
            throw std::runtime_error("Unexpected output shape from the model.");
        }

        // Decode the output: find the index with the highest probability at each position
        std::vector<int64_t> output_ids_vector(output_shape[1]);
        for (size_t i = 0; i < output_shape[1]; ++i) {
            std::vector<float> logits(output_data + i * output_shape[2], output_data + (i + 1) * output_shape[2]);
            std::vector<float> probabilities = softmax(logits);

            auto max_prob_iter = std::max_element(probabilities.begin(), probabilities.end());
            output_ids_vector[i] = std::distance(probabilities.begin(), max_prob_iter);
        }

        return output_ids_vector;
    }
}