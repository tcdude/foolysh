"""
Provides general tools.
"""

import sdl2.ext
import plyer

# from .vector import Point
# from .vector import Vector

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

# Position Constants
CENTER = 0
TOP_LEFT = 1
BOTTOM_RIGHT = 2


def load_sprite(factory, fpath):
    """Dirty hack -> Update when p4a's pysdl2 recipe is updated."""
    surface = sdl2.ext.load_image(fpath, 'SDL')
    sprite = factory.from_surface(surface)
    sdl2.SDL_FreeSurface(surface)
    return sprite


def nop():
    pass


def toast(message):
    plyer.notification.notify(message=message, toast=True)
