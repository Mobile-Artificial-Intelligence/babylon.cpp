#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

std::vector<std::string> split_into_threes(const std::string& str) {
    std::vector<std::string> parts;
    int length = str.length();

    // Process the string from the end
    for (int i = length; i > 0; i -= 3) {
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

std::string hundreds_to_words(int hundreds) {
    std::string result;
    int hundreds_digit = hundreds / 100;
    int tens_digit = (hundreds % 100) / 10;
    int ones_digit = hundreds % 10;

    if (hundreds_digit > 0) {
        result += number_to_word(hundreds_digit) + " hundred";

        if (tens_digit > 0 || ones_digit > 0) {
            result += " and";
        }
    }

    if (tens_digit > 1) {
        if (result.length() > 0) {
            result += " ";
        }
        result += tens_to_word(tens_digit);
    } else if (tens_digit == 1) {
        if (result.length() > 0) {
            result += " ";
        }
        result += teens_to_word(hundreds % 100);
    }

    if (ones_digit > 0 && tens_digit != 1) {
        if (result.length() > 0) {
            result += " ";
        }
        result += number_to_word(ones_digit);
    }

    return result;
}

namespace DeepPhonemizer {
    std::vector<std::string> numbers_to_words(const std::string& text) {
        std::vector<std::string> result;
        std::vector<std::string> parts = split_into_threes(text);
        std::vector<std::string> suffixes = {
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

        for (int i = 0; i < parts.size(); i++) {
            int number = std::stoi(parts[i]);
            result.push_back(hundreds_to_words(number));

            if (i > 0 && i < suffixes.size()) {
                result.back() += " " + suffixes[i - 1];
            }
        }

        return result;
    }
}