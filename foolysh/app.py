"""
Provides the App class to handle everything related to execution of an App.
"""

import ctypes
import time
from typing import Iterable
from typing import Optional
from typing import Tuple
from typing import Union

import sdl2
import sdl2.ext

from .tools import config
from . import animation
from . import eventhandler
from . import interval
from . import render
from .scene import node
from . import taskmanager
from . import tools
from .tools import vector2
from .tools import spriteloader
from .tools import clock

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

FRAME_TIME = 1 / 60

ISANDROID = False
try:
    # noinspection PyPackageRequirements
    import android
    ISANDROID = True
except ImportError:
    class Android(object):
        def remove_presplash(self):
            pass
    android = Android()


class AppClock(object):
    def __init__(self, clock):
        self._clock = clock

    def get_dt(self):
        return self._clock.get_dt()

    def get_time(self):
        return self._clock.get_time()

    def tick(self):
        raise RuntimeError('Calls to this method are blocked.')


class App(object):
    # noinspection PyUnresolvedReferences
    """
        Base class that handles everything necessary to run an App.

        :param window_title: Optional str -> Window Title

        Example Usage:

        >>> class MyApp(App):
        ...    def __init__(self):
        ...         super(MyApp, self).__init__('My App Title')
        ...
        >>> MyApp().run()  # Opens the App and runs, until the App is closed.
        """
    def __init__(self, window_title='foolysh engine', config_file=None):
        self._cfg = config.Config(config_file)
        if 'base' not in self._cfg:
            raise ValueError(
                f'Section "base" missing in loaded config_file '
                f'("{self._cfg.cfg_path}")'
            )
        self._taskmgr = taskmanager.TaskManager()
        self._event_handler = eventhandler.EventHandler()
        self._taskmgr.add_task('__EVENT_HANDLER__', self._event_handler)
        self._font_manager = None  # type: Union[sdl2.ext.FontManager, None]
        self._root = node.Node()
        self._renderer = None
        self._factory = None
        self._window = None
        self._window_title = window_title
        self._screen_size = (0, 0)
        self._running = False
        self._mouse_pos = vector2.Point2()
        self._animation_manager = animation.AnimationManager()
        self._taskmgr.add_task('__ANIMATION__', self._animation_manager.animate)
        self._frames = 0
        self._fps = 0.0
        self._clock = clock.Clock()
        self._app_clock = AppClock(self._clock)
        self._init_sdl()
        self.__clean_exit = False
        self._sprite_loader = spriteloader.SpriteLoader(
            self._factory,
            self._cfg['base']['asset_dir']
                if 'asset_dir' in self._cfg['base'] else '.',
            self._cfg['base']['cache_dir']
                if 'cache_dir' in self._cfg['base'] else None
        )
        self._renderer.root_node = self._root
        apr = self._cfg['base']['asset_pixel_ratio']
        self._renderer.asset_pixel_ratio = int(apr)
        self._renderer.sprite_loader = self._sprite_loader


    @property
    def isandroid(self):
        # type: () -> bool
        """``bool`` -> ``True`` if platform is android, otherwise ``False``."""
        return ISANDROID

    @property
    def renderer(self):
        # type: () -> sdl2.ext.TextureSpriteRenderSystem
        """``sdl2.ext.TextureSpriteRenderSystem``"""
        return self._renderer

    # noinspection PyUnusedLocal
    @property
    def event_handler(self, *args, **kwargs):
        # type: (...) -> eventhandler.EventHandler
        """``EventHandler``"""
        return self._event_handler

    @property
    def task_manager(self):
        # type: () -> taskmanager.TaskManager
        """``TaskManager``"""
        return self._taskmgr

    @property
    def window(self):
        # type: () -> sdl2.ext.Window
        """``sdl2.ext.Window``"""
        return self._window

    @property
    def mouse_pos(self):
        # type: () -> vector.Point
        """``Point`` -> current mouse position (=last touch location)"""
        return self._mouse_pos + vector2.Vector2()

    @property
    def screen_size(self):
        # type: () -> Tuple[int, int]
        """``Tuple[int, int]``"""
        return self._screen_size

    @property
    def clock(self):
        # type: () -> AppClock
        """:class:`foolysh.tools.clock.Clock` object of the running app."""
        return self._app_clock

    @property
    def root(self):
        # type: () -> node.Node
        """Root:class:`foolysh.scene.node.Node`."""
        return self._root

    def toast(self, message):
        # type: (str) -> None
        """
        If on android, shows ``message`` as a `toast` on screen.

        :param message: ``str`` -> the message to display.
        """
        if self.isandroid:
            tools.toast(message)

    def init_font_manager(
            self,
            font_path,                          # type: str
            alias=None,                         # type: Optional[str]
            size=16,                            # type: Optional[int]
            color=sdl2.ext.Color(),             # type: Optional[sdl2.ext.Color]
            bgcolor=sdl2.ext.Color(0, 0, 0, 0)  # type: Optional[sdl2.ext.Color]
    ):
        # type: (...) -> None
        """
        Initializes the ``sdl2.ext.FontManager``.

        :param font_path: ``str`` -> Path to the default font.
        :param alias: Optional ``str`` -> alias of the font.
        :param size: Optional ``int`` -> font size
        :param color: Optional ``sdl2.ext.Color`` -> foreground color
        :param bgcolor: Optional ``sdl2.ext.Color`` -> background color
        """
        if self._font_manager is not None:
            self._font_manager.close()
        self._font_manager = sdl2.ext.FontManager(
            font_path,
            alias,
            size,
            color,
            bgcolor
        )

    def add_font(self, font_path, alias=None, size=16):
        """
        Add a font to the ``sdl2.ext.FontManager``.

        :param font_path: ``str`` -> Path to the default font.
        :param alias: Optional ``str`` -> alias of the font.
        :param size: Optional ``int`` -> font size
        """
        if self._font_manager is None:
            raise ValueError('FontManager not initialized. Call '
                             'init_font_manager() method first')
        self._font_manager.add(font_path, alias, size)

    def text_sprite(
            self,
            text,               # type: str
            alias=None,         # type: Optional[str]
            size=None,          # type: Optional[int]
            width=None,         # type: Optional[int]
            color=None,         # type: Optional[sdl2.ext.Color]
            bg_color=None,      # type: Optional[sdl2.ext.Color]
            **kwargs
    ):
        # type: (...) -> sdl2.ext.TextureSprite
        """
        Load text as a Sprite.

        :param text: ``str`` -> the text to load.
        :param alias: Optional ``str`` -> the alias of the font to use.
        :param size: Optional ``int`` -> the font size.
        :param width: Optional ``int`` -> the width used for word wrap.
        :param color: Optional ``sdl2.ext.Color`` -> the foreground color
        :param bg_color: Optional ``sdl2.ext.Color`` -> the background color
        :param kwargs: additional keyword arguments, passed into
            ``sdl2.ext.FontManager.render()``
        :return: ``sdl2.ext.TextureSprite``
        """
        if self._font_manager is None:
            raise ValueError('FontManager not initialized. Call '
                             'init_font_manager() method first')
        surface = self._font_manager.render(
            text,
            alias,
            size,
            width,
            color,
            bg_color,
            **kwargs
        )
        sprite = self._factory.from_surface(surface)
        sdl2.SDL_FreeSurface(surface)
        return sprite

    def __update_mouse(self):
        # type: (...) -> None
        """Updates ``App.mouse_pos``."""
        if not self._running:
            return
        x, y = ctypes.c_int(0), ctypes.c_int(0)
        _ = sdl2.mouse.SDL_GetMouseState(ctypes.byref(x), ctypes.byref(y))
        f = 1 / min(self._window.size)
        self._mouse_pos.x, self._mouse_pos.y = x.value * f, y.value * f

    def run(self):
        """
        Run the main loop until :meth:`App.quit` gets called.

        .. warning::
            Make sure to call ``super().run()`` if you override this method
            at the end of your implementation.
        """
        self._clock.tick()
        try:
            self._running = True
            while self._running:
                self.__update_mouse()
                self.task_manager()
                self.renderer.render()
                self._clock.tick()
                st = max(0.0, FRAME_TIME - self._clock.get_dt())
                self._frames += 1
                if st:
                    time.sleep(st)
        except (KeyboardInterrupt, SystemExit):
            self.quit(blocking=False)
        finally:
            sdl2.ext.quit()
            self.__clean_exit = True

    # noinspection PyUnusedLocal
    def quit(self, blocking=True, event=None):
        # type: (Optional[bool], Optional[sdl2.SDL_Event]) -> None
        """
        Exit the main loop and quit the app.

        :param blocking: Optional ``bool`` -> whether the method should wait
            until the App has quit.
        :param event: Optional ``sdl2.SDL_Event`` -> Unused, used to enable
            being executed by an event callback.

        .. warning::
            Do not override this method, override :meth:`App.on_quit` instead!!!

        """
        if not self._running:
            return
        self.on_quit()
        self._running = False
        if blocking:
            while not self.__clean_exit:
                time.sleep(0.01)

    def on_quit(self):
        """
        Method to override to perform cleanup when :meth:`App.quit()` gets
        called.
        """
        pass

    def _init_sdl(self):
        """Initializes SDL2."""
        sdl2.ext.init()
        sdl2.SDL_SetHint(sdl2.SDL_HINT_RENDER_SCALE_QUALITY, b'1')
        if self.isandroid:
            dm = sdl2.SDL_DisplayMode()
            sdl2.SDL_GetCurrentDisplayMode(0, dm)
            self._screen_size = (dm.w, dm.h)
            sdl2.ext.Window.DEFAULTFLAGS = sdl2.SDL_WINDOW_FULLSCREEN
            self._window = sdl2.ext.Window(
                self._window_title,
                size=self._screen_size
            )
        else:
            if 'window_size' in self._cfg['base']:
                sx, sy = self._cfg['base']['window_size'].split('x')
                size = int(sx), int(sy)
            else:
                size = (720, 1280)
            self._screen_size = size
            self._window = sdl2.ext.Window(
                self._window_title,
                size=self._screen_size,
                flags=sdl2.SDL_WINDOW_RESIZABLE
            )
        self._window.show()
        android.remove_presplash()
        self._renderer = render.HWRenderer(self.window)
        self._factory = sdl2.ext.SpriteFactory(
            sdl2.ext.TEXTURE,
            renderer=self._renderer
        )

    def __del__(self):
        """Make sure, ``sdl2.ext.quit()`` gets called latest on destruction."""
        if not self.__clean_exit:
            sdl2.ext.quit()
