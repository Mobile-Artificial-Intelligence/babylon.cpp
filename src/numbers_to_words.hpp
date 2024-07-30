#include <string>
#include <vector>
#include <iostream>

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

std::vector<std::string> numbers_to_words(const std::string& text) {
    std::vector<std::string> result;
    
    for (int i = 0; i < text.length(); i++) {
        result.push_back(number_to_word(text[i] - '0'));
        std::cout << number_to_word(text[i] - '0') << std::endl;
    }

    return result;
}