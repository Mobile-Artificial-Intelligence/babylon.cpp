#ifndef BABYLON_HPP
#define BABYLON_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class LanguageTokenizer {
public:
    LanguageTokenizer(const std::vector<std::string>& languages);
    int operator()(const std::string& lang) const;
    std::string decode(int index) const;

private:
    std::unordered_map<std::string, int> lang_index;
    std::unordered_map<int, std::string> index_lang;
};

class SequenceTokenizer {
public:
    SequenceTokenizer(const std::vector<std::string>& symbols, const std::vector<std::string>& languages, int char_repeats, bool lowercase = true, bool append_start_end = true);
    std::vector<int> operator()(const std::string& sentence, const std::string& language) const;
    std::string decode(const std::vector<int>& sequence, bool remove_special_tokens = false) const;

private:
    std::unordered_map<std::string, int> token_to_idx;
    std::unordered_map<int, std::string> idx_to_token;
    int char_repeats;
    bool lowercase;
    bool append_start_end;
    int pad_index;
    int end_index;
    std::string pad_token;
    std::string end_token;
    std::unordered_set<std::string> special_tokens;

    int get_start_index(const std::string& language) const;
    std::string make_start_token(const std::string& language) const;
};

class Babylon {
public:
    Babylon(const std::string& model_path);
    ~Babylon();

    std::vector<std::string> GraphemeToPhoneme(const std::string& text, const std::string& language);

private:
    void* session; // Placeholder for ONNX Runtime session
    LanguageTokenizer* lang_tokenizer;
    SequenceTokenizer* text_tokenizer;
    SequenceTokenizer* phoneme_tokenizer;
};

#endif // BABYLON_H
