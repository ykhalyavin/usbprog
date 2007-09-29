# setup.py
from distutils.core import setup
import glob, sys, py2exe, os

sys.path.append("../pybsl")
sys.path.append("../pyserjtag")


sys.argv.append("sdist")
sys.argv.append("py2exe")

setup(
    name='pyserjtag',
    version='0.1',
    options = {"py2exe":
        {
            'dist_dir': 'bin',
            'excludes': ['javax.comm', 'fcntl'],
        }
    },
    console = ["firmware_updater.py"],
    zipfile = "lib/shared-pyserjtag-firmware-updater.zip",
)

#~ if os.path.exists('bin/firmware_updater.exe'):
    #~ if os.path.exists('bin/msp430-serjtag.exe'):
        #~ os.remove('bin/msp430-serjtag.exe')
    #~ os.rename('bin/firmware_updater.exe', 'bin/msp430-serjtag.exe')