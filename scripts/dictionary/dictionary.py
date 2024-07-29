# Python script to convert text file into C++ unordered_map

# Read the file
file_name = "./babylon_dict.txt"
with open(file_name, "r") as file:
    lines = file.readlines()

# Process the lines to create C++ unordered_map entries
unordered_map_entries = []
for line in lines:
    parts = line.strip().split('\t')
    if len(parts) == 2:
        key = parts[0].lower()
        values = parts[1].split(' ')
        value_list = ', '.join([f'"{value}"' for value in values])
        unordered_map_entries.append(f'{{"{key}", {{ {value_list} }} }}')

# Create the C++ unordered_map code
unordered_map_code = (
    '#include <unordered_map>\n'
    '#include <vector>\n'
    '#include <string>\n\n'
    'std::unordered_map<std::string, std::vector<std::string>> myMap = {\n'
)
unordered_map_code += ',\n'.join(unordered_map_entries)
unordered_map_code += '\n};\n'

# Save the C++ code to a file
output_file_name = "output.cpp"
with open(output_file_name, "w") as output_file:
    output_file.write(unordered_map_code)

print(f"C++ unordered_map code has been written to {output_file_name}")
