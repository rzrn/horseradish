# Copyright © 2011–2012 Mathias Kaerlev
# Copyright © 2011 enari
# Copyright © 2017–2019 NotAFile
# Copyright © 2017 Godwhoa
# Copyright © 2017 Flávio Monteiro
# Copyright © 2024 utf-4096
# Copyright © 2026 rzrn

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

import logging
import random
import sys

from pyspades.logger import getLogger

from horseradish.commands import command, join_arguments

log = getLogger()


@command('servername', admin_only=True)
def server_name(connection, *arg):
    """
    Change the server name until it restarts
    /servername <new-name>
    Also affects the master list
    """
    if not arg:
        raise ValueError("no argument given")
    name = join_arguments(arg)
    protocol = connection.protocol
    protocol.set_server_name(name)
    message = "%s changed servername to '%s'" % (connection.name, name)
    log.info("{message}", message=message)
    if connection in connection.protocol.players.values():
        return message
    return None


@command('server')
def server_info(connection):
    """
    Tell you the name of this server and its aos:// URI
    /server
    """
    protocol = connection.protocol
    msg = 'You are playing on "%s"' % protocol.name
    if protocol.identifier is not None:
        msg += ' at %s' % protocol.identifier
    return msg


@command()
def version(connection):
    """
    Tell you this server's software version
    /version
    """
    import horseradish
    return 'Server version is "{}" on "{}"'.format(
        horseradish.__version__, sys.platform)


@command()
def scripts(connection):
    """
    Tell you which scripts are enabled on this server currently
    /scripts
    """
    scripts = connection.protocol.config.get('scripts', [])
    if not scripts:
        return 'No scripts are enabled currently'
    return 'Scripts enabled: %s' % (', '.join(scripts))


@command('togglemaster', 'master', admin_only=True)
def toggle_master(connection):
    """
    Toggle connection to the master server list
    /togglemaster
    """
    protocol = connection.protocol
    protocol.set_master_state(not protocol.master)
    message = ("toggled master broadcast %s" % ['OFF', 'ON'][
        int(protocol.master)])
    if connection in connection.protocol.players.values():
        return "You " + message


@command('loglevel', admin_only = True)
def c_loglevel(connection, argval = None):
    """
    Get or set the current logging level for the root logger
    /loglevel [DEBUG|INFO|WARNING|ERROR|CRITICAL]
    """

    rootLogger = logging.getLogger()
    oldLevelName = logging.getLevelName(rootLogger.level)

    if argval is None:
        return "{}".format(oldLevelName)

    loglevel = logging.getLevelName(argval)

    if isinstance(loglevel, int):
        rootLogger.setLevel(loglevel)

        newLevelName = logging.getLevelName(rootLogger.level)
        return "{} -> {}".format(oldLevelName, newLevelName)
    else:
        return "Unknown logging level: {}".format(argval)