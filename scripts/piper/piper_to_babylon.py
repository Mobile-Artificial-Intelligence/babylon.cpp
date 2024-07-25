import onnx

onnx_file_path = './en_US-curie-medium-V1.4.onnx'

# Verify the ONNX model
onnx_model = onnx.load(onnx_file_path)

# Add metadata to the ONNX model
metadata = {
    "language": "en_us",
}

for key, value in metadata.items():
    meta = onnx_model.metadata_props.add()
    meta.key = key
    meta.value = str(value)

onnx.save(onnx_model, onnx_file_path)
onnx.checker.check_model(onnx_model)