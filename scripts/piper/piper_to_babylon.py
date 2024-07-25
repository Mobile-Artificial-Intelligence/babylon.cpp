import json
import onnx

onnx_file_path = './en_US-curie-medium-V1.4.onnx'
config_file_path = './en_US-curie-medium-V1.4.onnx.json'

# Verify the ONNX model
onnx_model = onnx.load(onnx_file_path)

# Load JSON data from file
with open(config_file_path, 'r') as file:
    json_data = file.read()

# Load JSON data
data = json.loads(json_data)

# Extract phoneme_id_map
phoneme_id_map = data['phoneme_id_map']

# Create strings for phonemes and phoneme IDs
phonemes = ' '.join(phoneme_id_map.keys().replace(' ', '<space>'))
phoneme_ids = ' '.join(str(num[0]) for num in phoneme_id_map.values())

# Add metadata to the ONNX model
metadata = {
    "phonemes": phonemes,
    "phoneme_ids": phoneme_ids
}

for key, value in metadata.items():
    meta = onnx_model.metadata_props.add()
    meta.key = key
    meta.value = str(value)

onnx.save(onnx_model, "./curie.onnx")
onnx.checker.check_model(onnx_model)