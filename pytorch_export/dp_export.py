import torch
import onnx
from typing import Dict

from dp.model.model import AutoregressiveTransformer, ForwardTransformer, load_checkpoint

# Load your model checkpoint
checkpoint_path = './en_us_cmudict_ipa_forward.pt'
model, config = load_checkpoint(checkpoint_path)

# Extract vocabulary sizes directly from the model's embedding layers
if isinstance(model, ForwardTransformer):
    encoder_vocab_size = model.embedding.num_embeddings
    decoder_vocab_size = model.fc_out.out_features
elif isinstance(model, AutoregressiveTransformer):
    encoder_vocab_size = model.encoder.num_embeddings
    decoder_vocab_size = model.decoder.num_embeddings
else:
    raise ValueError("Unsupported model type")

# Define a dummy input for the model based on expected input shape and vocabulary sizes
dummy_input_forward = {
    'text': torch.randint(0, encoder_vocab_size, (10, 50)).to(torch.int64)
}
dummy_input_autoreg = {
    'text': torch.randint(0, encoder_vocab_size, (10, 50)).to(torch.int64),
    'phonemes': torch.randint(0, decoder_vocab_size, (10, 50)).to(torch.int64),
    'start_index': torch.randint(0, decoder_vocab_size, (10,)).to(torch.int64)
}

# Choose the appropriate dummy input based on your model type
if isinstance(model, ForwardTransformer):
    dummy_input = dummy_input_forward
    input_names = ['text']
    dynamic_axes = {'text': {0: 'batch_size', 1: 'sequence_length'}}
elif isinstance(model, AutoregressiveTransformer):
    dummy_input = dummy_input_autoreg
    input_names = ['text', 'phonemes', 'start_index']
    dynamic_axes = {
        'text': {0: 'batch_size', 1: 'sequence_length'},
        'phonemes': {0: 'batch_size', 1: 'sequence_length'},
        'start_index': {0: 'batch_size'}
    }
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

# Convert model to ONNX format
onnx_file_path = './deep_phonemizer.onnx'
torch.onnx.export(
    wrapped_model,
    args=(dummy_input['text'], dummy_input.get('phonemes'), dummy_input.get('start_index')),
    f=onnx_file_path,
    opset_version=14,
    input_names=input_names,
    output_names=['output'],
    dynamic_axes=dynamic_axes
)

# Verify the ONNX model
onnx_model = onnx.load(onnx_file_path)
onnx.checker.check_model(onnx_model)
print(f"Model successfully converted to {onnx_file_path}")
