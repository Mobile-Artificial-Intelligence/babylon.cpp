#include "babylon.hpp"
#include <iostream>

int main() {
    DeepPhonemizer::Session dp("./models/deep_phonemizer.onnx", "en_us", true);
    std::string text = "Hello world, This is an example program for the Babylon project.";

    std::vector<std::string> phonemes = dp.g2p(text);

    for (const auto& phoneme : phonemes) {
        std::cout << phoneme << " ";
    }
    std::cout << std::endl;

    Vits::Session curie("./models/curie.onnx");

    curie.tts(phonemes, "./output.wav");

    return 0;
}
