#include "babylon.h"
#include "babylon.hpp"
#include <iostream>

static DeepPhonemizer::Session* dp;
static Vits::Session* vits;

extern "C" {
    BABYLON_EXPORT int babylon_g2p_init(const char* model_path, const char* language, int use_punctuation) {
        try {
            dp = new DeepPhonemizer::Session(model_path, language, use_punctuation);
            return 0;
        } 
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }

    BABYLON_EXPORT char* babylon_g2p(const char* text) {
        if (dp == nullptr) {
            std::cerr << "DeepPhonemizer session not initialized." << std::endl;
            return nullptr;
        }

        std::string phonemes = "";
        try {
            std::vector<std::string> phoneme_vec = dp->g2p(text);
            for (const auto& phoneme : phoneme_vec) {
                phonemes += phoneme + " ";
            }
        } 
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

        return strdup(phonemes.c_str());
    }

    BABYLON_EXPORT int* babylon_g2p_tokens(const char* text) {
        if (dp == nullptr) {
            std::cerr << "DeepPhonemizer session not initialized." << std::endl;
            return nullptr;
        }

        std::vector<int64_t> phoneme_ids;
        try {
            phoneme_ids = dp->g2p_tokens(text);
        } 
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

        phoneme_ids.push_back(-1); // Sentinel value

        int* phoneme_ids_arr = new int[phoneme_ids.size()];
        for (size_t i = 0; i < phoneme_ids.size(); i++) {
            phoneme_ids_arr[i] = phoneme_ids[i];
        }

        return phoneme_ids_arr;
    }

    BABYLON_EXPORT void babylon_g2p_free(void) {
        delete dp;
    }

    BABYLON_EXPORT int babylon_tts_init(const char* model_path) {
        try {
            vits = new Vits::Session(model_path);
            return 0;
        } 
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }

    BABYLON_EXPORT void babylon_tts(const char* text, const char* output_path) {
        if (vits == nullptr) {
            std::cerr << "VITS session not initialized." << std::endl;
            return;
        }

        if (dp == nullptr) {
            std::cerr << "DeepPhonemizer session not initialized." << std::endl;
            return;
        }

        try {
            std::vector<std::string> phonemes = dp->g2p(text);
            vits->tts(phonemes, output_path);
        } 
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    BABYLON_EXPORT void babylon_tts_free(void) {
        delete vits;
    }
}