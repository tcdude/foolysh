"""
Provides the App class to handle everything related to execution of an App.
"""

import ctypes
from dataclasses import dataclass
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
from .tools import vec2
from .tools import spriteloader
from .tools import clock
from .ui import uihandler
from .ui import uinode

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


@dataclass
class Systems:
    """Class for keeping track of the various handler/manager objects."""
    # pylint: disable=too-many-instance-attributes
    task_manager: taskmanager.TaskManager
    event_handler: eventhandler.EventHandler
    ui_handler: uihandler.UIHandler
    animation_manager: animation.AnimationManager
    sprite_loader: spriteloader.SpriteLoader = None
    window: sdl2.ext.Window = None
    renderer: render.HWRenderer = None
    factory: sdl2.ext.SpriteFactory = None


@dataclass
class AppStats:
    """Class for keeping runtime stats of the App."""
    # pylint: disable=too-many-instance-attributes
    clock: clock.Clock
    window_title: str
    mouse_pos: vec2.Point2 = vec2.Point2()
    mouse_down: Optional[vec2.Point2] = None
    mouse_up: Optional[vec2.Point2] = None
    enter_down: bool = False
    enter: bool = False
    backspace_down: bool = False
    backspace: bool = False
    frames: int = 0
    fps: float = 0.0
    running: bool = False
    clean_exit: bool = True
    resolution_change: bool = True


@dataclass
class UIAnchors:
    """Class representing all UI anchor Node instances."""
    # pylint: disable=too-many-instance-attributes
    root: node.Node
    top_center: node.Node
    top_right: node.Node
    center_left: node.Node
    center: node.Node
    center_right: node.Node
    bottom_left: node.Node
    bottom_center: node.Node
    bottom_right: node.Node

    @property
    def top_left(self):
        """Alias for root."""
        return self.root


@dataclass
class AppNodes:
    """Class for keeping special Node instances."""
    # pylint: disable=invalid-name
    root: node.Node
    ui: UIAnchors


def _node_setup() -> AppNodes:
    root = node.Node('Root Node')
    uiroot = node.Node('UI Root Node / Top Left')
    node_list = [
        ('UI Top Center', node.Origin.TOP_CENTER),
        ('UI Top Right', node.Origin.TOP_RIGHT),
        ('UI Center Left', node.Origin.CENTER_LEFT),
        ('UI Center', node.Origin.CENTER),
        ('UI Center Right', node.Origin.CENTER_RIGHT),
        ('UI Bottom Left', node.Origin.BOTTOM_LEFT),
        ('UI Bottom Center', node.Origin.BOTTOM_CENTER),
        ('UI Bottom Right', node.Origin.BOTTOM_RIGHT),
    ]
    uinodes = []
    for name, origin in node_list:
        uind = uiroot.attach_node(name)
        uind.origin = origin
        uind.depth = 0
        uinodes.append(uind)
    return AppNodes(root, UIAnchors(uiroot, *uinodes))


def _update_android():
    display_mode = sdl2.SDL_DisplayMode()
    sdl2.SDL_GetCurrentDisplayMode(0, display_mode)
    screen_size = (display_mode.w, display_mode.h)
    ddpi = ctypes.c_float()
    sdl2.SDL_GetDisplayDPI(0, None, None, ctypes.byref(ddpi))
    physical_size = screen_size[0] ** 2 + screen_size[1] ** 2
    physical_size /= ddpi.value ** 2
    if physical_size < 49:
        global ISANDROID  # pylint: disable=global-statement
        ISANDROID = True


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

    def __init__(self, window_title=None, config_file=None):
        self.__cfg = config.Config(config_file)
        if 'base' not in self.__cfg:
            raise ValueError(
                f'Section "base" missing in loaded config_file '
                f'("{self.__cfg.cfg_path}")'
            )
        self.__nodes = _node_setup()
        event_h = eventhandler.EventHandler()
        self.__systems = Systems(
            task_manager=taskmanager.TaskManager(),
            event_handler=event_h,
            ui_handler=uihandler.UIHandler(self.__nodes.ui.root, event_h),
            animation_manager=animation.AnimationManager()
        )
        uinode.UIHANDLER_INSTANCE = self.__systems.ui_handler
        window_title = window_title or self.__cfg.get('base', 'window_title',
                                                      fallback='foolysh engine')
        self.__stats = AppStats(clock.Clock(), window_title)
        self.__app_clock = AppClock(self.__stats.clock)

        from . import dragdrop  # pylint: disable=import-outside-toplevel
        drag_threshold = self.__cfg.getfloat('base', 'drag_threshold',
                                             fallback=0.025)
        drag_button = self.__cfg.getint('base', 'drag_drop_button',
                                        fallback=sdl2.SDL_BUTTON_LEFT)
        self.__drag_drop = dragdrop.DragDrop(self, drag_threshold, drag_button)
        sdl2.ext.init()
        self.__stats.clean_exit = False
        _update_android()

    @property
    def isandroid(self):
        # type: () -> bool
        """``bool`` -> ``True`` if platform is android, otherwise ``False``."""
        return ISANDROID

    @property
    def event_handler(self, *unused_args, **unused_kwargs):
        # type: (...) -> eventhandler.EventHandler
        """``EventHandler``"""
        return self.__systems.event_handler

    @property
    def task_manager(self):
        # type: () -> taskmanager.TaskManager
        """``TaskManager``"""
        return self.__systems.task_manager

    @property
    def window(self):
        # type: () -> sdl2.ext.Window
        """``sdl2.ext.Window``"""
        return self.__systems.window

    @property
    def mouse_pos(self):
        # type: () -> vec2.Vec2
        """``Point`` -> current mouse position (=last touch location)"""
        return self.__stats.mouse_pos + vec2.Vec2()

    @property
    def screen_size(self):
        # type: () -> Tuple[int, int]
        """``Tuple[int, int]``"""
        if self.__systems.window is not None:
            return self.__systems.window.size
        if self.isandroid:
            display_mode = sdl2.SDL_DisplayMode()
            sdl2.SDL_GetCurrentDisplayMode(0, display_mode)
            return display_mode.w, display_mode.h
        size_x, size_y = self.__cfg.get('base', 'window_size',
                                        fallback='720x1280').split('x')
        return int(size_x), int(size_y)

    @property
    def clock(self):
        # type: () -> AppClock
        """:class:`foolysh.tools.clock.Clock` object of the running app."""
        return self.__app_clock

    @property
    def renderer(self):
        """:class:`foolysh.render.HWRenderer` object of the running app."""
        return self.__systems.renderer

    @property
    def root(self):
        # type: () -> node.Node
        """Root :class:`foolysh.scene.node.Node`."""
        return self.__nodes.root

    @property
    def ui(self):  # pylint: disable=invalid-name
        # type: () -> UIAnchors
        """:class:`UIAnchors`."""
        return self.__nodes.ui

    @property
    def drag_drop(self):
        # type: () -> dragdrop.DragDrop
        """:class:`~foolysh.dragdrop.DragDrop` instance."""
        return self.__drag_drop

    @property
    def config(self):
        # type: () -> config.Config
        """:class:`~foolysh.tools.config.Config` instance."""
        return self.__cfg

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
        """Updates 'mouse_pos'."""
        if not self.__stats.running:
            return
        x, y = ctypes.c_int(0), ctypes.c_int(0)
        state = sdl2.mouse.SDL_GetMouseState(ctypes.byref(x), ctypes.byref(y))
        world_unit = 1 / min(self.__systems.window.size)
        self.__stats.mouse_pos.x = x.value * world_unit
        self.__stats.mouse_pos.y = y.value * world_unit
        if state > 0 and self.__stats.mouse_down is None:
            self.__stats.mouse_down = self.__stats.mouse_pos + 0.0
        elif state == 0 and self.__stats.mouse_down is not None \
                and self.__stats.mouse_up is None:
            self.__stats.mouse_up = self.__stats.mouse_pos + 0.0
        elif state == 0 and self.__stats.mouse_down is not None \
                and self.__stats.mouse_up is not None:
            self.__stats.mouse_down = None
            self.__stats.mouse_up = None

    def __update_keyboard(self):
        """Updates keyboard state."""
        state = sdl2.SDL_GetKeyboardState(None)
        st_e = state[sdl2.SDL_SCANCODE_RETURN]
        state = sdl2.SDL_GetKeyboardState(None)
        st_e = st_e or state[sdl2.SDL_SCANCODE_RETURN2]
        state = sdl2.SDL_GetKeyboardState(None)
        st_e = st_e or state[sdl2.SDL_SCANCODE_KP_ENTER]
        if st_e and not self.__stats.enter_down:
            self.__stats.enter_down = True
        elif not st_e and self.__stats.enter_down:
            self.__stats.enter_down = False
            self.__stats.enter = True
        else:
            self.__stats.enter = False
        state = sdl2.SDL_GetKeyboardState(None)
        st_bs = state[sdl2.SDL_SCANCODE_BACKSPACE]
        if st_bs:
            self.__stats.backspace_down = True
        elif not st_bs and self.__stats.backspace_down:
            self.__stats.backspace_down = False
            self.__stats.backspace = True
        else:
            self.__stats.backspace = False

    def __update_ui_anchors(self, new_res):
        units_x = new_res[0] / min(new_res)
        units_y = new_res[1] / min(new_res)
        half_x, half_y = units_x / 2, units_y / 2
        self.__nodes.ui.top_center.pos = half_x, 0
        self.__nodes.ui.top_center.pos = units_x, 0
        self.__nodes.ui.center_left.pos = 0, half_y
        self.__nodes.ui.center.pos = half_x, half_y
        self.__nodes.ui.center_right.pos = units_x, half_y
        self.__nodes.ui.bottom_left.pos = 0, units_y
        self.__nodes.ui.bottom_center.pos = half_x, units_y
        self.__nodes.ui.bottom_right.pos = units_x, units_y

    def run(self):
        """
        Run the main loop until :meth:`App.quit` gets called.

        .. warning::
            Make sure to call ``super().run()`` if you override this method
            at the end of your implementation.
        """
        self.__init_sdl()
        self.__stats.clock.tick()
        last_time = self.__stats.clock.get_time()
        try:
            frame_clock = clock.Clock()
            self.__stats.running = True
            last_resolution = None
            while self.__stats.running:
                frame_clock.tick()
                new_res = self.screen_size
                if new_res != last_resolution:
                    self.__stats.resolution_change = True
                    self.__update_ui_anchors(new_res)
                else:
                    self.__stats.resolution_change = False
                self.__systems.event_handler()
                self.__update_mouse()
                self.__update_keyboard()
                if self.__systems.ui_handler(self.__stats.mouse_pos,
                                             self.__stats.mouse_down,
                                             self.__stats.mouse_up,
                                             self.__stats.enter,
                                             self.__stats.backspace):
                    self.renderer.set_dirty()
                self.__systems.animation_manager.animate(
                    self.__stats.clock.get_dt())
                self.__systems.task_manager()
                self.__systems.renderer.render()
                frame_clock.tick()
                sleep_time = max(0.0, FRAME_TIME - frame_clock.get_dt())
                if sleep_time:
                    time.sleep(sleep_time)
                self.__stats.frames += 1
                if self.__stats.frames % 10 == 0:
                    delta = self.__stats.clock.get_time() - last_time
                    self.__stats.fps = 10 / delta
                    last_time = self.__stats.clock.get_time()
                self.__stats.clock.tick()
        except (KeyboardInterrupt, SystemExit):
            self.quit(blocking=False)
        finally:
            sdl2.ext.quit()
            self.__stats.clean_exit = True

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
        if not self.__stats.running:
            return
        self.on_quit()
        self.__stats.running = False
        if blocking:
            while not self.__stats.clean_exit:
                time.sleep(0.01)

    def on_quit(self):
        """
        Method to override to perform cleanup when :meth:`App.quit()` gets
        called.
        """

    def __init_sdl(self):
        """Initializes SDL2."""
        sdl2.SDL_SetHint(sdl2.SDL_HINT_RENDER_SCALE_QUALITY, b'1')
        if self.isandroid:
            sdl2.ext.Window.DEFAULTFLAGS = sdl2.SDL_WINDOW_FULLSCREEN
            self.__systems.window = sdl2.ext.Window(
                self.__stats.window_title,
                size=self.screen_size
            )
        else:
            size_x, size_y = self.__cfg.get('base', 'window_size',
                                            fallback='480x800').split('x')
            self.__systems.window = sdl2.ext.Window(
                self.__stats.window_title,
                size=(int(size_x), int(size_y)),
                flags=sdl2.SDL_WINDOW_RESIZABLE
            )
        self.__systems.window.show()
        android.remove_presplash()
        self.__systems.renderer = render.HWRenderer(self.window)
        self.__systems.factory = sdl2.ext.SpriteFactory(
            sdl2.ext.TEXTURE,
            renderer=self.__systems.renderer
        )
        self.__stats.clean_exit = False
        self.__systems.sprite_loader = spriteloader.SpriteLoader(
            self.__systems.factory,
            self.__cfg.get('base', 'asset_dir', fallback='assets/'),
            self.__cfg.get('base', 'cache_dir', fallback=None)
        )
        self.__systems.renderer.root_node = self.__nodes.root
        self.__systems.renderer.uiroot = self.__nodes.ui.root
        self.__systems.renderer.asset_pixel_ratio = \
            self.__cfg.getint('base', 'asset_pixel_ratio')
        self.__systems.renderer.sprite_loader = self.__systems.sprite_loader

    def __del__(self):
        """Make sure, ``sdl2.ext.quit()`` gets called latest on destruction."""
        if not self.__stats.clean_exit:
            sdl2.ext.quit()
