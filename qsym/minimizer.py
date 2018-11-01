#!/usr/bin/env python2
import atexit
import os
import subprocess as sp
import tempfile

import utils

# status for TestCaseMinimizer
NEW = 0
OLD = 1
CRASH = 2

TIMEOUT = 5 * 1000
MAP_SIZE = 65536

def read_bitmap_file(bitmap_file):
    with open(bitmap_file, "rb") as f:
        return map(ord, list(f.read()))

def write_bitmap_file(bitmap_file, bitmap):
    with open(bitmap_file, "wb") as f:
        f.write(''.join(map(chr, bitmap)))

class TestcaseMinimizer(object):
    def __init__(self, cmd, afl_path, out_dir, qemu_mode, map_size=MAP_SIZE):
        self.cmd = cmd
        self.qemu_mode = qemu_mode
        self.showmap = os.path.join(afl_path, "afl-showmap")
        self.bitmap_file = os.path.join(out_dir, "afl-bitmap")
        self.crash_bitmap_file = os.path.join(out_dir, "afl-crash-bitmap")
        _, self.temp_file = tempfile.mkstemp(dir=out_dir)
        atexit.register(self.cleanup)

        self.bitmap = self.initialize_bitmap(self.bitmap_file, map_size)
        self.crash_bitmap = self.initialize_bitmap(self.crash_bitmap_file, map_size)

    def initialize_bitmap(self, filename, map_size):
        if os.path.exists(filename):
            bitmap = read_bitmap_file(filename)
            assert len(bitmap) == map_size
        else:
            bitmap = [0] * map_size
        return bitmap

    def check_testcase(self, testcase):
        cmd = [self.showmap,
               "-t",
               str(TIMEOUT),
               "-m", "256T", # for ffmpeg
               "-b" # binary mode
        ]

        if self.qemu_mode:
            cmd += ['-Q']

        cmd += ["-o",
               self.temp_file,
               "--"
        ] + self.cmd

        cmd, stdin = utils.fix_at_file(cmd, testcase)
        with open(os.devnull, "wb") as devnull:
            proc = sp.Popen(cmd, stdin=sp.PIPE, stdout=devnull, stderr=devnull)
            proc.communicate(stdin)

        this_bitmap = read_bitmap_file(self.temp_file)
        return self.is_interesting_testcase(this_bitmap, proc.returncode)

    def is_interesting_testcase(self, bitmap, returncode):
        if returncode == 0:
            my_bitmap = self.bitmap
            my_bitmap_file = self.bitmap_file
        else:
            my_bitmap = self.crash_bitmap
            my_bitmap_file = self.crash_bitmap_file

        # Maybe need to port in C to speed up
        interesting = False
        for i in xrange(len(bitmap)):
            old = my_bitmap[i]
            new = my_bitmap[i] | bitmap[i]
            if old != new:
                interesting = True
                my_bitmap[i] = new

        if interesting:
            write_bitmap_file(my_bitmap_file, my_bitmap)
        return interesting

    def cleanup(self):
        os.unlink(self.temp_file)
