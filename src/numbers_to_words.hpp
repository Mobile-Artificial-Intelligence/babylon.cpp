#include <string>
#include <vector>

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

std::string length_to_word(int length) {
    switch (length) {
        case 0:
            return "";
        case 1:
            return "";
        case 2:
            return "";
        case 3:
            return "hundred";
        case 4:
            return "thousand";
        case 5:
            return "thousand";
        case 6:
            return "hundred thousand";
        case 7:
            return "million";
        case 8:
            return "million";
        case 9:
            return "hundred million";
        case 10:
            return "billion";
        case 11:
            return "billion";
        case 12:
            return "hundred billion";
        case 13:
            return "trillion";
        case 14:
            return "trillion";
        case 15:
            return "hundred trillion";
        case 16:
            return "quadrillion";
        case 17:
            return "quadrillion";
        case 18:
            return "hundred quadrillion";
        case 19:
            return "quintillion";
        case 20:
            return "quintillion";
    }
}

std::string tens_to_word(int tens) {
    switch (tens) {
        case 0:
            return "";
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
    }
}

std::vector<std::string> numbers_to_words(const std::string& text) {
    std::vector<std::string> result;
    
    if (text.length() > 20) {
        for (int i = 0; i < text.length(); i++) {
            result.push_back(number_to_word(text[i] - '0'));
        }
    }
    else {
        for (int i = 0; i < text.length(); i++) {
            int position = text.length() - i - 1;

            if (position < 2) {
                result.push_back(number_to_word(text[i] - '0'));
            }
            else if (position == 2 || position == 5 || position == 8 || position == 11 || position == 14 || position == 17) {
                result.push_back(tens_to_word(text[i] - '0'));
                result.push_back(length_to_word(position));
            }
            else {
                result.push_back(number_to_word(text[i] - '0'));
                result.push_back(length_to_word(position));
            }
        }
    }

    return result;
}