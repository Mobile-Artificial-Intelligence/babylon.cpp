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

    DeepPhonemizer::Session dp(dp_model_path);

    Vits::Session vits(vits_model_path);

    std::vector<std::string> phonemes = dp.g2p(text);

    vits.tts(phonemes, "./babylon_output.wav");

    return 0;
}