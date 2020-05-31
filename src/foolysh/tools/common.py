"""
Common constants and functions.
"""

from enum import Enum
import re
from typing import Tuple
from typing import Union

__author__ = 'Tiziano Bettio'
__license__ = 'MIT'
__version__ = '0.1'
__copyright__ = """Copyright (c) 2020 Tiziano Bettio

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE."""

# Default configuration

DEFAULT_CONFIG = {
    'base': {
        'asset_pixel_ratio': 1080,
        'logical_screen_size_multiplier': 1,
        'window_size': '720x1280',
        'cache_dir': 'cache'
    }
}


# Enums

class Origin(Enum):
    """
    Enum: Origin of a :class:`foolysh.scene.node.Node`.
    """
    TOP_LEFT = 0
    TOP_RIGHT = 1
    TOP_CENTER = 2
    CENTER_LEFT = 3
    CENTER = 4
    CENTER_RIGHT = 5
    BOTTOM_LEFT = 6
    BOTTOM_CENTER = 7
    BOTTOM_RIGHT = 8

# Custom Types & Enumerations

COLOR = Union[Tuple[int, int, int], Tuple[int, int, int, int]]
SCALE = Union[float, Tuple[float, float]]


# Helper functions

def valid_color(color: COLOR) -> COLOR:
    """
    Check if a valid color was passed and if only 3 component value was passed,
    appends the alpha component with value `255`.

    Args:
        color: ``3/4 value tuple`` -> the color to verify.


    Returns:
        ``4 value tuple`` -> a valid color tuple.
    """
    if not isinstance(color, tuple):
        raise TypeError('Expected tuple.')
    if sum([not isinstance(i, int) for i in color]):
        raise TypeError('Expected only int.')
    if sum([not -1 < i < 256 for i in color]):
        raise ValueError('Expected values to lie in (0..255).')
    if not 2 < len(color) < 5:
        raise ValueError('Expected 3- or 4-value tuple.')
    return color if len(color) == 4 else color + (255, )


def to_snake_case(expr: str) -> str:
    """
    Converts a PascalCase string into a snake_case one.

    Args:
        expr: ``str`` -> the string to convert.

    Returns:
        ``str`` -> the string formatted as snake_case.
    """
    regex = r'(.+?)([A-Z])'

    def snake(match):
        return match.group(1).lower() + '_' + match.group(2).lower()

    return re.sub(regex, snake, expr, 0).lower()
