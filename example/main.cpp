#include "babylon.hpp"
#include <iostream>

int main() {
    DeepPhonemizer::Session dp("./models/deep_phonemizer.onnx", "en_us", true);
    std::string text = "Hello world, This is an example program for the Babylon project. Text to speech models can be used to generate speech from text. This is a very powerful tool for many applications. For example, it can be used to generate speech for virtual assistants, audiobooks, and more.";

    std::vector<std::string> phonemes = dp.g2p(text);

    for (const auto& phoneme : phonemes) {
        std::cout << phoneme << " ";
    }
    std::cout << std::endl;

    Vits::Session curie("./models/curie.onnx");

    curie.tts(phonemes, "./output.wav");

    return 0;
}
