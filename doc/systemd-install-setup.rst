Example Linux Setup with Systemd
================================

Overview
--------

These instructions will give you a flexible and secure setup that starts
automatically at boot, restarts on crashes, and collects logs.

It also allows you to run as many instances as you want in parallel.

Instructions
------------

Create a dedicated directory for your server data. You can put this anywhere
you like. It is a good idea to put some identifier for your server, such as
``ctf`` in the folder name, in case you want to create more server configs in
the future.

.. code:: bash

   # mkdir -p /var/lib/aos/servername/

We want a seperate group to be able to restrict permissions in a more
granular way

.. code:: bash

   # groupadd --system aos

Optionally join your own user to the ``aos`` group to be able to
edit files in the server directory freely.

.. code:: bash

   # usermod -a -G aos yourusername

We want to copy the default config directory over.

.. code:: bash

   # horseradish --copy-config -d /var/lib/aos/servername

Edit a new file, ``/etc/systemd/system/horseradish@.service`` and insert
the following contents.

.. code:: ini

   [Unit]
   Description=Horseradish

   [Service]
   ExecStart=/usr/local/bin/horseradish -d /var/lib/aos/%i
   User=aos
   Group=aos
   Restart=always

   # Security Sandbox Settings
   Group=aos
   DynamicUser=true
   # only allow access to the state folder, nothing else
   ProtectHome=true
   TemporaryFileSystem=/var:ro
   PrivateDevices=true
   StateDirectory=aos/%i

   # disallow any unusual syscalls
   SystemCallFilter=@system-service

   [Install]
   WantedBy=network.target

You can now start, stop, and see the status of the process using
systemctl.

.. code:: bash

   # systemctl start horseradish@servername
   # systemctl stop horseradish@servername
   # systemctl status horseradish@servername

You will probably want to start the server at boot. To do this, run:

.. code:: bash

   # systemctl enable horseradish@servername

To tail the logs, run

.. code:: bash

   # journalctl -f -u horseradish@servername
