# Copyright (c) Mathias Kaerlev 2011-2012.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import importlib
import math
import random
import time
from typing import List, Optional, Union

from pyspades.logger import getLogger

from pyspades.vxl import VXLData
from horseradish.config import config

log = getLogger()


class MapNotFound(Exception):

    def __init__(self, the_map):
        self.map = the_map
        Exception.__init__(self, 'map %s does not exist' % the_map)

    def __nonzero__(self):
        return False


def check_rotation(maps: List[Union[str, 'RotationInfo']], load_dir:
                   Optional[str]=None) -> List['RotationInfo']:
    """
    Checks if provided maps exist in maps dir. and
    returns an array of RotationInfo objects for those maps.
    Raises MapNotFound exception if maps are not found.
    """
    if load_dir is None:
        load_dir = os.path.join(config.config_dir, 'maps')
    infos = []
    for the_map in maps:
        if not isinstance(the_map, RotationInfo):
            the_map = RotationInfo(the_map)
        infos.append(the_map)
        if (not os.path.isfile(the_map.get_map_filename(load_dir))
                and not os.path.isfile(the_map.get_meta_filename(load_dir))):
            raise MapNotFound(the_map)
    return infos


class Map:
    # pylint: disable=too-many-instance-attributes

    def __init__(self, rot_info: 'RotationInfo', load_dir: str) -> None:
        self.load_information(rot_info, load_dir)

        # we want to count how long a map load or generate takes
        start_time = time.monotonic()
        if self.gen_script:
            if map_on_seed_generation := getattr(self.info, 'on_seed_generation', None):
                seed = map_on_seed_generation(rot_info)
            else:
                seed = rot_info.get_seed()

            self.name = '{} #{}'.format(rot_info.name, seed)
            log.info("Generating map '{mapname}'...", mapname=self.name)
            random.seed(seed)
            self.data = self.gen_script(rot_info.name, seed)
        else:
            log.info("Loading map '{mapname}'...", mapname=self.name)
            self.load_vxl(rot_info)

        log.info('Map loaded successfully. (took {duration:.2f}s)',
                 duration=time.monotonic() - start_time)

    def load_information(self, rot_info: 'RotationInfo', load_dir: str) -> None:
        path = rot_info.get_meta_filename(load_dir)

        self.__dict__.update(
            __file__            = path,
            __name__            = rot_info.name,
            author              = '(unknown)',
            version             = '1.0',
            description         = '',
            extensions          = dict(),
            gen_script          = None,
            load_dir            = load_dir,
            rot_info            = rot_info,
            script              = None,
            time_limit          = None,
            cap_limit           = None,
            get_spawn_location  = None,
            get_entity_location = None,
            on_map_change       = None,
            on_map_leave        = None,
            on_block_destroy    = None,
            is_indestructable   = None,
            info                = self
        )

        try:
            with open(path, 'r') as fin:
                source = fin.read()
        except FileNotFoundError:
            log.error("Map info file not found {path}", path = path)

        try:
            exec(compile(source, path, 'exec'), self.__dict__)
        except Exception as exc:
            log.error("Error while loading map info", exc_info = exc)

        if self.gen_script:
            self.short_name = rot_info.name
            self.name = rot_info.full_name
        else:
            self.name = getattr(self, 'name', self.rot_info.name)
            self.short_name = self.name

    def apply_script(self, protocol, connection, config):
        if self.script is not None:
            protocol, connection = self.script(protocol, connection, config)
        return protocol, connection

    def load_vxl(self, rot_info):
        try:
            fp = open(rot_info.get_map_filename(self.load_dir), 'rb')
        except OSError:
            raise MapNotFound(rot_info.name)
        self.data = VXLData(fp)
        fp.close()


class RotationInfo:
    seed = None

    def __init__(self, name: str = "pyspades") -> None:
        self.full_name = name

        splitted = name.split("#")
        if len(splitted) > 1:  # user specified a seed
            name = splitted[0].strip()
            self.seed = int(splitted[1])
        self.name = name

    def get_seed(self) -> int:
        if self.seed is not None:
            return self.seed
        random.seed()
        return random.randint(0, int(math.pow(2, 31)))

    def get_map_filename(self, load_dir: str) -> str:
        return os.path.join(load_dir, '%s.vxl' % self.name)

    def get_meta_filename(self, load_dir: str) -> str:
        return os.path.join(load_dir, '%s.txt' % self.name)

    def __str__(self):
        return self.full_name
