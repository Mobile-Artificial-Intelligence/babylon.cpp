#include "babylon.hpp"
#include <onnxruntime_cxx_api.h>
#include <iostream>
#include <vector>

Babylon::Babylon(const std::string& model_path) {
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "Babylon");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    
    session = new Ort::Session(env, model_path.c_str(), session_options);
}

Babylon::~Babylon() {
    delete (Ort::Session*)session;
}

std::vector<std::string> Babylon::GraphemeToPhoneme(const std::string& text) {
    Ort::Session* ort_session = (Ort::Session*)session;
    Ort::AllocatorWithDefaultOptions allocator;

    // Convert input text to tensor
    std::vector<int64_t> input_ids = /* convert text to input IDs based on your tokenizer */;
    std::vector<int64_t> input_shape = {1, static_cast<int64_t>(input_ids.size())};
    Ort::Value input_tensor = Ort::Value::CreateTensor<int64_t>(allocator, input_ids.data(), input_ids.size(), input_shape.data(), input_shape.size());

    const char* input_names[] = {"text"};
    const char* output_names[] = {"output"};
    auto output_tensors = ort_session->Run(Ort::RunOptions{nullptr}, input_names, &input_tensor, 1, output_names, 1);

    // Process the output tensor
    float* floatarr = output_tensors.front().GetTensorMutableData<float>();
    std::vector<int64_t> output_shape = output_tensors.front().GetTensorTypeAndShapeInfo().GetShape();
    int64_t num_classes = output_shape[1];

    // Convert output IDs to phonemes
    std::vector<std::string> phonemes = /* convert output tensor to phonemes based on your tokenizer */;

    return phonemes;
}
