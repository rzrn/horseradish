# Copyright © 2011–2012 Mathias Kaerlev
# Copyright © 2011 duckslingers
# Copyright © 2017 noway
# Copyright © 2017–2019 NotAFile
# Copyright © 2017–2019 Godwhoa
# Copyright © 2017 Flávio Monteiro

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
    command, get_player, PermissionDenied, player_only, target_player)


@command("client", "clin", "cli")
@target_player
def client(connection, player):
    """
    Tell you information about your client or the client of a given player
    /client [player]
    """
    if connection is player:
        who_is = "You are"
    else:
        who_is = player.name + " is"

    return "{} connected with {}".format(who_is, player.client_string)


@command()
def weapon(connection, value):
    """
    Tell you what weapon a given player is using
    /weapon <player>
    """
    player = get_player(connection.protocol, value)
    if player.weapon_object is None:
        name = '(unknown)'
    else:
        name = player.weapon_object.name
    return '%s has a %s' % (player.name, name)


@command()
@player_only
def intel(connection):
    """
    Inform you of who has the enemy intel
    /intel
    """
    flag = connection.team.other.flag
    if flag.player is not None:
        if flag.player is connection:
            return "You have the enemy intel, return to base!"
        else:
            return "%s has the enemy intel!" % flag.player.name
    return "Nobody in your team has the enemy intel"


@command()
def kill(connection, value=None):
    """
    Kill yourself or a given player
    /kill [target]
    """
    if value is None:
        player = connection
    else:
        if not connection.rights.kill and not connection.admin:
            raise PermissionDenied("you can't kill other players")
        player = get_player(connection.protocol, value, False)
    player.kill()
    if connection is not player:
        message = '%s killed %s' % (connection.name, player.name)
        connection.protocol.broadcast_chat(message)


@command(admin_only=True)
@target_player
def heal(connection, player):
    """
    Heal and refill yourself or a given player and inform everyone on the server of this action
    /heal [player]
    """
    if connection is player:
        message = '%s was healed' % (connection.name)
    else:
        message = '%s was healed by %s' % (player.name, connection.name)
    player.refill()
    connection.protocol.broadcast_chat(message)


@command()
def deaf(connection, value=None):
    """
    Make you or a given player no longer receive chat messages
    /deaf [player]
    """
    if value is not None:
        if not connection.admin and not connection.rights.deaf:
            return 'No administrator rights!'
        connection = get_player(connection.protocol, value)
    message = '%s deaf' % ('now' if not connection.deaf else 'no longer')
    message = "You're " + message
    if connection.deaf:
        connection.deaf = False
        connection.send_chat(message)
    else:
        connection.send_chat(message)
        connection.deaf = True
