#include "babylon.hpp"
#include <iostream>

int main() {
    DeepPhonemizer::Session session("./models/deep_phonemizer.onnx");
    std::string text = "Hello world. My name is John Doe.";

    std::vector<std::string> phonemes = session.g2p(text);

    for (const auto& phoneme : phonemes) {
        std::cout << phoneme << " ";
    }
    std::cout << std::endl;

    return 0;
}
