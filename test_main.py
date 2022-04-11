from test_checks import *
import subprocess
import os

# Credit to https://gist.github.com/martin-ueding/4007035
class Colorcodes(object):
    """
    Provides ANSI terminal color codes which are gathered via the ``tput``
    utility. That way, they are portable. If there occurs any error with
    ``tput``, all codes are initialized as an empty string.

    The provides fields are listed below.

    Control:

    - bold
    - reset

    Colors:

    - blue
    - green
    - orange
    - red

    :license: MIT
    """
    def __init__(self):
        try:
            self.bold = subprocess.check_output("tput bold".split())
            self.reset = subprocess.check_output("tput sgr0".split())

            self.blue = subprocess.check_output("tput setaf 4".split())
            self.green = subprocess.check_output("tput setaf 2".split())
            self.orange = subprocess.check_output("tput setaf 3".split())
            self.red = subprocess.check_output("tput setaf 1".split())
        except subprocess.CalledProcessError as e:
            self.bold = ""
            self.reset = ""

            self.blue = ""
            self.green = ""
            self.orange = ""
            self.red = ""

_c = Colorcodes()

os.system('make')
try:
    subprocess.run(['./barrier_test', '-all'], timeout=30)
except Exception:
    print(_c.orange.decode() + "You might've timed out... This likely indicates a race in your code. " + _c.reset.decode())
res = check_tests()
fail= _c.red.decode() + "failed" + _c.reset.decode()
good = _c.bold.decode() + _c.green.decode() + "passed" + _c.reset.decode()
words = {False: fail, True: good}

print("Single use barrier test (sbt) " + words[res[0]])
print("Barrier Re-use test (brt) " + words[res[1]])
print("Application test " + words[res[2]])
