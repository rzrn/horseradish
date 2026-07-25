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

import sys
import code
import traceback
import rlcompleter
from os import path
from contextlib import redirect_stdout, redirect_stderr

try:
    import asyncssh
except ImportError as e:
    print("ERROR: piqueserver was not installed with the [ssh] option")
    print("but SSH was enabled in the settings")
    print(e)
    sys.exit(1)

from piqueserver.config import config

def ssh_handle_client(*, locals = None, ps1 = ">>> ", ps2 = "... "):
    async def handle_client(process):
        client_locals = dict(locals)

        console = code.InteractiveInterpreter(client_locals)
        completer = rlcompleter.Completer(client_locals)

        def on_tab_key_received(line, pos):
            left, right = line[:pos], line[pos:]

            if middle := completer.complete(left, 0):
                return middle + right, len(middle)
            else:
                return True

        process.channel.register_key('\t', on_tab_key_received)

        try:
            buffer = []

            while True:
                process.stdout.write(ps2 if len(buffer) > 0 else ps1)

                recv = await process.stdin.readline()
                buffer.append(recv.rstrip('\n'))

                # This blocks the main thread, but otherwise it is really hard
                # to redirect stdout/stderr without messing everything up.
                # `twisted.conch.manhole` blocks in the corresponding place as well.
                with redirect_stdout(process.stdout), redirect_stderr(process.stdout):
                    if console.runsource('\n'.join(buffer)) is False:
                        buffer.clear()
        except asyncssh.BreakReceived:
            pass
        except SystemExit:
            pass
        except Exception as exc:
            traceback.print_exc()

        process.exit(0)

    return handle_client

async def create_remote_console(host, port, *, server_host_keys, authorized_client_keys, locals = None):
    ssh_base_path = path.join(config.config_dir, "ssh")

    server_host_keys_path = path.join(ssh_base_path, server_host_keys)
    authorized_client_keys_path = path.join(ssh_base_path, authorized_client_keys)

    if path.isfile(server_host_keys_path) is False:
        print("ERROR: You don't have any keys in the host key location")
        print("Generate one with:")
        print("  mkdir -p {}".format(ssh_base_path))
        print("  ssh-keygen -f {} -t ed25519".format(server_host_keys_path))
        print("Make sure to specify no password")

        return

    if path.isfile(authorized_client_keys_path) is False:
        print("ERROR: You don't have any authorized keys configured. Add one with:")
        print("  echo /path/to/your/id_rsa.pub >> {}".format(authorized_client_keys_path))

        return

    await asyncssh.listen(
        host, port,
        server_host_keys = server_host_keys_path,
        authorized_client_keys = authorized_client_keys_path,
        process_factory = ssh_handle_client(locals = locals)
    )