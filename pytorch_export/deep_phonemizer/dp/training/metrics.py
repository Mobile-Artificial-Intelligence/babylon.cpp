import numpy
from typing import List, Union, Tuple


def word_error(predicted: List[Union[str, int]], target: List[Union[str, int]]) -> float:
    """Calculates the word error rate of a single word result.

    Args:
      predicted: Predicted word.
      target: Target word.
      predicted: List[Union[str: 
      int]]: 
      target: List[Union[str: 

    Returns:
      Word error

    """

    return int(predicted != target)


def phoneme_error(predicted: List[Union[str, int]], target: List[Union[str, int]]) -> Tuple[int, int]:
    """Calculates the phoneme error rate of a single result based on the Levenshtein distance.

    Args:
      predicted: Predicted word.
      target: Target word.
      predicted: List[Union[str: 
      int]]: 
      target: List[Union[str: 

    Returns:
      Phoneme error.

    """

    d = numpy.zeros((len(target) + 1) * (len(predicted) + 1),
                    dtype=numpy.uint8)
    d = d.reshape((len(target) + 1, len(predicted) + 1))
    for i in range(len(target) + 1):
        for j in range(len(predicted) + 1):
            if i == 0:
                d[0][j] = j
            elif j == 0:
                d[i][0] = i

    for i in range(1, len(target) + 1):
        for j in range(1, len(predicted) + 1):
            if target[i - 1] == predicted[j - 1]:
                d[i][j] = d[i - 1][j - 1]
            else:
                substitution = d[i - 1][j - 1] + 1
                insertion = d[i][j - 1] + 1
                deletion = d[i - 1][j] + 1
                d[i][j] = min(substitution, insertion, deletion)

    return d[len(target)][len(predicted)], len(target)

