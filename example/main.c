#include "babylon.h"

int main() {
    babylon_g2p_init("./models/deep_phonemizer.onnx", "en_us", 1);

    const char* text = "Hello world. There is 317 characters in this sentence.";

    babylon_tts_init("./models/curie.onnx");

    babylon_tts(text, "./output.wav");

    babylon_tts_free();

    babylon_g2p_free();

    return 0;
}