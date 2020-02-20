"""
Provides the App class to handle everything related to execution of an App.
"""

import ctypes
import time
from typing import Optional
from typing import Tuple

import sdl2
import sdl2.ext

from .tools import config
from . import animation
from . import eventhandler
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
    import android
    ISANDROID = True
except ImportError:
    class Android:
        """Android dummy class."""
        # pylint: disable=too-few-public-methods
        def remove_presplash(self):
            """Dummy method."""
    android = Android()


class AppClock:
    """
    Dummy class that simulates a Clock object with the
    :meth:`~foolysh.tools.clock.Clock.tick` method disabled.
    """
    def __init__(self, clock_obj):
        self._clock = clock_obj

    def get_dt(self):
        """See: :meth:`~foolysh.tools.clock.Clock.get_dt`."""
        return self._clock.get_dt()

    def get_time(self):
        """See: :meth:`~foolysh.tools.clock.Clock.get_time`."""
        return self._clock.get_time()

    @staticmethod
    def tick():
        """Raises a RuntimeError if called."""
        raise RuntimeError('Calls to this method are blocked.')


class App:
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
    # pylint: disable=too-many-instance-attributes

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
        self._root = node.Node()
        self._renderer = None
        self._factory = None
        self._window = None
        window_title = self._cfg.get('base', 'window_title',
                                     fallback=window_title)
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
            self._cfg.get('base', 'asset_dir', fallback='assets/'),
            self._cfg.get('base', 'cache_dir', fallback=None)
        )
        self._renderer.root_node = self._root
        self._renderer.asset_pixel_ratio = self._cfg.getint('base',
                                                            'asset_pixel_ratio')
        self._renderer.sprite_loader = self._sprite_loader
        from . import dragdrop  # pylint: disable=import-outside-toplevel
        drag_threshold = self._cfg.getfloat('base', 'drag_threshold',
                                            fallback=0.025)
        drag_button = self._cfg.getint('base', 'drag_drop_button',
                                       fallback=sdl2.SDL_BUTTON_LEFT)
        self._drag_drop = dragdrop.DragDrop(self, drag_threshold, drag_button)

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

    @property
    def event_handler(self, *unused_args, **unused_kwargs):
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
        """Root :class:`foolysh.scene.node.Node`."""
        return self._root

    @property
    def drag_drop(self):
        # type: () -> dragdrop.DragDrop
        """:class:`~foolysh.dragdrop.DragDrop` instance."""
        return self._drag_drop

    @property
    def config(self):
        # type: () -> config.Config
        """:class:`~foolysh.tools.config.Config` instance."""
        return self._cfg

    def toast(self, message):
        # type: (str) -> None
        """
        If on android, shows ``message`` as a `toast` on screen.

        :param message: ``str`` -> the message to display.
        """
        if self.isandroid:
            tools.toast(message)

    def __update_mouse(self):
        # type: (...) -> None
        """Updates ``App.mouse_pos``."""
        if not self._running:
            return
        x, y = ctypes.c_int(0), ctypes.c_int(0)
        _ = sdl2.mouse.SDL_GetMouseState(ctypes.byref(x), ctypes.byref(y))
        world_unit = 1 / min(self._window.size)
        self._mouse_pos.x = x.value * world_unit
        self._mouse_pos.y = y.value * world_unit

    def run(self):
        """
        Run the main loop until :meth:`App.quit` gets called.

        .. warning::
            Make sure to call ``super().run()`` if you override this method
            at the end of your implementation.
        """
        self._clock.tick()
        try:
            frame_clock = clock.Clock()
            self._running = True
            while self._running:
                frame_clock.tick()
                self.__update_mouse()
                self.task_manager()
                self.renderer.render()
                frame_clock.tick()
                sleep_time = max(0.0, FRAME_TIME - frame_clock.get_dt())
                if sleep_time:
                    time.sleep(sleep_time)
                self._frames += 1
                self._clock.tick()
        except (KeyboardInterrupt, SystemExit):
            self.quit(blocking=False)
        finally:
            sdl2.ext.quit()
            self.__clean_exit = True

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
        # pylint: disable=unused-argument
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

    def _init_sdl(self):
        """Initializes SDL2."""
        sdl2.ext.init()
        sdl2.SDL_SetHint(sdl2.SDL_HINT_RENDER_SCALE_QUALITY, b'1')
        if self.isandroid:
            display_mode = sdl2.SDL_DisplayMode()
            sdl2.SDL_GetCurrentDisplayMode(0, display_mode)
            self._screen_size = (display_mode.w, display_mode.h)
            sdl2.ext.Window.DEFAULTFLAGS = sdl2.SDL_WINDOW_FULLSCREEN
            self._window = sdl2.ext.Window(
                self._window_title,
                size=self._screen_size
            )
        else:
            size_x, size_y = self._cfg.get('base', 'window_size', fallback='720x1280')
            self._screen_size = int(size_x), int(size_y)
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
