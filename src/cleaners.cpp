#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>

std::unordered_map<std::string, std::string> abbreviations = {
    {"mrs", "misess"},
    {"mr", "mister"},
    {"dr", "doctor"},
    {"st", "saint"},
    {"co", "company"},
    {"jr", "junior"},
    {"maj", "major"},
    {"gen", "general"},
    {"drs", "doctors"},
    {"rev", "reverend"},
    {"lt", "lieutenant"},
    {"hon", "honorable"},
    {"sgt", "sergeant"},
    {"capt", "captain"},
    {"esq", "esquire"},
    {"ltd", "limited"},
    {"col", "colonel"},
    {"ft", "foot"},
    {"pty", "proprietary"}
};

std::vector<std::string> split_into_threes(const std::string& str) {
    std::vector<std::string> parts;

    // Process the string from the end
    for (int i = str.length(); i > 0; i -= 3) {
        if (i < 3) {
            parts.push_back(str.substr(0, i));
        } else {
            parts.push_back(str.substr(i - 3, 3));
        }
    }

    // Since we processed from the end, reverse the order of parts
    std::reverse(parts.begin(), parts.end());

    return parts;
}

std::string number_to_word(int number) {
    switch (number) {
        case 0:
            return "zero";
        case 1:
            return "one";
        case 2:
            return "two";
        case 3:
            return "three";
        case 4:
            return "four";
        case 5:
            return "five";
        case 6:
            return "six";
        case 7:
            return "seven";
        case 8:
            return "eight";
        case 9:
            return "nine";
        default:
            return std::string(1, number);
    }
}

std::string tens_to_word(int tens) {
    switch (tens) {
        case 1:
            return "ten";
        case 2:
            return "twenty";
        case 3:
            return "thirty";
        case 4:
            return "forty";
        case 5:
            return "fifty";
        case 6:
            return "sixty";
        case 7:
            return "seventy";
        case 8:
            return "eighty";
        case 9:
            return "ninety";
        default:
            return "";
    }
}

std::string teens_to_word(int teens) {
    switch (teens) {
        case 11:
            return "eleven";
        case 12:
            return "twelve";
        case 13:
            return "thirteen";
        case 14:
            return "fourteen";
        case 15:
            return "fifteen";
        case 16:
            return "sixteen";
        case 17:
            return "seventeen";
        case 18:
            return "eighteen";
        case 19:
            return "nineteen";
        default:
            return "";
    }
}

std::vector<std::string> hundreds_to_words(int hundreds) {
    const int hundreds_digit = hundreds / 100;
    const int tens_digit = (hundreds % 100) / 10;
    const int ones_digit = hundreds % 10;

    std::vector<std::string> result;

    if (hundreds_digit > 0) {
        result.push_back(number_to_word(hundreds_digit));
        result.push_back("hundred");

        if (tens_digit > 0 || ones_digit > 0) {
            result.push_back("and");
        }
    }

    if (tens_digit > 1) {
        result.push_back(tens_to_word(tens_digit));
    } else if (tens_digit == 1) {
        result.push_back(teens_to_word(hundreds % 100));
    }

    if (ones_digit > 0 && tens_digit != 1) {
        result.push_back(number_to_word(ones_digit));
    }

    return result;
}

std::vector<std::string> numbers_to_words(const std::string& text) {
    const std::vector<std::string> suffixes = {
        "thousand", 
        "million", 
        "billion", 
        "trillion", 
        "quadrillion", 
        "quintillion",
        "sextillion",
        "septillion",
        "octillion",
        "nonillion",
        "decillion"
    };

    const std::vector<std::string> parts = split_into_threes(text);

    std::vector<std::string> result;

    for (int i = 0; i < parts.size(); i++) {
        int number = std::stoi(parts[i]);
        std::vector<std::string> words = hundreds_to_words(number);

        result.insert(result.end(), words.begin(), words.end());

        if (i < suffixes.size() && i < parts.size() - 1) {
            int suffix = parts.size() - i - 2;

            result.push_back(suffixes[suffix]);
        }
    }

    return result;
}

namespace DeepPhonemizer {
    std::vector<std::string> clean_text(const std::string& text) {
        std::vector<std::string> words;

        std::stringstream ss(text);
        std::string word;
        while (ss >> word) {
            std::string cleaned_word(word);
            cleaned_word.erase(std::remove_if(cleaned_word.begin(), cleaned_word.end(), ::ispunct), cleaned_word.end());

            if (std::all_of(cleaned_word.begin(), cleaned_word.end(), ::isdigit)) {
                std::vector<std::string> number_words = numbers_to_words(cleaned_word);
                words.insert(words.end(), number_words.begin(), number_words.end());
            }
            else if (abbreviations.find(word) != abbreviations.end()) {
                words.push_back(abbreviations[word]);
            }
            else {
                words.push_back(word);
            }
        }

        return words;
    }
}