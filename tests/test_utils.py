import os
import shutil
import subprocess as sp
import sys
import tempfile

MAIN = "main"
TESTS_DIR = os.path.abspath(os.path.dirname(__file__))

import qsym

def check_testcase(exe, s):
    with open(os.devnull, "wb") as dev_null:
        p = sp.Popen("timeout -k 5 %d %s" % (5,  exe), # 5 second
                stdin=sp.PIPE, stdout=sp.PIPE, stderr=dev_null, shell=True)
        stdout, stderr = p.communicate(s)
        if stdout.startswith("Good"):
            return True
        else:
            return False

def get_testcases(roots):
    testcases = []
    for root, dirs, files in os.walk(roots):
        basename = os.path.basename(root)
        if basename.startswith("_"):
            continue
        if MAIN in files:
            testcases.append(root)
    return sorted(testcases)

def run_single_test(target):
    output_dir = tempfile.mkdtemp(prefix="qsym-")
    try:
        exe = os.path.join(target, MAIN)
        assert os.path.exists(exe)

        q = qsym.Executor([exe], os.path.join(target, "input.bin"), output_dir)
        q.run(30) # 30 seconds for timeout

        for path in q.get_testcases():
            with open(path, "rb") as f:
                s = f.read()

            if check_testcase(exe, s):
                return True
        return False
    finally:
        shutil.rmtree(output_dir)
