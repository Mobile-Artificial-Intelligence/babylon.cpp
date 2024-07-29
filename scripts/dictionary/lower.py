def convert_words_to_lowercase(input_file, output_file):
    with open(input_file, 'r') as file:
        lines = file.readlines()
    
    with open(output_file, 'w') as file:
        for line in lines:
            word, phonemes = line.strip().split(maxsplit=1)
            file.write(f"{word.lower()}\t{phonemes}\n")

# Specify the input and output file names
input_file = 'babylon_dict.txt'
output_file = 'babylon_dict_lower.txt'

# Convert words to lowercase
convert_words_to_lowercase(input_file, output_file)

print("Conversion completed successfully!")
