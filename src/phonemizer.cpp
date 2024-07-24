#include "babylon.hpp"
#include <onnxruntime_cxx_api.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace DeepPhonemizer {
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

        // Pad the sequence to the maximum length (50)
        int max_length = 50;
        while (sequence.size() < max_length) {
            sequence.push_back(pad_index);
        }

        return sequence;
    }

    std::vector<std::string> SequenceTokenizer::decode(const std::vector<int64_t>& sequence, bool remove_special_tokens) const {
        std::vector<int64_t> pruned_sequence = sequence;
        pruned_sequence.erase(
            std::remove(pruned_sequence.begin(), pruned_sequence.end(), pad_index),
            pruned_sequence.end()
        );

        std::vector<int64_t> processed_sequence;
        if (append_start_end) {
            processed_sequence.push_back(pruned_sequence.front());
            for (size_t i = 1; i < pruned_sequence.size() - 1; i += char_repeats) {
                processed_sequence.push_back(pruned_sequence[i]);
            }
            processed_sequence.push_back(pruned_sequence.back());
        } else {
            for (size_t i = 0; i < pruned_sequence.size(); i += char_repeats) {
                processed_sequence.push_back(pruned_sequence[i]);
            }
        }

        // Remove consecutive duplicate tokens
        auto last = std::unique(processed_sequence.begin(), processed_sequence.end());
        processed_sequence.erase(last, processed_sequence.end());

        std::vector<std::string> decoded;
        for (int64_t token : processed_sequence) {
            if (token == end_index) {
                break;
            }
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

    std::vector<float> softmax(const std::vector<float>& logits) {
        std::vector<float> probabilities(logits.size());
        float max_logit = *std::max_element(logits.begin(), logits.end());
        float sum = 0.0;

        for (size_t i = 0; i < logits.size(); ++i) {
            probabilities[i] = std::exp(logits[i] - max_logit);
            sum += probabilities[i];
        }

        for (size_t i = 0; i < logits.size(); ++i) {
            probabilities[i] /= sum;
        }

        return probabilities;
    }

    Session::Session(const std::string& model_path, const std::string language) {
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "DeepPhonemizer");
        env.DisableTelemetryEvents();

        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        session = new Ort::Session(env, model_path.c_str(), session_options);

        // Load metadata from the model
        Ort::ModelMetadata model_metadata = session->GetModelMetadata();
        Ort::AllocatorWithDefaultOptions allocator;

        Ort::AllocatedStringPtr langs_ptr = model_metadata.LookupCustomMetadataMapAllocated("languages", allocator);

        std::vector<std::string> languages;
        std::stringstream languages_stream(langs_ptr.get());
        std::string language_buffer;
        while (languages_stream >> language_buffer) {
            languages.push_back(language_buffer);
        }

        Ort::AllocatedStringPtr text_symbols_ptr = model_metadata.LookupCustomMetadataMapAllocated("text_symbols", allocator);

        std::vector<std::string> text_symbols;
        std::stringstream text_symbols_stream(text_symbols_ptr.get());
        std::string text_symbol_buffer;
        while (text_symbols_stream >> text_symbol_buffer) {
            text_symbols.push_back(text_symbol_buffer);
        }

        Ort::AllocatedStringPtr phoneme_symbols_ptr = model_metadata.LookupCustomMetadataMapAllocated("phoneme_symbols", allocator);

        std::vector<std::string> phoneme_symbols;
        std::stringstream phoneme_symbols_stream(phoneme_symbols_ptr.get());
        std::string phoneme_symbol_buffer;
        while (phoneme_symbols_stream >> phoneme_symbol_buffer) {
            phoneme_symbols.push_back(phoneme_symbol_buffer);
        }

        int char_repeats = model_metadata.LookupCustomMetadataMapAllocated("char_repeats", allocator).get()[0] - '0';

        bool lowercase = model_metadata.LookupCustomMetadataMapAllocated("lowercase", allocator).get()[0] == '1';

        if (std::find(languages.begin(), languages.end(), language) == languages.end()) {
            throw std::runtime_error("Language not supported.");
        }

        lang = language;
        text_tokenizer = new SequenceTokenizer(text_symbols, languages, char_repeats, lowercase);
        phoneme_tokenizer = new SequenceTokenizer(phoneme_symbols, languages, 1, false);
    }

    Session::~Session() {
        delete session;
        delete text_tokenizer;
        delete phoneme_tokenizer;
    }

    std::vector<std::string> Session::g2p(const std::string& text) {
        // Split sentence into words
        std::vector<std::string> words;
        
        std::stringstream ss(text);
        std::string word;
        while (ss >> word) {
            words.push_back(word);
        }

        // Convert each word to phonemes
        std::vector<std::string> phonemes;
        for (const auto& word : words) {
            std::vector<std::string> word_phonemes = g2p_internal(word);
            phonemes.insert(phonemes.end(), word_phonemes.begin(), word_phonemes.end());
            phonemes.push_back(" ");
        }

        return phonemes;
    }

    std::vector<std::string> Session::g2p_internal(const std::string& text) {
        // Convert input text to tensor
        std::vector<Ort::Value> input_tensors;
        std::vector<int64_t> input_ids = text_tokenizer->operator()(text, lang);

        std::vector<int64_t> input_shape = {1, static_cast<int64_t>(input_ids.size())};
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        input_tensors.push_back(Ort::Value::CreateTensor<int64_t>(memory_info, input_ids.data(), input_ids.size(), input_shape.data(), input_shape.size()));

        std::array<const char *, 1> input_names = {"text"};
        std::array<const char *, 1> output_names = {"output"};
        auto output_tensors = session->Run(Ort::RunOptions{nullptr}, input_names.data(), input_tensors.data(), 1, output_names.data(), 1);

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

        // Convert output IDs to phonemes
        std::vector<std::string> phonemes = phoneme_tokenizer->decode(output_ids_vector, true);

        return phonemes;
    }
}