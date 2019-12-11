#!/bin/bash
AFL_CC=$HOME/afl-2.52b/afl-gcc
AFL_CXX=$HOME/afl-2.52b/afl-g++

if [ ! -e $AFL_CC ]; then
  echo "Build AFL first!"
  exit -1
fi

if [ -e lava_corpus ]; then
  echo "'lava_corpus' is already existed"
  exit -1
fi

wget http://panda.moyix.net/~moyix/lava_corpus.tar.xz
tar -xvf lava_corpus.tar.xz
rm lava_corpus.tar.xz
patch -p0 < LAVA.diff

pushd lava_corpus/LAVA-M
for app in base64 md5sum uniq who
  do
    pushd $app
      # build for AFL
      CC=$AFL_CC CXX=$AFL_CXX ./validate.sh
      pushd coreutils-8.24-lava-safe
      mv lava-install afl-lava-install
      popd

      # build for QSYM
      ./validate.sh
    popd
  done
popd

# XXX: Don't know why, but a binary path should be short
# So make a symbolic link
ln -s lava_corpus/LAVA-M ./LAVA-M
