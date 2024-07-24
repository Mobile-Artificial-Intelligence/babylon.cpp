#include "babylon.hpp"
#include <iostream>

int main() {
    DeepPhonemizer::Session session("./models/deep_phonemizer.onnx");
    std::string text = "hello";
    std::string language = "en_us";

    std::vector<std::string> phonemes = session.g2p(text);

    for (const auto& phoneme : phonemes) {
        std::cout << phoneme << " ";
    }
    std::cout << std::endl;

    return 0;
}
