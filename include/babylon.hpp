#ifndef BABYLON_HPP
#define BABYLON_HPP

#include <string>
#include <vector>

class Babylon {
public:
    Babylon(const std::string& model_path);
    ~Babylon();

    std::vector<std::string> GraphemeToPhoneme(const std::string& text);

private:
    void* session; // Placeholder for ONNX Runtime session
};

#endif // BABYLON_H
