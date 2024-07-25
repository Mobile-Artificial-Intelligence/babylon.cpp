#include "babylon.hpp"
#include <onnxruntime_cxx_api.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace VITS {
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

        std::vector<int> phoneme_ids;
        std::stringstream phoneme_id_stream(phoneme_id_str.get());
        std::string phoneme_id_buffer;
        while (phoneme_id_stream >> phoneme_id_buffer) {
            phoneme_ids.push_back(std::stoi(phoneme_id_buffer));
        }
    }

    Session::~Session() {
        delete session;
    }
}