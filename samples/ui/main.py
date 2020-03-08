"""
Sample application to demonstrate the ui submodule.
"""

import sdl2

from foolysh import app
from foolysh.ui import button, frame, entry, label

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


def set_text(element, text):
    """A non member callback function."""
    element.text = text


class UISample(app.App):
    """
    Sample app with some UI elements.
    """
    def __init__(self):
        super().__init__(config_file='foolysh.ini')

        screen_x, screen_y = self.screen_size
        world_unit = min(screen_x, screen_y)
        ratio_x, ratio_y = screen_x / world_unit, screen_y / world_unit

        my_frame = frame.Frame(name='A Frame', size=(0.8, 0.8),
                               pos=((ratio_x - 0.8) / 2, (ratio_y - 0.8) / 2),
                               frame_color=(30, 210, 50), border_thickness=0.01,
                               border_color=(255, 255, 255), corner_radius=0.05,
                               alpha=190)
        my_frame.reparent_to(self.ui.top_left)

        my_label = label.Label(name='A Label', size=(0.6, 0.1), pos=(0.1, 0.1),
                               margin=0.05, text='Sample Label',
                               font='fonts/SpaceMono.ttf', font_size=0.06,
                               text_color=(10, 10, 10, 255),
                               align='center', frame_color=(210, 210, 210),
                               border_thickness=0.002,
                               border_color=(0, 0, 0), corner_radius=0.02,
                               alpha=255)
        my_label.reparent_to(my_frame)

        self.but_a = button.Button(name='A Button', size=(0.26, 0.1),
                                   pos=(0.1, 0.22), margin=0.05, text='ButtonA',
                                   font='fonts/SpaceMono.ttf', font_size=0.05,
                                   text_color=(10, 10, 10, 255), align='center',
                                   frame_color=(210, 210, 210),
                                   border_thickness=0.002,
                                   border_color=(0, 0, 0), corner_radius=0.02,
                                   alpha=240, down_frame_color=(255, 255, 255),
                                   down_text='Down', down_alpha=255,
                                   disabled_text='Disabled',
                                   disabled_frame_color=(180, 180, 180),
                                   disabled_border_color=(180, 180, 180),
                                   disabled_text_color=(240, 240, 240, 255))
        self.but_a.reparent_to(my_frame)
        self.but_a.onclick(self.toggle, 0)

        self.but_b = button.Button(name='A Button', size=(0.26, 0.1),
                                   pos=(0.44, 0.22), margin=0.05,
                                   text='ButtonB', font='fonts/SpaceMono.ttf',
                                   font_size=0.05, text_color=(10, 10, 10, 255),
                                   align='center', frame_color=(210, 210, 210),
                                   border_thickness=0.002,
                                   border_color=(0, 0, 0), corner_radius=0.02,
                                   alpha=240, down_frame_color=(255, 255, 255),
                                   down_text='Down', down_alpha=255,
                                   disabled_text='Disabled',
                                   disabled_frame_color=(180, 180, 180),
                                   disabled_border_color=(180, 180, 180),
                                   disabled_text_color=(200, 200, 200, 255))
        self.but_b.reparent_to(my_frame)
        self.but_b.onclick(self.toggle, 1)
        self.but_b.enabled = False

        my_entry = entry.Entry(name='An Input', size=(0.6, 0.1),
                               pos=(0.1, 0.34), margin=0.01,
                               hint_text='Enter text...',
                               hint_text_color=(10, 10, 10, 180),
                               font='fonts/SpaceMono.ttf', font_size=0.06,
                               text_color=(10, 10, 10, 255),
                               align='left', frame_color=(255, 255, 255),
                               border_thickness=0.001,
                               border_color=(0, 0, 0), corner_radius=0.02,
                               alpha=255)
        my_entry.reparent_to(my_frame)

        my_label = label.Label(name='Event Label', size=(0.6, 0.1),
                               pos=(0.1, 0.46), margin=0.05, text='',
                               font='fonts/SpaceMono.ttf', font_size=0.04,
                               text_color=(10, 10, 10, 255),
                               align='left', frame_color=(210, 210, 210),
                               border_thickness=0.002,
                               border_color=(0, 0, 0), corner_radius=0.02,
                               alpha=255)
        my_label.reparent_to(my_frame)

        my_entry.oninput(set_text, my_label, text='Input Event')
        my_entry.onenterfocus(set_text, my_label, text='Enter Focus')
        my_entry.onexitfocus(set_text, my_label, text='Exit Focus')
        my_entry.onenter(set_text, my_label, text='Enter/Return Key')

        self.event_handler.listen(
            'quit',
            sdl2.SDL_QUIT,
            self.quit,
            priority=0,
            blocking=False
        )

    def toggle(self, value):
        """Callback method executed when a button is clicked."""
        if value == 0:
            self.but_a.enabled = False
            self.but_b.enabled = True
        else:
            self.but_a.enabled = True
            self.but_b.enabled = False


UISample().run()
