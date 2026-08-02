Installation
============

.. note:: Only Python 3.10 and above is supported


All platforms
--------------

Installing from source
~~~~~~~~~~~~~~~~~~~~~~
.. code:: bash

    # required for https, pillow, map compression etc.
    # these are for ubuntu 16.04, find similar packages for your own distro/OSs
    sudo apt-get install python3-dev libssl-dev libffi-dev libjpeg-dev zlib1g-dev
    # get the source
    git clone https://github.com/rzrn/horseradish
    cd horseradish
    # we make git tags for every version so you can checkout out to specific version if you want
    # git checkout v0.1.3
    # create a new python3 venv
    virtualenv -p python3 venv
    source venv/bin/activate
    # install deps.
    pip install -r requirements.txt
    # install the server itself
    python setup.py install

    # don't forget to deactivate the venv when finished!
    deactivate

Windows
-------

Installation from source
~~~~~~~~~~~~~~~~~~~~~~~~

Tricky bit for Windows is to get Cython working. 

* Install Visual C++ compiler please follow `this guide <https://wiki.python.org/moin/WindowsCompilers>`_.
* Don't forget to upgrade `setuptools`
* Install git or download sources from github and unzip
* If you decided to use git: `git clone https://github.com/rzrn/horseradish`

.. tip:: If you see errors like "unable to find vcvarsall.bat" refer to `this article <https://blogs.msdn.microsoft.com/pythonengineering/2016/04/11/unable-to-find-vcvarsall-bat/>`_.

.. code:: bash

    cd horseradish
    pip3 install -r requirements.txt
    python setup.py install

