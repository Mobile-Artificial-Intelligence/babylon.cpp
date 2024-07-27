<div align="center">
  <img alt="logo" height="200px" src="babylon.svg">
</div>

# babylon.cpp

Babylon.cpp is a C++ library for grapheme to phoneme conversion and text to speech synthesis. 
For phonemization a ONNX runtime port of the [DeepPhonemizer](https://github.com/as-ideas/DeepPhonemizer) model is used. 
For speech synthesis [VITS](https://github.com/jaywalnut310/vits) models are used. 
[Piper](https://github.com/rhasspy/piper) models are compatible after a conversion script is run.

## Build and Run

To build and run the library run the following commands:

```bash
make
./bin/babylon_example
```