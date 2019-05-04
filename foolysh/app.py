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

from . import eventhandler
from . import interval
from . import render
from .scene import nodepath
from . import taskmanager
from . import tools
from .tools import vector

__author__ = 'Tiziano Bettio'
__license__ = 'MIT'
__version__ = '0.1'
__copyright__ = """Copyright (c) 2019 Tiziano Bettio

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

SEQUENCE_TYPE = Iterable[Tuple[float, vector.Point, vector.Point]]
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

        Todo:
            * Replace PySDL2 Component System with SceneGraph
        """
    def __init__(self, window_title='foolysh engine'):
        self._taskmgr = taskmanager.TaskManager()
        self._event_handler = eventhandler.EventHandler()
        self._taskmgr.add_task('___EVENT_HANDLER___', self._event_handler)
        self._font_manager = None  # type: Union[sdl2.ext.FontManager, None]
        self._world = sdl2.ext.World()
        self._root = nodepath.NodePath('RootNodePath')
        self._renderer = None
        self._factory = None
        self._window = None
        self._window_title = window_title
        self._screen_size = (0, 0)
        self._running = False
        self._mouse_pos = vector.Point()
        self._taskmgr.add_task('___MOUSE_WATCHER___', self.__update_mouse__)
        self._sequences = {}
        self._anim_callbacks = {}
        self._taskmgr.add_task('___ANIMATION___', self.__animation__)
        self._frames = 0
        self._fps = 0.0
        self._init_sdl()
        self._clean_exit = False

    @property
    def isandroid(self):
        # type: () -> bool
        """``bool`` -> ``True`` if platform is android, otherwise ``False``."""
        return ISANDROID

    @property
    def world(self):
        # type: () -> sdl2.ext.World
        """``sdl2.ext.World``"""
        return self._world

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
        return self._mouse_pos.asint()

    @property
    def screen_size(self):
        # type: () -> Tuple[int, int]
        """``Tuple[int, int]``"""
        return self._screen_size

    def load_sprite(self, fpath):
        # type: (str) -> sdl2.ext.TextureSprite
        """
        Load a sprite from ``fpath``.
        :param fpath: ``str`` -> path of an image file.
        :return: ``sdl2.ext.TextureSprite``
        """
        return tools.load_sprite(self._factory, fpath)

    def entity_in_sequences(self, entity):
        # type: (sdl2.ext.Entity) -> bool
        """
        Returns ``True`` when ``entity`` is currently in a sequence.

        :param entity: ``sdl2.ext.Entity``
        :return: ``bool``
        """
        return True if str(entity) in self._sequences else False

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

    # noinspection PyUnusedLocal
    def __update_mouse__(self, *args, **kwargs):
        # type: (...) -> None
        """Updates ``App.mouse_pos``."""
        if not self._running:
            return
        x, y = ctypes.c_int(0), ctypes.c_int(0)
        _ = sdl2.mouse.SDL_GetMouseState(ctypes.byref(x), ctypes.byref(y))
        self._mouse_pos.x, self._mouse_pos.y = x.value, y.value

    # noinspection PyUnusedLocal
    def __animation__(self, dt, *args, **kwargs):
        # type: (float, ..., ...) -> None
        """Animation Task."""
        if not self._running or not self._sequences:
            return
        for k in self._sequences:
            sequence = self._sequences[k]
            rt = sequence[0].step(dt)
            if rt > 0:
                continue
            while rt <= 0:
                sequence.pop(0)
                if len(sequence):
                    rt = sequence[0].step(dt)
                else:
                    break
        p = []
        e = []
        for k in self._sequences:
            if not self._sequences[k]:
                p.append(k)
                if k in self._anim_callbacks:
                    e.append(k)
        for k in p:
            self._sequences.pop(k)
        for k in e:
            f, args, kwargs = self._anim_callbacks.pop(k)
            f(*args, **kwargs)

    def position_sequence(
            self,
            entity,         # type: sdl2.ext.Entity
            depth,          # type: int
            sequence,       # type: SEQUENCE_TYPE
            callback=None,  # type: Optional[callable]
            *args,
            **kwargs
    ):
        # type: (...) -> None
        """
        Add a sequence of PositionInterval for ``entity``.

        :param entity: ``sdl2.ext.Entity``
        :param depth: ``int`` -> depth during the sequence.
        :param sequence: ``Iterable[Tuple[float, Point, Point]]`` -> iterable of
            3-tuple containing (``duration``, ``start_pos``, ``end_pos``).
        :param callback: Optional ``callable`` -> callable to execute after the
            sequence is completed.
        :param args: Optional positional arguments to pass to ``callback``.
        :param kwargs: Optional keyword arguments to pass to ``callback``.
        """
        seq = []
        for duration, start_pos, end_pos in sequence:
            seq.append(interval.PositionInterval(
                entity,
                depth,
                duration,
                start_pos,
                end_pos
            ))
        k = str(entity)
        if k in self._sequences:
            if k in self._anim_callbacks:
                f, args, kwargs = self._anim_callbacks[k]
                f(*args, **kwargs)
        self._sequences[k] = seq
        if callback is not None:
            self._anim_callbacks[k] = (callback, args, kwargs)

    def stop_all_position_sequences(self):
        """Stops all position sequences and calls the respective callbacks."""
        for k in self._anim_callbacks:
            f, args, kwargs = self._anim_callbacks[k]
            f(*args, **kwargs)
        self._sequences = {}
        self._anim_callbacks = {}

    def run(self):
        """
        Run the main loop until ``App.quit()`` gets called.

        .. warning::
            Make sure to call ``super(YourClassName, self).run()`` if you
            override this method!!!
        """
        try:
            self._running = True
            st = time.perf_counter()
            while self._running:
                self.task_manager(st)
                self.world.process()
                self._frames += 1
                nt = time.perf_counter()
                time.sleep(max(0.0, FRAME_TIME - (nt - st)))
                st = nt
        except (KeyboardInterrupt, SystemExit):
            self.quit(blocking=False)
        finally:
            sdl2.ext.quit()
            self._clean_exit = True

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
            Do not override this method, override ``App.on_quit()`` instead!!!

        """
        if not self._running:
            return
        self.on_quit()
        self._running = False
        if blocking:
            while not self._clean_exit:
                time.sleep(0.01)

    def on_quit(self):
        """
        Method to override to perform cleanup when ``App.quit()`` gets called.
        """
        pass

    def _init_sdl(self):
        """Initializes SDL2."""
        sdl2.ext.init()
        if self.isandroid:
            dm = sdl2.SDL_DisplayMode()
            sdl2.SDL_GetCurrentDisplayMode(0, dm)
            self._screen_size = (dm.w, dm.h)
            sdl2.ext.Window.DEFAULTFLAGS = sdl2.SDL_WINDOW_FULLSCREEN
        else:
            self._screen_size = (720, 1280)
        self._window = sdl2.ext.Window(
            self._window_title,
            size=self._screen_size
        )
        self._window.show()
        android.remove_presplash()
        self._renderer = render.HWRenderer(self.window)
        self._factory = sdl2.ext.SpriteFactory(
            sdl2.ext.TEXTURE,
            renderer=self._renderer
        )
        self.world.add_system(self._renderer)

    def __del__(self):
        """Make sure, ``sdl2.ext.quit()`` gets called latest on destruction."""
        if not self._clean_exit:
            sdl2.ext.quit()
