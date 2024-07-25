#include "babylon.hpp"
#include <onnxruntime_cxx_api.h>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace Vits {
    SequenceTokenizer::SequenceTokenizer(const std::vector<std::string>& phonemes, const std::vector<const int>& phoneme_ids) {
        if (phonemes.size() != phoneme_ids.size()) {
            throw std::invalid_argument("Phonemes and phoneme IDs must have the same length.");
        }

        for (int i = 0; i < phonemes.size(); i++) {
            token_to_idx[phonemes[i]] = phoneme_ids[i];
        }
    }

    std::vector<int64_t> SequenceTokenizer::operator()(const std::vector<std::string>& phonemes) const {
        std::vector<int64_t> phoneme_ids;
        for (const auto& phoneme : phonemes) {
            phoneme_ids.push_back(token_to_idx.at(phoneme));
        }

        return phoneme_ids;
    }

    Session::Session(const std::string& model_path) {
        Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "VITS");
        env.DisableTelemetryEvents();

        Ort::SessionOptions session_options;
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_DISABLE_ALL);
        session_options.DisableCpuMemArena();
        session_options.DisableMemPattern();
        session_options.DisableProfiling();

        session = new Ort::Session(env, model_path.c_str(), session_options);

        // Load metadata from the model
        Ort::ModelMetadata model_metadata = session->GetModelMetadata();
        Ort::AllocatorWithDefaultOptions allocator;

        // Load phonemes
        Ort::AllocatedStringPtr phoneme_str = model_metadata.LookupCustomMetadataMapAllocated("phonemes", allocator);

        std::vector<std::string> phonemes;
        std::stringstream phoneme_stream(phoneme_str.get());
        std::string phoneme_buffer;
        while (phoneme_stream >> phoneme_buffer) {
            if (phoneme_buffer == "<space>") {
                phoneme_buffer = " ";
            }

            phonemes.push_back(phoneme_buffer);
        }

        // Load phoneme IDs
        Ort::AllocatedStringPtr phoneme_id_str = model_metadata.LookupCustomMetadataMapAllocated("phoneme_ids", allocator);

        std::vector<const int> phoneme_ids;
        std::stringstream phoneme_id_stream(phoneme_id_str.get());
        std::string phoneme_id_buffer;
        while (phoneme_id_stream >> phoneme_id_buffer) {
            phoneme_ids.push_back(std::stoi(phoneme_id_buffer));
        }

        sample_rate = std::stoi(model_metadata.LookupCustomMetadataMapAllocated("sample_rate", allocator).get());

        float noise_scale = std::stof(model_metadata.LookupCustomMetadataMapAllocated("noise_scale", allocator).get());

        float length_scale = std::stof(model_metadata.LookupCustomMetadataMapAllocated("length_scale", allocator).get());

        float noise_w = std::stof(model_metadata.LookupCustomMetadataMapAllocated("noise_w", allocator).get());

        scales = {noise_scale, length_scale, noise_w};

        phoneme_tokenizer = new SequenceTokenizer(phonemes, phoneme_ids);
    }

    Session::~Session() {
        delete session;
        delete phoneme_tokenizer;
    }
}