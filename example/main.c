#include "babylon.h"

int main() {
    babylon_g2p_init("./models/deep_phonemizer.onnx", "en_us", 1);

    const char* text = "Hello world. There is 316 characters in this sentence. This is an example program for the Babylon project. Text to speech models can be used to generate speech from text. This is a very powerful tool for many applications. For example, it can be used to generate speech for virtual assistants, audiobooks, and more.";

    babylon_tts_init("./models/curie.onnx");

    babylon_tts(text, "./output.wav");

    babylon_tts_free();

    babylon_g2p_free();

    return 0;
}