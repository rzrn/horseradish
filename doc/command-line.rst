Command line arguments
======================

View of help output
-------------------

::

    $ horseradish --help
    usage: horseradish [-h] [-c CONFIG_FILE] [-j JSON_PARAMETERS] [-d CONFIG_DIR] [--copy-config] [--update-geoip] [--version]

    horseradish is an open-source Python server implementation for the voxel-based game "Ace of Spades".

    options:
      -h, --help            show this help message and exit
      -c, --config-file CONFIG_FILE
                            specify the config file - default is "config.toml" in the config dir
      -j, --json-parameters JSON_PARAMETERS
                            add extra settings in json format (overrides the config present in the config file)
      -d, --config-dir CONFIG_DIR
                            specify the directory which contains maps, scripts, etc (in correctly named subdirs) - default is
                            ~/.config/horseradish
      --copy-config         copies the default/example config dir to its default location or as specified by "-d"
      --update-geoip        download the latest geoip database
      --version             show the version and exit

Explanation
-----------

``-h`` or ``--help``
~~~~~~~~~~~~~~~~~~~~

self explanatory - display help about running

``-c`` or ``--config-file``
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Takes a parameter which is the path to the desired configuration file.
Defaults to ``config.json`` is the configuration directory.

``-d`` or ``--config-dir``
~~~~~~~~~~~~~~~~~~~~~~~~~~

Specifies the directory to use for its configuration. Defaults to
``$XDG_CONFIG_HOME/horseradish/`` or ``$HOME/.config/horseradish/`` if
the former environment variable isn’t set. This directory is also used
by ``--copy-config`` as the target directory for copying the example
configuration, as well as the base path when giving a relative path to
``--config-file``.

``--copy-config``
~~~~~~~~~~~~~~~~~

Copies the included example configuration directory to the default
configuration directory, or to the location specified by
``--config-dir``. Will create the directory if it doesn’t exist, and
will not copy if the directory already exists to avoid overwriting
existing config.

``-j`` or ``--json-parameters``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Example: ``horseradish -j '{"profile":true}'``

Takes the json object and uses it to override fields from the json
configuration file. Useful for testing out a quick change where you
don’t want to edit ``config.json``.

``--update-geoip``
~~~~~~~~~~~~~~~~~~

Downloads the latest data file containing geoip data into
``data/GeoLiteCity.dat`` in the configuration directory. This data file
is required for the ``from`` command to work in-game.
