#include "babylon.hpp"
#include <onnxruntime_cxx_api.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

struct WavHeader {
  uint8_t RIFF[4] = {'R', 'I', 'F', 'F'};
  uint32_t chunk_size;
  uint8_t WAVE[4] = {'W', 'A', 'V', 'E'};

  // fmt
  uint8_t fmt[4] = {'f', 'm', 't', ' '};
  uint32_t fmt_size = 16;       // bytes
  uint16_t audio_format = 1;    // PCM
  uint16_t num_channels;        // mono
  uint32_t sample_rate;         // Hertz
  uint32_t bytes_per_second;    // sample_rate * sample_width
  uint16_t block_align = 2;     // 16-bit mono
  uint16_t bits_per_sample = 16;

  // data
  uint8_t data[4] = {'d', 'a', 't', 'a'};
  uint32_t data_size;
};

namespace Vits {
    const float FMIN = static_cast<float>(std::numeric_limits<int16_t>::min());
    const float FMAX = static_cast<float>(std::numeric_limits<int16_t>::max());

    SequenceTokenizer::SequenceTokenizer(const std::vector<std::string>& phonemes, const std::vector<int>& phoneme_ids) {
        if (phonemes.size() != phoneme_ids.size()) {
            throw std::invalid_argument("Phonemes and phoneme IDs must have the same length.");
        }

        for (int i = 0; i < phonemes.size(); i++) {
            token_to_idx[phonemes[i]] = phoneme_ids[i];
        }
    }

    std::vector<int64_t> SequenceTokenizer::operator()(const std::vector<std::string>& phonemes) const {
        std::vector<int64_t> phoneme_ids = {1, 0};
        for (const auto& phoneme : phonemes) {
            try {
                phoneme_ids.push_back(token_to_idx.at(phoneme));
                phoneme_ids.push_back(0);
            } 
            catch (const std::out_of_range&) {
                std::cerr << "Token not found: " << phoneme << std::endl;
                //throw;
            }
        }

        phoneme_ids.push_back(2);

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

        std::vector<int> phoneme_ids;
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

        // Print ids
        for (int64_t i = 0; i < phoneme_ids.size(); i++) {
            std::cout << phoneme_ids[i] << " ";
        }
        std::cout << std::endl;

        //std::string test_data = "1 0 32 0 120 0 61 0 23 0 31 0 32 0 3 0 32 0 59 0 3 0 31 0 28 0 120 0 21 0 122 0 32 0 96 0 3 0 25 0 120 0 51 0 122 0 17 0 59 0 24 0 38 0 3 0 23 0 39 0 26 0 3 0 15 0 21 0 122 0 3 0 22 0 120 0 33 0 122 0 38 0 17 0 3 0 32 0 59 0 3 0 17 0 108 0 120 0 61 0 26 0 60 0 88 0 121 0 18 0 74 0 32 0 3 0 31 0 28 0 120 0 21 0 122 0 32 0 96 0 3 0 19 0 88 0 102 0 25 0 3 0 32 0 120 0 61 0 23 0 31 0 32 0 10 0 2";
        //std::vector<int64_t> phoneme_ids;
        //std::stringstream phoneme_stream(test_data);
        //std::string phoneme_buffer;
        //while (phoneme_stream >> phoneme_buffer) {
        //    phoneme_ids.push_back(std::stoi(phoneme_buffer));
        //}

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

        const float *output_data = output_tensors.front().GetTensorData<float>();
        std::vector<int64_t> output_shape = output_tensors.front().GetTensorTypeAndShapeInfo().GetShape();
        int64_t output_count = output_shape[output_shape.size() - 1];

        // Get max audio value for scaling
        float max_output_value = 0.01f;
        for (int64_t i = 0; i < output_count; i++) {
          float output_value = abs(output_data[i]);
          if (output_value > max_output_value) {
            max_output_value = output_value;
          }
        }

        std::vector<int16_t> audio_data;
        audio_data.reserve(output_count);

        // Scale audio to fill range and convert to int16
        float audio_scale = (32767.0f / std::max(0.01f, max_output_value));
        for (int64_t i = 0; i < output_count; i++) {
            float audio_value = output_data[i] * audio_scale;
            float clamped_audio_value = std::clamp(audio_value, FMIN, FMAX);
            int16_t int_audio_value = static_cast<int16_t>(clamped_audio_value);
            audio_data.push_back(int_audio_value);
        }

        std::ofstream audio_file(output_path, std::ios::binary);
        
        int sample_width = 2;
        int channels = 1;

        WavHeader header;
        header.data_size = audio_data.size() * sample_width * channels;
        header.chunk_size = header.data_size + sizeof(WavHeader) - 8;
        header.sample_rate = sample_rate;
        header.num_channels = channels;
        header.bytes_per_second = sample_rate * sample_width * channels;
        header.block_align = sample_width * channels;

        audio_file.write(reinterpret_cast<const char *>(&header), sizeof(header));
        audio_file.write((const char *) audio_data.data(), sizeof(int16_t) * audio_data.size());
    }
}