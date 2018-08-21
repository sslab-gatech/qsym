#!/usr/bin/env python2
import logging
import os
import shutil
import tempfile

from test_utils import TESTS_DIR, qsym, check_testcase

SCHEDULE_DIR = os.path.join(TESTS_DIR, "schedule")
logging.getLogger('qsym.Executor').setLevel(logging.DEBUG)

def get_testcases(exe, bitmap, input_binary):
    output_dir = tempfile.mkdtemp(prefix="qsym-")
    input_file = tempfile.NamedTemporaryFile(prefix="qsym-", delete=False).name
    new_inputs = []

    with open(input_file, "wb") as f:
        f.write(input_binary)

    try:
        q = qsym.Executor([exe], input_file, output_dir, bitmap=bitmap)
        q.run()

        for path in q.get_testcases():
            with open(path, "rb") as f:
                data = f.read()
            new_inputs.append(data)
        return new_inputs
    finally:
        shutil.rmtree(output_dir)
        os.unlink(input_file)
    return None

def get_seeds(target_dir):
    seeds = []
    inputs_dir = os.path.join(target_dir, "inputs")
    for name in os.listdir(inputs_dir):
        path = os.path.join(inputs_dir, name)
        with open(path, "rb") as f:
            data = f.read()
        seeds.append(data)
    return seeds

def get_all_testcases(target, max_iter=30):
    target_dir = os.path.join(SCHEDULE_DIR, target)
    exe = os.path.join(target_dir, "main")
    inputs = get_seeds(target_dir)
    processed = []
    bitmap = tempfile.NamedTemporaryFile(prefix="qsym-", delete=False).name

    try:
        for i in xrange(max_iter):
            if not inputs:
                break
            input_binary = inputs.pop()
            new_inputs = get_testcases(exe, bitmap, input_binary)
            assert new_inputs is not None
            inputs.extend(new_inputs)
            processed.append(input_binary)
        return processed
    finally:
        os.unlink(bitmap)

def check_testcases(exe, testcases):
    input_file = tempfile.NamedTemporaryFile(prefix="qsym-", delete=False).name

    try:
        for testcase in testcases:
            if check_testcase(exe, testcase):
                return True
    finally:
        os.unlink(input_file)

    return False

def test_dup():
    testcases = get_all_testcases("dup")
    # default + 0xdeadbeef
    assert len(testcases) == 2
