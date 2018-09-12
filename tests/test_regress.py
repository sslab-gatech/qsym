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

logging.getLogger('qsym.Executor').setLevel(logging.DEBUG)

def test_null_deref_DependencyForest_addNode():
    target = os.path.join(TESTS_DIR, "regress/null-deref-DependencyForest-addNode")
    assert os.path.exists(target)

    output_dir = tempfile.mkdtemp(prefix="qsym-")
    try:
        exe = os.path.join(target, MAIN)
        assert os.path.exists(exe)

        q = qsym.Executor([exe], os.path.join(target, "input.bin"), output_dir,
                argv=["-l", "1"])
        res = q.run(30) # 30 seconds for timeout
        assert res.returncode == 0
    finally:
        shutil.rmtree(output_dir)
