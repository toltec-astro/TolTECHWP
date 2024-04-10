
# gcc 826lib_64_so.o -shared -o 826lib_64.so

from ctypes import *
import os

filename = "lib826_64.so"
xss = cdll.LoadLibrary(os.path.abspath(filename))
