from typing import Dict, List, Tuple

import torch
from torch.nn.utils.rnn import pad_sequence

from dp import Prediction
from dp.model.model import load_checkpoint
from dp.model.utils import _get_len_util_stop
from dp.preprocessing.text import Preprocessor
from dp.preprocessing.utils import _batchify, _product


class Predictor:

    """ Performs model predictions on a batch of inputs. """

    def __init__(self,
                 model: torch.nn.Module,
                 preprocessor: Preprocessor) -> None:
        """
        Initializes a Predictor object with a trained transformer model a preprocessor.

        Args:
            model (Model): Trained transformer model.
            preprocessor (Preprocessor): Preprocessor corresponding to the model configuration.
        """

        self.model = model
        self.text_tokenizer = preprocessor.text_tokenizer
        self.phoneme_tokenizer = preprocessor.phoneme_tokenizer

    def __call__(self,
                 words: List[str],
                 lang: str,
                 batch_size: int = 8) -> List[Prediction]:
        """
        Predicts phonemes for a list of words.

        Args:
          words (list): List of words to predict.
          lang (str): Language of texts.
          batch_size (int): Size of batch for model input to speed up inference.

        Returns:
          List[Prediction]: A list of result objects containing (word, phonemes, phoneme_tokens, token_probs, confidence)
        """

        predictions = dict()
        valid_texts = set()

        # handle words that result in an empty input to the model
        for word in words:
            input = self.text_tokenizer(sentence=word, language=lang)
            decoded = self.text_tokenizer.decode(
                sequence=input, remove_special_tokens=True)
            if len(decoded) == 0:
                predictions[word] = ([], [])
            else:
                valid_texts.add(word)

        valid_texts = sorted(list(valid_texts), key=lambda x: len(x))
        batch_pred = self._predict_batch(texts=valid_texts, batch_size=batch_size,
                                         language=lang)
        predictions.update(batch_pred)

        output = []
        for word in words:
            tokens, probs = predictions[word]
            out_phons = self.phoneme_tokenizer.decode(
                sequence=tokens, remove_special_tokens=True)
            out_phons_tokens = self.phoneme_tokenizer.decode(
                sequence=tokens, remove_special_tokens=False)
            output.append(Prediction(word=word,
                                     phonemes=''.join(out_phons),
                                     phoneme_tokens=out_phons_tokens,
                                     confidence=_product(probs),
                                     token_probs=probs))

        return output

    def _predict_batch(self,
                       texts: List[str],
                       batch_size: int,
                       language: str) \
            -> Dict[str, Tuple[List[int], List[float]]]:
        """
        Returns dictionary with key = word and val = Tuple of (phoneme tokens, phoneme probs)
        """

        predictions = dict()
        text_batches = _batchify(texts, batch_size)
        for text_batch in text_batches:
            input_batch, lens_batch = [], []
            for text in text_batch:
                input = self.text_tokenizer(text, language)
                input_batch.append(torch.tensor(input))
                lens_batch.append(torch.tensor(len(input)))

            input_batch = pad_sequence(sequences=input_batch,
                                       batch_first=True, padding_value=0)
            lens_batch = torch.stack(lens_batch)
            start_indx = self.phoneme_tokenizer._get_start_index(language)
            start_inds = torch.tensor([start_indx]*input_batch.size(0)).to(input_batch.device)
            batch = {
                'text': input_batch,
                'text_len': lens_batch,
                'start_index': start_inds
            }
            with torch.no_grad():
                output_batch, probs_batch = self.model.generate(batch)
            output_batch, probs_batch = output_batch.cpu(), probs_batch.cpu()
            for text, output, probs in zip(text_batch, output_batch, probs_batch):
                seq_len = _get_len_util_stop(output, self.phoneme_tokenizer.end_index)
                predictions[text] = (output[:seq_len].tolist(), probs[:seq_len].tolist())

        return predictions

    @classmethod
    def from_checkpoint(cls, checkpoint_path: str, device='cpu') -> 'Predictor':
        """Initializes the predictor from a checkpoint (.pt file).

        Args:
          checkpoint_path (str): Path to the checkpoint file (.pt).
          device (str): Device to load the model on ('cpu' or 'cuda'). (Default value = 'cpu').

        Returns:
          Predictor: Predictor object.

        """
        model, checkpoint = load_checkpoint(checkpoint_path, device=device)
        preprocessor = checkpoint['preprocessor']
        return Predictor(model=model, preprocessor=preprocessor)

