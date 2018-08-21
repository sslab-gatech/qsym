import logging
import multiprocessing as mp
import os
import subprocess as sp
import sys
import shutil
import tempfile

import pytest
import tempfile

from test_utils import *

def pytest_generate_tests(metafunc):
    if 'target' in metafunc.fixturenames:
        testcases = get_testcases(os.path.join(TESTS_DIR, "avx2"))
        metafunc.parametrize('target', testcases)

def test_functions(target):
    logging.getLogger('qsym.Executor').setLevel(logging.DEBUG)
    # if cpu does not support avx2, then return
    if not 'avx2' in open("/proc/cpuinfo").read():
        return

    assert run_single_test(target)
