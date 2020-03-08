"""
Sample application to demonstrate the ability to display shapes with "Signed
Distance Functions".
"""

import sdl2

from foolysh import app
from foolysh.tools import sdf

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


class SDFSample(app.App):
    """
    Sample to demonstrate the use of SDF strings to create image nodes.
    """
    def __init__(self):
        super().__init__(config_file='foolysh.ini')

        # Create a framed box with relative values to world units (smaller part
        # of screen size = 1 world unit) using a SDF string. When relative
        # values are provided, the SDF will scale according to the screen.
        relative_box = self.root\
            .attach_image_node('Box', 'SDF:box:F:width=0.8:height=0.2:'
                                      'corner_radius=0.1:border_thickness=0.01:'
                                      'frame_color=(180, 40, 10):'
                                      'border_color=(230, 230, 230):'
                                      'multi_sampling=1:alpha=180')
        relative_box.pos = 0.1, 0.1
        txt = self.root.attach_text_node('RBoxText', 'multi_sampling=1',
                                         'fonts/SpaceMono.ttf', 0.05)
        txt.pos = relative_box, 0.12, 0.06
        txt.depth = 15

        # Create a framed circle using the helper function from the sdf module
        # to generate the string for us.
        sdfstr = sdf.framed_circle_str(radius=0.2, border_thickness=0.01,
                                       frame_color=(40, 210, 20),
                                       border_color=(230, 230, 230),
                                       multi_sampling=2, alpha=150)
        relative_circle = self.root\
            .attach_image_node('Circle', sdfstr)
        screen_x, screen_y = self.screen_size
        half_y = screen_y / screen_x / 2
        relative_circle.pos = 0.3, half_y - 0.4
        txt = self.root.attach_text_node('RCircleText', 'multi_sampling=2',
                                         'fonts/SpaceMono.ttf', 0.025)
        txt.pos = relative_circle, 0.07, 0.12
        txt.depth = 15

        # Create a framed box with absolute values (=pixel). With absolute
        # values the SDF will not scale when the screen size changes.
        width, height = int(screen_x * 0.8), int(screen_y * 0.06)
        box = self.root\
            .attach_image_node('Box', f'SDF:box:I:width={width}:'
                                      f'height={height}:'
                                      f'corner_radius={height // 4}:'
                                      f'border_thickness={height // 8}:'
                                      f'frame_color=(40, 10, 210):'
                                      f'border_color=(230, 230, 230):'
                                      f'multi_sampling=3:alpha=180')
        box.pos = (0.5 - (width / 2) / screen_x,
                   half_y + half_y - height / screen_x - 0.025)
        txt = self.root.attach_text_node('BoxText', 'multi_sampling=3',
                                         'fonts/SpaceMono.ttf', 0.04)
        txt.pos = box, 0.1, 0.025
        txt.depth = 15

        # Create a framed circle with absolute values (=pixel).
        radius, border = int(screen_x * 0.4), int(screen_x * 0.4 / 30)
        circle = self.root\
            .attach_image_node('Circle', f'SDF:circle:I:radius={radius}:'
                                         f'border_thickness={border}:'
                                         f'frame_color=(210, 20, 210):'
                                         f'border_color=(230, 230, 230):'
                                         f'multi_sampling=4:alpha=180')
        circle.pos = 0.5 - radius / screen_x, half_y - radius / 2 / screen_x
        circle.depth = 10
        txt = self.root.attach_text_node('CircleText', 'multi_sampling=4',
                                         'fonts/SpaceMono.ttf', 0.06)
        txt.pos = circle, 0.12, 0.35
        txt.depth = 15

        self.event_handler.listen(
            'quit',
            sdl2.SDL_QUIT,
            self.quit,
            blocking=False
        )


SDFSample().run()
