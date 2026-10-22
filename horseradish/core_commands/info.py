# Copyright © 2011 Mathias Kaerlev
# Copyright © 2011 duckslingers
# Copyright © 2017 NotAFile
# Copyright © 2017 Flávio Monteiro
# Copyright © 2018–2019 Godwhoa
# Copyright © 2019 Samuel Walladge
# Copyright © 2022 lokka30

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

from horseradish.commands import (
    command, _commands, has_permission, get_player, get_command_help, player_only, target_player)


@command()
@player_only
def streak(connection):
    """
    Tell your current kill streak
    /streak
    """
    return ('Your current kill streak is %s. Best is %s kills' %
            (connection.streak, connection.best_streak))


@command()
@target_player
def ping(connection, player):
    """
    Tell your current ping (time for your actions to be received by the server)
    /ping
    """
    ping = player.latency
    if connection is player:
        return 'Your ping is {} ms. Lower ping is better!'.format(player.latency)
    return "{}'s ping is {} ms".format(player.name, player.latency)


@command()
@player_only
def rules(connection):
    """
    Show you the server rules
    /rules
    """
    lines = connection.protocol.rules
    if lines is None:
        return
    connection.send_lines(lines, 'rules')


@command()
def commands(connection):
    """
    Print all available commands
    /commands
    """
    # Compact and simple output for admins
    if connection.admin:
        names = [command.command_name for command in _commands.values()
                 if has_permission(command, connection)]
        return 'Available commands: %s' % (', '.join(names))
    # More helpful output for regular players
    lines = []
    for cmd in _commands.values():
        if not has_permission(cmd, connection):
            continue
        desc, _, _ = get_command_help(cmd)
        lines.append("/{} {}".format(cmd.command_name, desc))
    connection.send_lines(lines, 'commands')


@command("help")
def help_command(connection, command_name=None):
    """
    Gives description and usage info for a command
    /help <command_name>
    """
    # Querying usage for a specific command
    if command_name:
        if command_name not in _commands:
            return 'Unknown command'
        command_func = _commands[command_name]
        desc, usage, _ = get_command_help(command_func)
        return 'Description: {}\n Usage: {}'.format(desc, usage)
    # Output help if present in config
    if connection.protocol.help:
        return connection.send_lines(connection.protocol.help, 'help')
