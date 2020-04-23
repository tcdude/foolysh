"""
Provides the Config class to handle engine and user specific configuration.
"""

import configparser
import pathlib
import os
from typing import Union

from plyer import storagepath

from . import common

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


def _find_config_file():
    chk_loc = pathlib.Path('.').glob('**/foolysh.ini')
    chk_loc = [str(f) for f in chk_loc]
    if chk_loc:
        if len(chk_loc) == 1:
            return os.path.abspath(chk_loc[0])
        raise ValueError('found multiple "foolysh.ini" files in the '
                         'current working directory.')
    chk_loc = pathlib.Path(storagepath.get_application_dir()).glob(
        '**/foolish.ini',
    )
    chk_loc = [str(f) for f in chk_loc]
    if chk_loc:
        if len(chk_loc) == 1:
            return os.path.abspath(chk_loc[0])
        raise ValueError('found multiple "foolysh.ini" files in the '
                         'application directory.')
    chk_loc = pathlib.Path(storagepath.get_home_dir()).glob('**/foolish.ini')
    chk_loc = [str(f) for f in chk_loc]
    if chk_loc:
        if len(chk_loc) == 1:
            return os.path.abspath(chk_loc[0])
        raise ValueError('found multiple "foolysh.ini" files in the '
                         'home directory.')

    if os.access(os.getcwd(), os.W_OK):
        basedir = os.path.join(os.getcwd(), '.foolysh')
    elif os.access(storagepath.get_application_dir(), os.W_OK):
        basedir = os.path.join(storagepath.get_application_dir(), '.foolysh')
    elif os.access(storagepath.get_home_dir(), os.W_OK):
        basedir = os.path.join(storagepath.get_home_dir(), '.foolysh')
    else:
        raise PermissionError('unable to find writable location for config '
                              'file.')
    os.makedirs(basedir, exist_ok=True)
    parser = configparser.ConfigParser()
    parser.read_dict(common.DEFAULT_CONFIG)
    fpath = os.path.join(basedir, 'foolysh.ini')
    with open(fpath, 'w') as f:
        parser.write(f)
    return fpath


class Config(configparser.ConfigParser):
    """
    ConfigParser extended to allow for reloading and saving the configuration.

    Args:
        config_file: Either a valid path to an existing config file or `None` to
            automatically search for one and create a blank default
            configuration, if no valid configuration file could be found.
        *args: Optional positional arguments passed on to the parent class.
        **kwargs: Optional keyword arguments passed on to the parent class.
    """
    # pylint: disable=too-many-ancestors
    def __init__(self, config_file: Union[None, str], *args, **kwargs):
        super(Config, self).__init__(*args, **kwargs)
        self._cfg_path = config_file or _find_config_file()
        self.read(self._cfg_path)

    def reload(self):
        """Reload the configuration from disk."""
        self.read(self._cfg_path)

    def save(self):
        """Write the configuration to disk."""
        try:
            self.write(open(self._cfg_path, 'w'))
            return True
        except PermissionError as err:
            if err.errno == 13:
                return False
            raise err

    @property
    def cfg_path(self):
        """Return the path to the configuration file."""
        return self._cfg_path
