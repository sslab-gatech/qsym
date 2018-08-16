#!/bin/bash

git submodule init
git submodule update

# install system deps
sudo apt-get update
sudo apt-get install -y libc6 libstdc++6 linux-libc-dev gcc-multilib llvm-dev g++ g++-multilib

# install z3
pushd third_party/z3
./configure
cd build
make -j$(nproc)
sudo make install
popd

echo <<EOM
Please install qsym by using (or check README.md):

  $ virtualenv venv
  $ source venv/bin/activate
  $ python setup.py install
EOM
