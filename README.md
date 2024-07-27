<div align="center">
  <img alt="logo" height="200px" src="babylon.svg">
</div>

# babylon.cpp

[![Build MacOS](https://github.com/Mobile-Artificial-Intelligence/babylon.cpp/actions/workflows/build-macos.yml/badge.svg)](https://github.com/Mobile-Artificial-Intelligence/babylon.cpp/actions/workflows/build-macos.yml)
[![Build Linux](https://github.com/Mobile-Artificial-Intelligence/babylon.cpp/actions/workflows/build-linux.yml/badge.svg)](https://github.com/Mobile-Artificial-Intelligence/babylon.cpp/actions/workflows/build-linux.yml)

Babylon.cpp is a C and C++ library for grapheme to phoneme conversion and text to speech synthesis. 
For phonemization a ONNX runtime port of the [DeepPhonemizer](https://github.com/as-ideas/DeepPhonemizer) model is used. 
For speech synthesis [VITS](https://github.com/jaywalnut310/vits) models are used. 
[Piper](https://github.com/rhasspy/piper) models are compatible after a conversion script is run.

## Build and Run

To build and run the library run the following commands:

```bash
make
./bin/babylon_example
```

## Usage

### C Example:

```c
#include "babylon.h"

int main() {
    babylon_g2p_init("path/to/deep_phonemizer.onnx", "en_us", 1);

    const char* text = "Hello World";

    babylon_tts_init("path/to/vits.onnx");

    babylon_tts(text, "path/to/output.wav");

    babylon_tts_free();
    
    babylon_g2p_free();

    return 0;
}
```

### C++ example:

```cpp
#include "babylon.hpp"

int main() {
    DeepPhonemizer::Session dp("path/to/deep_phonemizer.onnx");

    Vits::Session vits("path/to/vits.onnx");

    std::string text = "Hello World";

    std::vector<std::string> phonemes = dp.g2p(text);

    vits.tts(phonemes, "path/to/output.wav");

    return 0;
}
```
