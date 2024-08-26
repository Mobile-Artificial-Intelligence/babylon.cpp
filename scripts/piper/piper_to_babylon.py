import json
import onnx

onnx_file_path = './en_US-amy-medium.onnx'
config_file_path = './en_US-amy-medium.onnx.json'

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
phonemes = ' '.join(phoneme.replace(' ', '<space>') for phoneme in phoneme_id_map.keys())
phoneme_ids = ' '.join(str(num[0]) for num in phoneme_id_map.values())

# Add metadata to the ONNX model
metadata = {
    "phonemes": phonemes,
    "phoneme_ids": phoneme_ids,
    "sample_rate": data['audio']['sample_rate'],
    "noise_scale": data['inference']['noise_scale'],
    "length_scale": data['inference']['length_scale'],
    "noise_w": data['inference']['noise_w'],
}

for key, value in metadata.items():
    meta = onnx_model.metadata_props.add()
    meta.key = key
    meta.value = str(value)

onnx.save(onnx_model, "./amy.onnx")
onnx.checker.check_model(onnx_model)

print("Metadata added successfully!")