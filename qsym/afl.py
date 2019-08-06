import atexit
import copy
import logging
import functools
import json
import os
import pickle
import shutil
import subprocess
import sys
import tempfile
import time

import pyinotify

from conf import SO
import executor
import minimizer
import utils

DEFAULT_TIMEOUT = 90
MAX_TIMEOUT = 10 * 60 # 10 minutes
TARGET_FILE = utils.AT_FILE

MAX_ERROR_REPORTS = 30
MAX_CRASH_REPORTS = 30

# minimum number of hang files to increase timeout
MIN_HANG_FILES = 30

logger = logging.getLogger('qsym.afl')

def get_score(testcase):
    # New coverage is the best
    score1 = testcase.endswith("+cov")
    # NOTE: seed files are not marked with "+cov"
    # even though it contains new coverage
    score2 = "orig:" in testcase
    # Smaller size is better
    score3 = -os.path.getsize(testcase)
    # Since name contains id, so later generated one will be chosen earlier
    score4 = testcase
    return (score1, score2, score3, score4)

def testcase_compare(a, b):
    a_score = get_score(a)
    b_score = get_score(b)
    return 1 if a_score > b_score else -1

def mkdir(dirp):
    if not os.path.exists(dirp):
        os.makedirs(dirp)

def check_so_file():
    for SO_file in SO.values():
        if not os.path.exists(SO_file):
            # Maybe updating now.. please wait
            logger.debug("Cannot find pintool. Maybe updating?")
            time.sleep(3 * 60)

        if not os.path.exists(SO_file):
            FATAL("Cannot find SO file!")

def get_afl_cmd(fuzzer_stats):
    with open(fuzzer_stats) as f:
        for l in f:
            if l.startswith("command_line"):
                # format= "command_line: [cmd]"
                return l.split(":")[1].strip().split()


class AFLExecutorState(object):
    def __init__(self):
        self.hang = set()
        self.processed = set()
        self.timeout = DEFAULT_TIMEOUT
        self.done = set()
        self.index = 0
        self.num_error_reports = 0
        self.num_crash_reports = 0
        self.crashes = {}

    def __setstate__(self, dict):
        self.__dict__ = dict

    def __getstate__(self):
        return self.__dict__

    def clear(self):
        self.hang = set()
        self.processed = set()

    def increase_timeout(self):
        old_timeout = self.timeout
        if self.timeout < MAX_TIMEOUT:
            self.timeout *= 2
            logger.debug("Increase timeout %d -> %d"
                         % (old_timeout, self.timeout))
        else:
            # Something bad happened, but wait until AFL resolves it
            logger.debug("Hit the maximum timeout")
            # Back to default timeout not to slow down fuzzing
            self.timeout = DEFAULT_TIMEOUT

        # sleep for a minutes to wait until AFL resolves it
        time.sleep(60)

        # clear state for restarting
        self.clear()

    def tick(self):
        old_index = self.index
        self.index += 1
        return old_index

    def get_num_processed(self):
        return len(self.processed) + len(self.hang) + len(self.done)

class AFLExecutor(object):
    def __init__(self, cmd, output, afl, name, filename=None, mail=None, asan_bin=None):
        self.cmd = cmd
        self.output = output
        self.afl = afl
        self.name = name
        self.filename = ".cur_input" if filename is None else filename
        self.mail = mail
        self.set_asan_cmd(asan_bin)

        self.tmp_dir = tempfile.mkdtemp()
        cmd, afl_path, qemu_mode = self.parse_fuzzer_stats()
        self.minimizer = minimizer.TestcaseMinimizer(
            cmd, afl_path, self.output, qemu_mode)
        self.import_state()
        self.make_dirs()
        atexit.register(self.cleanup)

    @property
    def cur_input(self):
        return os.path.realpath(os.path.join(self.my_dir, self.filename))

    @property
    def afl_dir(self):
        return os.path.join(self.output, self.afl)

    @property
    def afl_queue(self):
        return os.path.join(self.afl_dir, "queue")

    @property
    def my_dir(self):
        return os.path.join(self.output, self.name)

    @property
    def my_queue(self):
        return os.path.join(self.my_dir, "queue")

    @property
    def my_hangs(self):
        return os.path.join(self.my_dir, "hangs")

    @property
    def my_errors(self):
        return os.path.join(self.my_dir, "errors")

    @property
    def metadata(self):
        return os.path.join(self.my_dir, "metadata")

    @property
    def bitmap(self):
        return os.path.join(self.my_dir, "bitmap")

    def set_asan_cmd(self, asan_bin):
        symbolizer = ""
        for e in [
                "/usr/bin/llvm-symbolizer",
                "/usr/bin/llvm-symbolizer-3.4",
                "/usr/bin/llvm-symbolizer-3.8"]:
            if os.path.exists(e):
                symbolizer = e
                break
        os.putenv("ASAN_SYMBOLIZER_PATH", symbolizer)
        os.putenv("ASAN_OPTIONS", "symbolize=1")

        if asan_bin and os.path.exists(asan_bin):
            self.asan_cmd = [asan_bin] + self.cmd[1:]
        else:
            self.asan_cmd = None

    def make_dirs(self):
        mkdir(self.tmp_dir)
        mkdir(self.my_queue)
        mkdir(self.my_hangs)
        mkdir(self.my_errors)

    def parse_fuzzer_stats(self):
        cmd = get_afl_cmd(os.path.join(self.afl_dir, "fuzzer_stats"))
        assert cmd is not None
        index = cmd.index("--")
        return cmd[index+1:], os.path.dirname(cmd[0]), '-Q' in cmd

    def import_state(self):
        if os.path.exists(self.metadata):
            with open(self.metadata, "rb") as f:
                self.state = pickle.load(f)
        else:
            self.state = AFLExecutorState()

    def sync_files(self):
        files = []
        for name in os.listdir(self.afl_queue):
            path = os.path.join(self.afl_queue, name)
            if os.path.isfile(path):
                files.append(path)

        files = list(set(files) - self.state.done - self.state.processed)
        return sorted(files,
                      key=functools.cmp_to_key(testcase_compare),
                      reverse=True)

    def run_target(self):
        # Trigger linearlize to remove complicate expressions
        q = executor.Executor(self.cmd, self.cur_input, self.tmp_dir, bitmap=self.bitmap, argv=["-l", "1"])
        ret = q.run(self.state.timeout)
        logger.debug("Total=%d s, Emulation=%d s, Solver=%d s, Return=%d"
                     % (ret.total_time,
                        ret.emulation_time,
                        ret.solving_time,
                        ret.returncode))
        return q, ret

    def handle_by_return_code(self, res, fp):
        retcode = res.returncode
        if retcode in [124, -9]: # killed
            shutil.copy2(fp, os.path.join(self.my_hangs, os.path.basename(fp)))
            self.state.hang.add(fp)
        else:
            self.state.done.add(fp)

        # segfault or abort
        if (retcode in [128 + 11, -11, 128 + 6, -6]):
            shutil.copy2(fp, os.path.join(self.my_errors, os.path.basename(fp)))
            self.report_error(fp, res.log)

    def send_mail(self, subject, info, attach=None):
        if attach is None:
            attach = []

        cmd = ["mail"]
        for path in attach:
            cmd += ["-A", path]
        cmd += ["-s", "[qsym-report] %s" % subject]
        cmd.append(self.mail)

        info = copy.copy(info)
        info["CMD"] = " ".join(self.cmd)

        text = "\n" # skip cc
        for k, v in info.iteritems():
            text += "%s\n" % k
            text += "-" * 30 + "\n"
            text += "%s" % v + "\n" * 3
        try:
            devnull = open(os.devnull, "wb")
            proc = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=devnull, stderr=devnull)
            proc.communicate(text)
        except OSError:
            pass
        finally:
            devnull.close()

    def check_crashes(self):
        for fuzzer in os.listdir(self.output):
            crash_dir = os.path.join(self.output, fuzzer, "crashes")
            if not os.path.exists(crash_dir):
                continue

            # initialize if it's first time to see the fuzzer
            if not fuzzer in self.state.crashes:
                self.state.crashes[fuzzer] = -1

            for name in sorted(os.listdir(crash_dir)):
                # skip readme
                if name == "README.txt":
                    continue

                # read id from the format "id:000000..."
                num = int(name[3:9])
                if num > self.state.crashes[fuzzer]:
                    self.report_crash(os.path.join(crash_dir, name))
                    self.state.crashes[fuzzer] = num

    def report_error(self, fp, log):
        logger.debug("Error is occured: %s\nLog:%s" % (fp, log))
        # if no mail, then stop
        if self.mail is None:
            return

        # don't do too much
        if self.state.num_error_reports >= MAX_ERROR_REPORTS:
            return

        self.state.num_error_reports += 1
        self.send_mail("Error found", {"LOG": log}, [fp])

    def report_crash(self, fp):
        logger.debug("Crash is found: %s" % fp)

        # if no mail, then stop
        if self.mail is None:
            return

        # don't do too much
        if self.state.num_crash_reports >= MAX_CRASH_REPORTS:
            return

        self.state.num_crash_reports += 1
        info = {}
        if self.asan_cmd is not None:
            stdout, stderr = utils.run_command(
                    ["timeout", "-k", "5", "5"] + self.asan_cmd,
                    fp)
            info["STDOUT"] = stdout
            info["STDERR"] = stderr
        self.send_mail("Crash found", info, [fp])

    def export_state(self):
        with open(self.metadata, "wb") as f:
            pickle.dump(self.state, f)

    def cleanup(self):
        try:
            self.export_state()
            #shutil.rmtree(self.tmp_dir)
        except:
            pass

    def handle_empty_files(self):
        if len(self.state.hang) > MIN_HANG_FILES:
            self.state.increase_timeout()
        else:
            logger.debug("Sleep for getting files")
            time.sleep(5)

    def run(self):
        logger.debug("Temp directory=%s" % self.tmp_dir)

        while True:
            files = self.sync_files()

            if not files:
                self.handle_empty_files()
                continue

            for fp in files:
                self.run_file(fp)
                break

    def run_file(self, fp):
        check_so_file()

        # copy the test case
        shutil.copy2(fp, self.cur_input)

        old_idx = self.state.index
        logger.debug("Run qsym: input=%s" % fp)

        q, ret = self.run_target()
        self.handle_by_return_code(ret, fp)
        self.state.processed.add(fp)

        target = os.path.basename(fp)[:len("id:......")]
        num_testcase = 0
        for testcase in q.get_testcases():
            num_testcase += 1
            if not self.minimizer.check_testcase(testcase):
                # Remove if it's not interesting testcases
                os.unlink(testcase)
                continue
            index = self.state.tick()
            filename = os.path.join(
                    self.my_queue,
                    "id:%06d,src:%s" % (index, target))
            shutil.move(testcase, filename)
            logger.debug("Creating: %s" % filename)

        if os.path.exists(q.log_file):
            os.unlink(q.log_file)

        # Remove testcase_dir if it`s empty
        try:
            os.rmdir(q.testcase_directory)
        except Exception:
            pass

        logger.debug("Generate %d testcases" % num_testcase)
        logger.debug("%d testcases are new" % (self.state.index - old_idx))

        self.check_crashes()
