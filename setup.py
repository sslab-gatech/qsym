#!/usr/bin/env python2
import os
import subprocess
import multiprocessing as mp
try:
    from setuptools import setup
    from setuptools import find_packages
    packages = find_packages()
except ImportError:
    from distutils.core import setup
    packages = [x.strip('./').replace('/','.') for x in os.popen('find -name "__init__.py" | xargs -n1 dirname').read().strip().split('\n')]

from distutils.core import setup
from distutils.command.build import build as DistutilsBuild

def build_pintool():
    if subprocess.call(["make", "-C", "qsym/pintool", "-j", str(mp.cpu_count())]) != 0:
        raise ValueError("Unable to build pintool")
    my_env = os.environ.copy()
    my_env["TARGET"] = "ia32"
    if subprocess.call(["make", "-C", "qsym/pintool", "-j", str(mp.cpu_count())], env=my_env) != 0:
        raise ValueError("Unable to build pintool")
    if int(open("/proc/sys/kernel/yama/ptrace_scope").read()) != 0:
        raise ValueError("Please disable yama/ptrace_scope:\n" \
                       + "echo 0 > /proc/sys/kernel/yama/ptrace_scope")

class MyBuild(DistutilsBuild):
    def run(self):
        self.execute(build_pintool, (), msg='Building pintool')
        DistutilsBuild.run(self)

data_files = []
def get_pin():
    for path, _, files in os.walk("third_party/pin-2.14-71313-gcc.4.4.7-linux"):
        paths = [os.path.join(path, f) for f in files]
        if paths:
            data_files.append((path, paths))
    return data_files

get_pin()
setup(name='qsym',
      version='0.1',
      description='Concolic execution based fuzzer',
      cmdclass={'build':MyBuild},
      packages=packages,
      include_package_data=True,
      package_data
        = {"qsym": ["pintool/obj-intel64/libqsym.so","pintool/obj-ia32/libqsym.so"]},
      install_requires=[
          'termcolor',          # for qsym/utils.py
          'pyinotify',          # for qsym/afl.py (XXX. doesn't seem to be using?)
          'pytest-xdist',             # for unit testing
      ],
      data_files=data_files
)
