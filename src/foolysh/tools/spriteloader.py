"""
Provides the SpriteLoader class, that handles loading and caching of assets.
"""
import pathlib
import hashlib
import os
from typing import Optional
from typing import Tuple

from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
from PIL import UnidentifiedImageError
from sdl2.ext import SpriteFactory
from sdl2.ext import TextureSprite
from sdl2 import endian
from sdl2 import surface
from sdl2 import pixels
from sdl2.ext import SDLError

from . import vector2

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

COLOR = Tuple[int, int, int, int]


# This function is adapted directly from the PySDL2 package, to perform the
# conversion of a PIL/Pillow image into a SDL2 Sprite.
def _image2sprite(image, factory):
    # pylint: disable=missing-docstring,bad-continuation,protected-access
    # pylint: disable=too-many-locals,too-many-branches,too-many-statements
    mode = image.mode
    width, height = image.size
    rmask = gmask = bmask = amask = 0
    if mode in ("1", "L", "P"):
        # 1 = B/W, 1 bit per byte
        # "L" = greyscale, 8-bit
        # "P" = palette-based, 8-bit
        pitch = width
        depth = 8
    elif mode == "RGB":
        # 3x8-bit, 24bpp
        if endian.SDL_BYTEORDER == endian.SDL_LIL_ENDIAN:
            rmask = 0x0000FF
            gmask = 0x00FF00
            bmask = 0xFF0000
        else:
            rmask = 0xFF0000
            gmask = 0x00FF00
            bmask = 0x0000FF
        depth = 24
        pitch = width * 3
    elif mode in ("RGBA", "RGBX"):
        # RGBX: 4x8-bit, no alpha
        # RGBA: 4x8-bit, alpha
        if endian.SDL_BYTEORDER == endian.SDL_LIL_ENDIAN:
            rmask = 0x000000FF
            gmask = 0x0000FF00
            bmask = 0x00FF0000
            if mode == "RGBA":
                amask = 0xFF000000
        else:
            rmask = 0xFF000000
            gmask = 0x00FF0000
            bmask = 0x0000FF00
            if mode == "RGBA":
                amask = 0x000000FF
        depth = 32
        pitch = width * 4
    else:
        # We do not support CMYK or YCbCr for now
        raise TypeError("unsupported image format")

    pxbuf = image.tobytes()
    imgsurface = surface.SDL_CreateRGBSurfaceFrom(pxbuf, width, height,
                                                    depth, pitch, rmask,
                                                    gmask, bmask, amask)
    if not imgsurface:
        raise SDLError()
    imgsurface = imgsurface.contents
    # the pixel buffer must not be freed for the lifetime of the surface
    imgsurface._pxbuf = pxbuf

    if mode == "P":
        # Create a SDL_Palette for the SDL_Surface
        def _chunk(seq, size):
            for x in range(0, len(seq), size):
                yield seq[x:x + size]

        rgbcolors = image.getpalette()
        sdlpalette = pixels.SDL_AllocPalette(len(rgbcolors) // 3)
        if not sdlpalette:
            raise SDLError()
        SDL_Color = pixels.SDL_Color
        # pylint: disable=invalid-name
        for idx, (r, g, b) in enumerate(_chunk(rgbcolors, 3)):
            sdlpalette.contents.colors[idx] = SDL_Color(r, g, b)
        ret = surface.SDL_SetSurfacePalette(imgsurface, sdlpalette)
        # This will decrease the refcount on the palette, so it gets
        # freed properly on releasing the SDL_Surface.
        pixels.SDL_FreePalette(sdlpalette)
        if ret != 0:
            raise SDLError()

    return factory.from_surface(imgsurface, free=True)


class SpriteLoader:
    """
    Provides ``load_*`` methods that return Sprite objects of (cached) images
    and a method to compose/flatten multiple images into a single one.
    Automatically caches scaled and/or composed images on first load to reduce
    subsequent load time.

    Requires a SpriteFactory, a valid path to the asset directory and optionally
    the cache directory can be specified, otherwise a 'cache' directory,
    relative to ``os.getcwd()`` will be used. The cache_dir will be created if
    absent on init.
    """
    def __init__(
            self,
            factory,                    # type: SpriteFactory
            asset_dir,                  # type: str
            cache_dir=None,             # type: Optional[str]
            resize_type=Image.BICUBIC   # type: Optional[int]
    ):
        # type: (...) -> None
        if not isinstance(factory, SpriteFactory):
            raise TypeError('expected sdl2.ext.SpriteFactory for factory')
        if not os.path.isdir(asset_dir):
            raise NotADirectoryError(f'Invalid asset_dir')
        self.factory = factory
        self.asset_dir = asset_dir
        self.cache_dir = cache_dir or os.path.join(os.getcwd(), 'cache')
        if not os.path.isdir(self.cache_dir):
            os.makedirs(self.cache_dir)
        self.resize_type = resize_type
        self._assets = {}
        self._sprite_cache = {}
        self._font_cache = {}
        self._refresh_assets()

    def _refresh_assets(self):
        paths = pathlib.Path(self.asset_dir).glob('**/*.*')
        paths = [str(f) for f in paths]
        paths = [
            s[s.find(self.asset_dir) + len(self.asset_dir):] for s in paths
        ]
        if paths and paths[0].startswith('/'):
            paths = [s[1:] for s in paths]
        self._assets = {}
        for k in paths:
            if k[-3:].lower() in ('ttf', 'otf'):
                self._assets[k] = os.path.join(self.asset_dir, k)
                continue
            try:
                _ = Image.open(os.path.join(self.asset_dir, k))
            except UnidentifiedImageError:
                continue
            except IsADirectoryError:
                continue
            self._assets[k] = Asset(k, self)

    def load_image(self, asset_path, scale=1.0):
        # type: (str, Optional[SCALE]) -> TextureSprite
        if asset_path in self._assets:
            k = self._assets[asset_path][scale]
            if k not in self._sprite_cache:
                self._sprite_cache[k] = _image2sprite(
                    Image.open(k),
                    self.factory
                )
            return self._sprite_cache[k]
        raise ValueError(f'asset_path must be a valid path relative to '
                         f'"{self.asset_dir}" without leading "/". Got '
                         f'"{asset_path}".')

    def load_text(self, text, font, size, color, align, spacing, multiline):
        # type: (str, str, int, COLOR, str, int, bool) -> TextureSprite
        """Load """
        # pylint: disable=too-many-arguments
        if font in self._assets:
            k = f'{text}{font}{size}{color}{align}{spacing}{multiline}'
            if k not in self._sprite_cache:
                font_k = (font, size)
                if font_k not in self._font_cache:
                    self._font_cache[font_k] = ImageFont.truetype(
                        os.path.join(self.asset_dir, font),
                        size
                    )
                img = Image.new(
                    'RGBA',
                    self._font_cache[font_k].getsize_multiline(
                        text,
                        spacing=spacing
                    ) if multiline else self._font_cache[font_k].getsize(text)
                )
                draw = ImageDraw.Draw(img)
                func = draw.multiline_text if multiline else draw.text
                func(
                    (0, 0),
                    text,
                    color,
                    self._font_cache[font_k],
                    spacing=spacing,
                    align=align
                )
                self._sprite_cache[k] = _image2sprite(img, self.factory)
            return self._sprite_cache[k]
        raise ValueError(f'font must be a valid path relative to '
                         f'"{self.asset_dir}" without leading "/". Got '
                         f'"{font}".')

    def valid_asset(self, asset_path):
        """Verify that a given asset path is valid."""
        return asset_path in self._assets

    def empty_cache(self):
        """Delete all cached files."""
        for asset in self._assets.values():
            asset.empty_cache()

class Asset:
    """
    Represents a single Image asset path. Provides caching of scaled images.
    Used by SpriteLoader.
    """
    # pylint: disable=too-many-instance-attributes

    def __init__(self, relative_path, parent):
        # type: (str, SpriteLoader) -> None
        self.relative_path = relative_path
        cache_name = hashlib.sha3_224(relative_path.encode()).hexdigest()
        self.cache_sub_dir = os.path.join(parent.cache_dir, cache_name[:2])
        self.cache_prefix = cache_name[2:]
        self.cache_suffix = '.' + relative_path.split('.')[-1]
        self.abs_path = os.path.join(parent.asset_dir, relative_path)
        self._img_size = vector2.Point2(Image.open(self.abs_path).size)
        self._cached_items = {}
        self.parent = parent
        self._refresh_cached()

    def _refresh_cached(self):
        self._cached_items = {}
        files = pathlib.Path(self.cache_sub_dir).glob(f'{self.cache_prefix}*')
        files = [str(f) for f in files]
        for file in files:
            res = file.split('.')[-2][-10:]
            res = int(res[:5]), int(res[5:])
            self._cached_items[res] = file

    @property
    def size(self):
        """Original image size."""
        return self._img_size

    def __getitem__(self, item):
        # type: (SCALE) -> str
        if isinstance(item, float):
            k = (int(self.size.x * item), int(self.size.y * item))
        elif isinstance(item, tuple) and len(item) == 2 and \
                isinstance(item[0], float) and isinstance(item[1], float):
            k = (int(self.size.x * item[0]), int(self.size.y * item[1]))
        else:
            raise TypeError('expected type Union[float, Tuple[float, float]]')
        if k not in self._cached_items:
            self._cache(k)
        return self._cached_items[k]

    def _cache(self, k):
        if not os.path.isdir(self.cache_sub_dir):
            os.makedirs(self.cache_sub_dir)
        fname = f'{self.cache_prefix}{k[0]:05d}{k[1]:05d}{self.cache_suffix}'
        pth = os.path.join(self.cache_sub_dir, fname)
        Image.open(self.abs_path).resize(k, self.parent.resize_type).save(pth)
        self._cached_items[k] = pth

    def empty_cache(self):
        """Delete all cached files from disk."""
        for pth in self._cached_items.values():
            os.remove(pth)
        try:
            os.rmdir(self.cache_sub_dir)
        except OSError as err:
            if err.errno != 39:
                raise err
        self._cached_items = {}
