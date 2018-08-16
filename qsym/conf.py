import os

ROOT = os.path.realpath(os.path.dirname(__file__))
PIN = os.path.join(ROOT, "../third_party/pin-2.14-71313-gcc.4.4.7-linux/pin.sh")
PINTOOL_DIR = os.path.join(ROOT, "pintool")
SO = os.path.join(PINTOOL_DIR, "obj-intel64/libqsym.so")
