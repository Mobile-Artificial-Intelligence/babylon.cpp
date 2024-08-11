import ctypes
import os

# Load the shared library
if os.name == 'nt':  # Windows
    babylon_lib = ctypes.CDLL('libbabylon.dll')
elif os.name == 'posix':  # macOS
    babylon_lib = ctypes.CDLL('./libbabylon.dylib')
else:  # Linux/Unix
    babylon_lib = ctypes.CDLL('./babylon.so')

# Define the function prototypes
babylon_lib.babylon_g2p_init.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
babylon_lib.babylon_g2p_init.restype = ctypes.c_int

babylon_lib.babylon_g2p.argtypes = [ctypes.c_char_p]
babylon_lib.babylon_g2p.restype = ctypes.c_char_p

babylon_lib.babylon_g2p_free.argtypes = []
babylon_lib.babylon_g2p_free.restype = None

babylon_lib.babylon_tts_init.argtypes = [ctypes.c_char_p]
babylon_lib.babylon_tts_init.restype = ctypes.c_int

babylon_lib.babylon_tts.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
babylon_lib.babylon_tts.restype = None

babylon_lib.babylon_tts_free.argtypes = []
babylon_lib.babylon_tts_free.restype = None

# Initialize G2P
def init_g2p(model_path, language, use_punctuation):
    return babylon_lib.babylon_g2p_init(model_path.encode('utf-8'), language.encode('utf-8'), use_punctuation)

# Use G2P
def g2p(text):
    result = babylon_lib.babylon_g2p(text.encode('utf-8'))
    return result.decode('utf-8')

# Free G2P resources
def free_g2p():
    babylon_lib.babylon_g2p_free()

# Initialize TTS
def init_tts(model_path):
    return babylon_lib.babylon_tts_init(model_path.encode('utf-8'))

# Use TTS
def tts(text, output_path):
    babylon_lib.babylon_tts(text.encode('utf-8'), output_path.encode('utf-8'))

# Free TTS resources
def free_tts():
    babylon_lib.babylon_tts_free()

# Example usage
if __name__ == '__main__':
    g2p_model_path = './models/deep_phonemizer.onnx'
    tts_model_path = './models/curie.onnx'
    language = 'en_us'
    use_punctuation = 1
    
    if init_g2p(g2p_model_path, language, use_punctuation) == 0:
        print('G2P initialized successfully')
        phonemes = g2p('Hello world')
        print(f'Phonemes: {phonemes}')
        free_g2p()
    else:
        print('Failed to initialize G2P')

    if init_tts(tts_model_path) == 0:
        print('TTS initialized successfully')
        tts('Hello world', 'output.wav')
        free_tts()
    else:
        print('Failed to initialize TTS')
