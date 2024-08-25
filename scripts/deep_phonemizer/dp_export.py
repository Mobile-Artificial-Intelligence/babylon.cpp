import torch
import onnx
from dp.model.model import AutoregressiveTransformer, ForwardTransformer, load_checkpoint

# Load your model checkpoint
checkpoint_path = './latin_ipa_forward.pt'
model, torch_meta = load_checkpoint(checkpoint_path)
config = torch_meta['config']
phoneme_dict = torch_meta['phoneme_dict']
preprocessing = config['preprocessing']

# Extract vocabulary sizes directly from the model's embedding layers
if isinstance(model, ForwardTransformer):
    print("ForwardTransformer model detected")
    encoder_vocab_size = model.embedding.num_embeddings
    decoder_vocab_size = model.fc_out.out_features
elif isinstance(model, AutoregressiveTransformer):
    print("AutoregressiveTransformer model detected")
    encoder_vocab_size = model.encoder.num_embeddings
    decoder_vocab_size = model.decoder.num_embeddings
else:
    raise ValueError("Unsupported model type")

# Define a dummy input for the model based on expected input shape and vocabulary sizes
dummy_input_forward = {
    'text': torch.randint(0, encoder_vocab_size, (1, 50)).to(torch.int64)  # Fixed batch size of 1 and sequence length of 50
}
dummy_input_autoreg = {
    'text': torch.randint(0, encoder_vocab_size, (1, 50)).to(torch.int64),  # Fixed batch size of 1 and sequence length of 50
    'phonemes': torch.randint(0, decoder_vocab_size, (1, 50)).to(torch.int64),  # Fixed batch size of 1 and sequence length of 50
    'start_index': torch.randint(0, decoder_vocab_size, (1,)).to(torch.int64)  # Fixed batch size of 1
}

# Choose the appropriate dummy input based on your model type
if isinstance(model, ForwardTransformer):
    dummy_input = dummy_input_forward
    input_names = ['text']
elif isinstance(model, AutoregressiveTransformer):
    dummy_input = dummy_input_autoreg
    input_names = ['text', 'phonemes', 'start_index']
else:
    raise ValueError("Unsupported model type")

# Create a wrapper module to handle the batch input
class WrapperModule(torch.nn.Module):
    def __init__(self, model):
        super(WrapperModule, self).__init__()
        self.model = model

    def forward(self, text, phonemes=None, start_index=None):
        batch = {'text': text}
        if phonemes is not None:
            batch['phonemes'] = phonemes
        if start_index is not None:
            batch['start_index'] = start_index
        return self.model(batch)

wrapped_model = WrapperModule(model)

# Set model to evaluation mode
wrapped_model.eval()

# Convert model to ONNX format with fixed input shape
onnx_file_path = './deep_phonemizer.onnx'
torch.onnx.export(
    wrapped_model,
    args=(dummy_input['text'], dummy_input.get('phonemes'), dummy_input.get('start_index')),
    f=onnx_file_path,
    opset_version=14,
    input_names=input_names,
    output_names=['output']
)

# Verify the ONNX model
onnx_model = onnx.load(onnx_file_path)

languages = ''
for language in preprocessing['languages']:
    languages += language + ' '
print(f"Languages: {languages}")

text_symbols = ''
for symbol in preprocessing['text_symbols']:
    text_symbols += symbol + ' '
print(f"Text symbols: {text_symbols}")

phoneme_symbols = ''
for symbol in preprocessing['phoneme_symbols']:
    phoneme_symbols += symbol + ' '
phoneme_symbols += '. , : ; ? ! \" ( ) -'
print(f"Phoneme symbols: {phoneme_symbols}")

char_repeats = f"{preprocessing['char_repeats']}"
print(f"Char repeats: {char_repeats}")

lowercase = f"{'1' if preprocessing['lowercase'] else '0'}"
print(f"Lowercase: {lowercase}")

n_val = f"{preprocessing['n_val']}"
print(f"n_val: {n_val}")

# Add metadata to the ONNX model
metadata = {
    "languages": languages,
    "text_symbols": text_symbols,
    "phoneme_symbols": phoneme_symbols,
    "char_repeats": char_repeats,
    "lowercase": lowercase,
    "n_val": n_val
}

for language in preprocessing['languages']:
    language_dict = phoneme_dict[language]
    language_dict_str = "\n".join(f"{key}\t{value}" for key, value in language_dict.items())
    metadata[f"{language}_dictionary"] = language_dict_str

print(metadata.keys())

for key, value in metadata.items():
    meta = onnx_model.metadata_props.add()
    meta.key = key
    meta.value = str(value)

onnx.save(onnx_model, onnx_file_path)
onnx.checker.check_model(onnx_model)
print(f"Model successfully converted to {onnx_file_path} with fixed input shape and metadata added")
