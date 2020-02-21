"""
Provides functions to generate signed distance fields and Pillow Image objects
of such.
"""

from typing import Optional
from typing import Tuple

from PIL import Image
import numpy as np

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
exist at the moment
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE."""


def box(
        width: int,
        height: int,
        corner_radius: Optional[int] = 0,
        thickness: Optional[int] = None
    ) -> np.ndarray:
    """
    Generate a SDF of a box.

    Args:
        width: ``int``.
        height: ``int``.
        corner_radius: ``Optional[int]`` -> radius of rounded corners.
        thickness: ``Optional[int]`` -> when only a frame is needed.

    Returns:
        ``np.ndarray`` with shape (`height`, `width`) of the box.
    """
    size = (np.array([height, width], dtype=np.float32) - 1) / 2
    size = size.reshape((2, 1, 1))
    frame = np.indices((height, width), dtype=np.float32)
    frame -= size
    frame = np.abs(frame)
    size -= corner_radius + (thickness or 0) / 2 + 0.5
    df = frame - size
    mx = np.minimum(0, np.max(df, 0))
    length = (np.sqrt(np.sum(np.maximum(0, df) ** 2, 0)) + mx) - corner_radius
    if thickness is not None and thickness != 0:
        length = np.abs(length) - thickness / 2
    length = np.minimum(1.5, length) / 1.5
    return 1 - (length > 0) * length


def framed_box_im(
        width: int,
        height: int,
        corner_radius: Optional[int] = 0,
        border_thickness: Optional[int] = None,
        frame_color: Optional[Tuple[int, int, int]] = (160, 160, 160),
        border_color: Optional[Tuple[int, int, int]] = (255, 255, 255),
        multi_sampling: Optional[int] = 1,
        alpha: Optional[int] = 255
    ) -> Image:
    """
    Generate a framed box image.

    Args:
        width: ``int`` -> width in pixel.
        height: ``int`` -> height in pixel.
        corner_radius: ``Optional[int]`` -> corner radius in pixel
            (default = 0).
        border_thickness: ``Optional[int]`` -> border_thickness in pixel
            (default = 1% of the higher value of `width` and `height`).
        frame_color: ``Optional[Tuple[int, int, int]]`` -> RGB color of the
            frame. Values in [0, 255].
        border_color: ``Optional[Tuple[int, int, int]]`` -> RGB color of the
            border. Values in [0, 255].
        multi_sampling: ``Optional[int]`` -> multiplier for the distance_field
            resolution.
        alpha: ``Optional[int]`` -> max alpha of the applied color.

    Returns:
        ``PIL.Image.Image`` of size (`width` x `height`) in mode ``RGBA``.
    """
    if multi_sampling < 1:
        raise ValueError('Expected positive, non zero value for '
                         'multi_sampling.')
    target_res = width, height
    sdf_res = width * multi_sampling, height * multi_sampling
    corner_radius *= multi_sampling
    if border_thickness == 0:
        frame = box(*sdf_res, corner_radius)
        im = sdf2image(frame, frame_color, alpha)
        if multi_sampling > 1:
            return im.resize(target_res, Image.BICUBIC)
        return im

    border_thickness = (border_thickness or (max(sdf_res) // 100))
    border_thickness *= multi_sampling
    half_thickness = max(border_thickness - 1, 1) * 2

    border = box(*sdf_res, corner_radius, border_thickness )
    frame = box(
        sdf_res[0] - half_thickness,
        sdf_res[1] - half_thickness,
        corner_radius - half_thickness
    )

    border_im = sdf2image(border, border_color, alpha)
    frame_im = sdf2image(frame, frame_color, alpha)

    im = Image.new('RGBA', sdf_res)
    im.paste(frame_im, (half_thickness // 2, half_thickness // 2), frame_im)
    im.alpha_composite(border_im)
    if multi_sampling > 1:
        return im.resize(target_res, Image.BICUBIC)
    return im


def circle(radius: int, thickness: Optional[int] = None):
    """
    Generate a SDF of a circle.

    Args:
        radius: ``int``.
        thickness: ``Optional[int]`` -> when only a frame is needed.

    Returns:
        ``np.ndarray`` with shape (`width`, `height`) of the circle.
    """
    frame = np.indices((radius * 2, radius * 2), dtype=np.float32)
    frame -= radius
    frame = np.abs(frame)
    half_t = (thickness or 0) / 2
    length = np.sqrt(np.sum(frame ** 2, 0)) - (radius - half_t - 0.5)
    if thickness is not None and thickness != 0:
        length = np.abs(length) - half_t
    length = np.minimum(1.5, length) / 1.5
    return 1 - (length > 0) * length


def framed_circle_im(
        radius: int,
        border_thickness: Optional[int] = None,
        frame_color: Optional[Tuple[int, int, int]] = (160, 160, 160),
        border_color: Optional[Tuple[int, int, int]] = (255, 255, 255),
        multi_sampling: Optional[int] = 1,
        alpha: Optional[int] = 255
    ) -> Image:
    """
    Generate a framed circle image.

    Args:
        radius: ``int``.
        border_thickness: ``Optional[int]`` -> border_thickness in pixel
            (default = 2% of the `radius`).
        frame_color: ``Optional[Tuple[int, int, int]]`` -> RGB color of the
            frame. Values in [0, 255].
        border_color: ``Optional[Tuple[int, int, int]]`` -> RGB color of the
            border. Values in [0, 255].
        multi_sampling: ``Optional[int]`` -> multiplier for the distance_field
            resolution.
        alpha: ``Optional[int]`` -> max alpha of the applied color.

    Returns:
        ``PIL.Image.Image`` of size (`width` x `height`) in mode ``RGBA``.
    """
    if multi_sampling < 1:
        raise ValueError('Expected positive, non zero value for '
                         'multi_sampling.')
    target_res = (radius * 2, ) * 2
    sdf_res = (radius * 2 * multi_sampling, ) * 2
    if border_thickness == 0:
        frame = circle(radius * multi_sampling)
        im = sdf2image(frame, frame_color, alpha)
        if multi_sampling > 1:
            return im.resize(target_res, Image.BICUBIC)
        return im

    border_thickness = (border_thickness or (radius // 50)) * multi_sampling
    half_t = max(border_thickness - 1, 1)
    radius = radius * multi_sampling
    border = circle(radius, border_thickness)
    frame = circle(radius - half_t)

    frame_im = sdf2image(frame, frame_color, alpha)
    border_im = sdf2image(border, border_color, alpha)

    im = Image.new('RGBA', sdf_res)
    im.paste(frame_im, (half_t, half_t), frame_im)
    im.alpha_composite(border_im)
    if multi_sampling > 1:
        return im.resize(target_res, Image.BICUBIC)
    return im


def sdf2image(
        df: np.ndarray,
        color: Optional[Tuple[int, int, int]] = (255, 255, 255),
        alpha: Optional[int]=255
    ) -> Image.Image:
    """
    Generate an image from a SDF.

    Args:
        df: ``np.ndarray`` -> SDF.
        color: ``Optional[Tuple[int, int, int]]`` -> RGB color.
        alpha: ``Optional[int]`` -> max alpha of the applied color.

    Returns:
        ``PIL.Image.Image`` of the SDF.
    """
    arr = np.zeros(df.shape + (4, ), dtype=np.float32)
    for i, c in enumerate(color):
        arr[:, :, i] = df * c
    arr[:, :, 3] = df * alpha
    return Image.fromarray(arr.astype(np.uint8), 'RGBA')
