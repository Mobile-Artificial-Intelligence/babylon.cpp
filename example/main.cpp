#include "babylon.hpp"
#include <iostream>

int main() {
    Babylon babylon("models/deep_phonemizer.onnx", "config.json");
    std::string text = "hello";
    std::string language = "en";

    std::vector<std::string> phonemes = babylon.GraphemeToPhoneme(text, language);

    for (const auto& phoneme : phonemes) {
        std::cout << phoneme << " ";
    }
    std::cout << std::endl;

    return 0;
}
