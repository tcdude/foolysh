"""
Provides the ButtonList class for building interactive and filterable lists.
"""

from dataclasses import dataclass
from math import ceil

from typing import Any, Callable, Dict, List, Tuple

from ..scene.node import Origin
from . import button, frame, label

__author__ = 'Tiziano Bettio'
__license__ = 'MIT'
__version__ = '0.1'
__copyright__ = """
Copyright (c) 2020 Tiziano Bettio

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
SOFTWARE.
"""


@dataclass
class _FormatFields:
    """Holds fields containing format data."""
    button_kwargs: Dict[str, Any]
    filter_kwargs: Dict[str, Any]
    filter_active_color: Tuple[int, int, int]


@dataclass
class _DataFields:
    """Holds data fields and callbacks"""
    # pylint: disable=too-many-instance-attributes
    itpp: int
    data: List[str]
    listcb: Callable
    filters: List[str] = None
    filtercb: Callable = None
    page: int = 0
    active_filter: int = 0
    pagechangecb: Callable = None


@dataclass
class _Layout:
    """Holds UIElements needed for the layout."""
    buttons: List[button.Button] = None
    filter_buttons: List[button.Button] = None
    page_label: label.Label = None


class ButtonList(frame.Frame):
    """
    A UI element that provides a fixed length, paged list of buttons
    representing the contents of a `list` object. Provides optional filtering
    and callbacks for list-, filter- and page-change clicks.

    Args:
        data: The list of items.
        onclick: Callable that accepts a positional argument indicating the
            list entry clicked.
        itpp: Items per page
        button_kwargs: Keyword arguments used to construct list entries
            (rendered as buttons).
        parent: Optional Node to reparent to.
        filters: Optional filter buttons for the list.
        filtercb: Optional callable that accepts a positional argument
            indicating the filter index clicked.
        filter_kwargs: Optional keyword arguments used for the filter buttons.
        filter_active: Optional color for the border and text of the currently
            active filter button.
        item_font_ratio: Optional factor to help adjust font size inside the
            list elements. This is applied as `button_height * item_font_ratio`.
        **kwargs: Keyword arguments passed on to
            :class:`~foolysh.ui.frame.Frame`.
    """
    def __init__(self, data: List[str], onclick: Callable, itpp: int,
                 button_kwargs: Dict[str, Any], filters: List[str] = None,
                 filtercb: Callable = None,
                 filter_kwargs: Dict[str, Any] = None,
                 filter_active: Tuple[int, int, int] = (240, ) * 3,
                 item_font_ratio: float = 0.43, **kwargs) -> None:
        # pylint: disable=too-many-arguments
        super().__init__(**kwargs)
        if filters and filtercb is None:
            raise ValueError('Expected filtercb when filters are specified.')
        if isinstance(filters, list) and len(filters) == 1:
            raise ValueError('Argument filters cannot be of length 1.')
        if filters and filter_kwargs is None:
            filter_kwargs = {}
            filter_kwargs.update(button_kwargs)
            if 'border_color' not in filter_kwargs:
                filter_kwargs['border_color'] = (200, ) * 3
            if 'text_color' not in filter_kwargs:
                filter_kwargs['text_color'] = (200, 220, 200)
        self._item_font_ratio = item_font_ratio
        self.origin = Origin.CENTER
        self._format = _FormatFields(button_kwargs, filter_kwargs, filter_active)
        self._data = _DataFields(itpp, data, onclick, filters, filtercb)
        self._layout = _Layout([], [])
        self._setup_layout()

    def onpagechange(self, callback: Callable[[int], int]) -> None:
        """
        Register a callback for page change. The callback has to accept a single
        int representing the page after the page change and return the page
        number, where to turn to instead.
        """
        self._data.pagechangecb = callback

    def update_filter(self, index: int, text: str = '',
                      enabled: bool = None) -> None:
        """
        Update the text and enabled state of one of the filter buttons in the
        list.
        """
        if text:
            self._layout.filter_buttons[index].change_text(text)
        if enabled is not None:
            self._layout.filter_buttons[index].enabled = enabled

    def _setup_layout(self) -> None:
        width, height = self.size
        margin = max(self.corner_radius / 2, self.border_thickness)
        btn_width = (width - 2 * margin)
        if self._data.filters:
            offset = 1
        else:
            offset = 0
        btn_height = (height - 2 * margin) / (self._data.itpp + 2 + offset)
        if offset > 0:
            self._generate_filters(height / -2 + margin, margin, width,
                                   btn_height)
        spacing = btn_height / (self._data.itpp + offset)
        btn_kw = self._format.button_kwargs
        btn_kw['size'] = btn_width, btn_height
        btn_kw['font_size'] = btn_height * self._item_font_ratio
        if 'align' not in btn_kw:
            btn_kw['align'] = 'left'
        self._generate_list(btn_kw, height, offset, spacing, margin)
        pos_y = height / -2 + margin + (self._data.itpp + offset) * btn_height
        pos_y += (self._data.itpp + offset) * spacing
        self._generate_paging(pos_y, (btn_width, btn_height))

    def _generate_list(self, btn_kw: Dict[str, Any], height: float, offset: int,
                       spacing: float, margin: float) -> None:
        # pylint: disable=too-many-arguments
        btn_width, btn_height = btn_kw['size']
        for i in range(self._data.itpp):
            if len(self._data.data) > i:
                txt = self._data.data[i]
            else:
                txt = ''
            btn = button.Button(text=txt, parent=self, **btn_kw)
            j = i + offset
            btn.pos = (btn_width / -2,
                       height / -2 + margin + j * btn_height + j * spacing)
            btn.onclick(self._cb_redirect, i)
            self._layout.buttons.append(btn)

    def _generate_filters(self, pos_y: float, margin: float,
                          width: float, btn_height: float) -> None:
        btn_width = (width - 2 * margin) / (len(self._data.filters) + 0.2)
        spacing = (width - 2 * margin) - len(self._data.filters) * btn_width
        spacing /= len(self._data.filters) - 1
        pos_x = (width - 2 * margin) / -2
        btn_kw = self._format.filter_kwargs
        btn_kw['size'] = btn_width, btn_height
        btn_kw['align'] = 'center'
        btn_kw['font_size'] = btn_height * 0.42
        for i, value in enumerate(self._data.filters):
            btn = button.Button(text=value, parent=self, **btn_kw)
            btn.pos = pos_x, pos_y
            btn.onclick(self._filtercb_redirect, i)
            self._layout.filter_buttons.append(btn)
            pos_x += btn_width + spacing

    def _generate_paging(self, pos_y: float,
                         btn_size: Tuple[float, float]) -> None:
        btn_width, btn_height = btn_size
        btn_kw = self._format.button_kwargs
        btn_kw['align'] = 'center'
        btn_kw['size'] = btn_width / 3.5, btn_height
        btn_kw['font_size'] = btn_height * 0.5
        btn = button.Button(text=chr(0xf0d9), parent=self, **btn_kw)
        btn.pos = btn_width / -2, pos_y
        btn.onclick(self._change_page, -1)
        self._layout.buttons.append(btn)
        btn = button.Button(text=chr(0xf0da), parent=self, **btn_kw)
        btn.pos = btn_width / 2 - btn_kw['size'][0], pos_y
        btn.onclick(self._change_page, 1)
        self._layout.buttons.append(btn)
        lbl_kw = {}
        for k in btn_kw:
            if k.startswith(('down_', 'disabled_')):
                continue
            lbl_kw[k] = btn_kw[k]
        self._layout.page_label = label.Label(text='', parent=self, **lbl_kw)
        self._layout.page_label.pos = btn_kw['size'][0] / -2, pos_y

    def update_content(self, reset_page: bool = False) -> None:
        """Update the displayed content."""
        if reset_page:
            self._data.page = 0
        num_pages = int(ceil(len(self._data.data) / self._data.itpp))
        if self._data.page + 1 > num_pages:
            self._data.page = max(num_pages - 1, 0)
        page_offset = self._data.page * self._data.itpp
        for i in range(self._data.itpp):
            idx = page_offset + i
            if idx < len(self._data.data):
                for j in self._layout.buttons[i].labels:
                    j.text = self._data.data[idx]
                if self._layout.buttons[i].hidden:
                    self._layout.buttons[i].show()
            else:
                self._layout.buttons[i].hide()
        if num_pages > 1:
            self._layout.page_label.show()
            self._layout.page_label.text = f'{self._data.page + 1}/{num_pages}'
            for i in self._layout.buttons[-2:]:
                i.show()
        else:
            self._layout.page_label.hide()
            for i in self._layout.buttons[-2:]:
                i.hide()
        self._update_filter()

    def _change_page(self, direction: int) -> None:
        max_page = self.numpages - 1
        if self._data.page + direction < 0:
            self._data.page = max_page
        elif self._data.page + direction > max_page:
            self._data.page = 0
        else:
            self._data.page += direction
        if self._data.pagechangecb is not None:
            new_page = self._data.pagechangecb(self._data.page)
            if new_page != self._data.page:
                self._data.page = new_page
        self.update_content()
        self.dirty = True

    def _cb_redirect(self, pos: int) -> None:
        self._data.listcb(self._data.page * self._data.itpp + pos)

    def _filtercb_redirect(self, pos: int) -> None:
        self._data.filtercb(pos)
        self._data.active_filter = pos
        self._update_filter()

    def _update(self) -> None:
        self.update_content()
        super()._update()

    def _update_filter(self) -> None:
        if not self._data.filters:
            return
        for i, btn in enumerate(self._layout.filter_buttons):
            if i == self._data.active_filter:
                bcol = self._format.filter_active_color
            else:
                bcol = self._format.filter_kwargs['border_color']
            for j in btn.labels:
                j.border_color = bcol

    @property
    def page(self) -> int:
        """The current page, starting at 0."""
        return self._data.page

    @property
    def numpages(self) -> int:
        """The the number of pages."""
        return int(ceil(len(self._data.data) / self._data.itpp))
