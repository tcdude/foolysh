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
from sdl2.ext import SpriteFactory
from sdl2.ext import TextureSprite
from sdl2 import endian
from sdl2 import surface
from sdl2 import pixels
from sdl2.ext import SDLError

from . import sdf
from . import vec2

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


def parse_sdf_str(sdf_str):
    """
    Parses SDF strings to get SDF type and keyword arguments for the function
    call.


    Args:
        sdf_str: ``str`` -> SDF strings are formatted as follows:
            "SDF:[sdf_type]:[I/F]:[parameter]=[value]:..." where value will be
            converted to either I=int or F=float, where appropriate. Colors are
            the exception, where always int is presumed in form of a tuple.
            .. note::
                SDF string Example:

                ``SDF:circle:I:radius=100:frame_color=(80,120,10):alpha=180``
                This would produce a circle with a radius of 100 pixel filled
                with the specified frame_color and alpha values.

    Returns:
        ``Tuple[Dict, str]`` -> kwargs as dictionary and the SDF type as string.
    """
    elements = sdf_str.split(':')
    try:
        isfloat = {'I': False, 'F': True}[elements[2]]
    except KeyError:
        raise ValueError(f'Expected either "I" or "F" for data type, got '
                         f'"{elements[2]}" instead.')
    converter = {
        'width': (float, int),
        'height': (float, int),
        'radius': (float, int),
        'corner_radius': (float, int),
        'border_thickness': (float, int),
        'frame_color': (
            lambda x: tuple([int(i) for i in x.strip()[1:-1].split(',')]),
        ),
        'border_color': (
            lambda x: tuple([int(i) for i in x.strip()[1:-1].split(',')]),
        ),
        'multi_sampling': (int, ),
        'alpha': (int, )
    }  # Insures that

    kwargs = {}
    w_unit = 0
    for i in elements[3:]:
        try:
            k, value = i.split('=')
        except ValueError:
            raise ValueError(f'Expected "parameter=value", got "{i}" instead.')
        if k in converter:
            try:
                kwargs[k] = converter[k][0 if isfloat else -1](value)
            except (TypeError, ValueError):
                raise ValueError(f'Unable to unpack parameter: "{i}"')
        elif isfloat and k == 'w':
            w_unit = int(value)
        else:
            raise ValueError(f'Unknown parameter: "{k}".')

    if isfloat:
        for k in kwargs:
            if converter[k][0] is float:
                kwargs[k] = int(kwargs[k] * w_unit + 0.5)

    return kwargs, elements[1]


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
            except IsADirectoryError:
                continue
            except IOError:
                continue
            self._assets[k] = Asset(k, self)

    def load_image(self, asset_path, scale=1.0):
        # type: (str, Optional[SCALE]) -> TextureSprite
        """
        Loads asset_path at specified scale or generates (if necessary) a SDF
        if `asset_path` starts with "SDF:" (SDF strings are case sensitive!).
        """
        if asset_path.startswith('SDF:'):
            return self._load_sdf(asset_path)
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
                fnt = self._font_cache[font_k]
                if multiline:
                    im_sz = fnt.getsize_multiline(text, spacing=spacing)
                    pos = 0, 0
                else:
                    pos = fnt.getoffset(text)
                    pos = -pos[0], -pos[1]
                    im_sz = fnt.getsize(text)
                    im_sz = im_sz[0] + pos[0], im_sz[1] + pos[1]
                img = Image.new('RGBA', im_sz)
                draw = ImageDraw.Draw(img)
                func = draw.multiline_text if multiline else draw.text
                func(pos, text=text, fill=color, font=fnt, spacing=spacing,
                     align=align)
                try:
                    img.tobytes()
                except SystemError:
                    print(f'went wrong with {k}')
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

    def _load_sdf(self, sdf_str):
        """
        Adds the appropriate SDF to the sprite cache.

        """
        kwargs, sdf_t = parse_sdf_str(sdf_str)
        arg_str = ''.join([f'{k}={kwargs[k]}' for k in sorted(kwargs)])
        cache_name = hashlib.sha3_224(arg_str.encode()).hexdigest()
        path = os.path.join(
            self.cache_dir,
            f'SDF/{sdf_t}/{cache_name[:2]}/{cache_name[2:]}.png'
        )
        if path not in self._sprite_cache:
            if not os.path.isfile(path):
                cache_dir = os.path.split(path)[0]
                if not os.path.isdir(cache_dir):
                    os.makedirs(cache_dir)
                if sdf_t == 'box':
                    image = sdf.framed_box_im(**kwargs)
                elif sdf_t == 'circle':
                    image = sdf.framed_circle_im(**kwargs)
                else:
                    raise ValueError(f'Unknown SDF type: "{sdf_t}"')
                image.save(path)
                self._sprite_cache[path] = _image2sprite(image, self.factory)
            else:
                self._sprite_cache[path] = _image2sprite(
                    Image.open(path),
                    self.factory
                )
        return self._sprite_cache[path]


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
        self._img_size = vec2.Point2(Image.open(self.abs_path).size)
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
