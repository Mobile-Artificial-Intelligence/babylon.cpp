#include "babylon.hpp"
#include <iostream>

int main(int argc, char** argv) {
    std::string dp_model_path = "./models/deep_phonemizer.onnx";
    std::string vits_model_path = "./models/curie.onnx";
    std::string text;

    if (argc < 2) {
        std::cerr << "Usage: babylon <text>" << std::endl;
        return 1;
    }

    text = argv[1];

    DeepPhonemizer::Session dp(dp_model_path, "en_us", true);

    Vits::Session vits(vits_model_path);

    std::vector<std::string> phonemes = dp.g2p(text);

    for (const auto& phoneme : phonemes) {
        std::cout << phoneme << " ";
    }
    std::cout << std::endl;

    vits.tts(phonemes, "./babylon_output.wav");

    std::vector<int64_t> phoneme_ids = dp.g2p_tokens(text);

    for (const auto& id : phoneme_ids) {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    return 0;
}