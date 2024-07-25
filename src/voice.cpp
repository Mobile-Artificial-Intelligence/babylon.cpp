#include "babylon.hpp"
#include "external/uni_algo.h"
#include <onnxruntime_cxx_api.h>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace Vits {
    SequenceTokenizer::SequenceTokenizer(const std::vector<std::string>& phonemes, const std::vector<const int>& phoneme_ids) {
        if (phonemes.size() != phoneme_ids.size()) {
            throw std::invalid_argument("Phonemes and phoneme IDs must have the same length.");
        }

        for (int i = 0; i < phonemes.size(); i++) {
            token_to_idx[phonemes[i][0]] = phoneme_ids[i];
        }
    }

    std::vector<int64_t> SequenceTokenizer::operator()(const std::vector<std::string>& phonemes) const {
        // Combine phonemes into a single string
        std::string phonemes_str = std::accumulate(phonemes.begin(), phonemes.end(), std::string(""));

        auto phonemes_normalized = una::norm::to_nfd_utf8(phonemes_str);
        auto phonemes_range = una::ranges::utf8_view(phonemes_normalized);
        
        std::vector<int64_t> phoneme_ids;

        for (const auto& phoneme : phonemes_range) {
            try {
                phoneme_ids.push_back(token_to_idx.at(phoneme));
            } 
            catch (const std::out_of_range&) {
                std::cerr << "Token not found: " << phoneme << std::endl;
                throw;
            }
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

    void Session::tts(const std::vector<std::string>& phonemes, const std::string& output_path) {
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        std::vector<Ort::Value> input_tensors;

        std::vector<int64_t> phoneme_ids = phoneme_tokenizer->operator()(phonemes);
        std::vector<int64_t> phoneme_ids_shape = {1, (int64_t) phoneme_ids.size()};
        input_tensors.push_back(Ort::Value::CreateTensor<int64_t>(
            memory_info, 
            phoneme_ids.data(), 
            phoneme_ids.size(), 
            phoneme_ids_shape.data(),
            phoneme_ids_shape.size()
        ));

        std::vector<int64_t> phoneme_ids_length = {(int64_t) phoneme_ids.size()};
        std::vector<int64_t> phoneme_ids_length_shape = {(int64_t) phoneme_ids_length.size()};
        input_tensors.push_back(Ort::Value::CreateTensor<int64_t>(
            memory_info, 
            phoneme_ids_length.data(), 
            phoneme_ids_length.size(),
            phoneme_ids_length_shape.data(), 
            phoneme_ids_length_shape.size()
        ));

        std::vector<int64_t> scales_shape = {(int64_t) scales.size()};
        input_tensors.push_back(Ort::Value::CreateTensor<float>(
            memory_info, 
            scales.data(), 
            scales.size(),
            scales_shape.data(), 
            scales_shape.size()
        ));

        std::vector<int64_t> sid = {0};
        std::vector<int64_t> sid_shape = {(int64_t) sid.size()};
        input_tensors.push_back(Ort::Value::CreateTensor<int64_t>(
            memory_info, 
            sid.data(), 
            sid.size(),
            sid_shape.data(), 
            sid_shape.size()
        ));

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
    }
}