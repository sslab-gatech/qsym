import os

def find_pin():
    pin_location = "third_party/pin-2.14-71313-gcc.4.4.7-linux/pin.sh"

    # TODO: fix setup.py to get consistent path
    # if import this directory directly
    candidate1 = os.path.join(ROOT, "../", pin_location)
    if os.path.exists(candidate1):
        return candidate1

    # if installed by pip
    candidate2 = os.path.join(ROOT, "../../../../", pin_location)
    if os.path.exists(candidate2):
        return candidate2

    raise ValueError('Cannot find pin.sh')

ROOT = os.path.realpath(os.path.dirname(__file__))
PINTOOL_DIR = os.path.join(ROOT, "pintool")
SO = {
    "intel64": os.path.join(PINTOOL_DIR, "obj-intel64/libqsym.so"),
    "ia32": os.path.join(PINTOOL_DIR, "obj-ia32/libqsym.so")
}
PIN = find_pin()
