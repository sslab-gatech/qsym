#!/usr/bin/env python2
import os
import logging
import subprocess

l = logging.getLogger('qsym.tests.build')

KNOWN_ERRORS = [
    "SSE4.1 instruction set not enabled",
    "SSE instruction set not enabled",
    "Only for x86_64"
]

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)

    for root, dirs, files in os.walk(os.path.dirname(os.path.realpath(__file__))):
        for dn in dirs:
            dp = os.path.join(root, dn)
            makefile = os.path.join(dp, "Makefile")
            if os.path.exists(makefile):
                pipe = subprocess.PIPE
                devnull = open(os.devnull, "wb")
                subprocess.call(["make", "clean", "-C", dp], stdout=devnull, stderr=devnull)
                p = subprocess.Popen(["make", "-C", dp], stdout=pipe, stderr=pipe)
                stdout, stderr = p.communicate()
                if p.returncode != 0:
                    # We know this error
                    if any([error in stderr for error in KNOWN_ERRORS]):
                        continue
                    l.info("dir=%s" % dp)
                    l.info("stderr=%s" % stderr)
