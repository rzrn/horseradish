# Copyright (c) Mathias Kaerlev 2011-2012.

# This file is part of pyspades.

# pyspades is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# pyspades is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with pyspades.  If not, see <http://www.gnu.org/licenses/>.

import asyncio
from pyspades.bytes import ByteWriter

import pyspades.enet as enet


class BaseConnection:
    disconnected = False
    timeout_call = None

    def __init__(self, protocol, peer):
        self.protocol = protocol
        self.peer = peer

    def timed_out(self):
        self.disconnect()

    def disconnect(self, data=0):
        if self.disconnected:
            return
        self.disconnected = True
        # disconnect_later waits for queued reliable packets to drain before
        # sending DISCONNECT, so e.g. the kick-reason chat reaches the client
        # before the disconnect notification fires.
        self.peer.disconnect_later(data)
        self.protocol.remove_peer(self.peer)
        self.on_disconnect()

    def loader_received(self, loader):
        raise NotImplementedError('loader_received() not implemented')

    def send_contained(self, contained, sequence=False):
        if self.disconnected:
            return
        if sequence:
            flags = enet.PACKET_FLAG_UNSEQUENCED
        else:
            flags = enet.PACKET_FLAG_RELIABLE
        data = ByteWriter()
        contained.write(data)
        packet = enet.Packet(bytes(data), flags)
        self.peer.send(0, packet)

    # events

    def on_connect(self):
        pass

    def on_disconnect(self):
        pass

    # properties

    @property
    def latency(self):
        return self.peer.roundTripTime


class BaseProtocol:
    connection_class = BaseConnection
    max_connections = 33
    is_client = False

    def __init__(self, port=None, interface=b'*',
                 update_interval=1 / 60.0):
        if port is not None and interface is not None:
            address = enet.Address(interface, port)
        else:
            address = None
        try:
            self.host = enet.Host(address, self.max_connections, 1)
        except MemoryError:
            # pyenet raises memoryerror when the enet host could not be created
            raise IOError("Failed to create ENet Host. Is the port in use?")

        self.host.compress_with_range_coder()
        self.update_loop = asyncio.ensure_future(self.update())
        self.connections = {}

    def connect(self, connection_class, host, port, version, channel_count = 1, timeout = 5.0):
        host = host.encode()
        peer = self.host.connect(enet.Address(host, port), channel_count, version)

        connection = connection_class(self, peer)
        connection.timeout_call = asyncio.get_running_loop().call_later(timeout, connection.timed_out)

        peer.data = connection

        return connection

    def remove_peer(self, peer):
        self.connections.pop(peer, None)

    def update(self):
        try:
            while True:
                if self.host is None:
                    return
                try:
                    event = self.host.service(0)
                except IOError:
                    break
                if event is None:
                    break

                event_type = event.type
                if event_type == enet.EVENT_TYPE_NONE:
                    break

                peer = event.peer
                connection = peer.data

                if event_type == enet.EVENT_TYPE_CONNECT:
                    if connection is None:
                         # For outcoming connections we fill `peer.data` before this point.
                         connection = self.connection_class(self, peer)
                         self.connections[peer] = connection
                         peer.data = connection
                    else:
                         # This is installed for each incoming connection in `connect(...)`.
                         connection.timeout_call.cancel()

                    connection.on_connect()
                elif event_type == enet.EVENT_TYPE_DISCONNECT:
                    if connection.disconnected is False:
                        connection.disconnected = True
                        connection.on_disconnect()

                    self.remove_peer(peer)
                    peer.data = None
                elif event_type == enet.EVENT_TYPE_RECEIVE:
                    connection.loader_received(event.packet)
        except:
            # make sure the LoopingCall doesn't catch this and stops
            import traceback
            traceback.print_exc()
