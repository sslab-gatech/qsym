import copy
import logging
import json
import os
import platform
import subprocess
import time

from conf import PIN, SO
import utils

l = logging.getLogger('qsym.Executor')
US_TO_S = float(1000 ** 2)
LOG_SMT_HEADER = " [STAT] SMT:"

class ExecutorResult(object):
    def __init__(self, start_time, end_time, returncode, log):
        self.returncode = returncode
        self.total_time = end_time - start_time
        self.log = log
        self.calc_solving_time(log, end_time)

    @property
    def emulation_time(self):
        return self.total_time - self.solving_time

    def calc_solving_time(self, log, end_time):
        # This function is dependent on logging mechanism of qsym
        # So if you fix the log function, you should fix this, too
        self.solving_time = 0
        for l in reversed(log.splitlines()):
            if l.startswith(LOG_SMT_HEADER):
                obj = json.loads(l[len(LOG_SMT_HEADER):])
                assert('solving_time' in obj)
                if 'start_time' in obj:
                    # If solving started, but not terminated
                    self.solving_time = (end_time * 10 ** 6) - obj['start_time']
                self.solving_time += obj['solving_time']
                break
        self.solving_time /= US_TO_S
        # This could be happened due to incorrect measurement
        self.solving_time = min(self.solving_time, self.total_time)

class Executor(object):
    def __init__(self, cmd, input_file, output_dir,
            bitmap=None, argv=None):
        self.cmd = cmd
        self.input_file = input_file
        self.output_dir = output_dir
        self.bitmap = bitmap
        self.argv = [] if argv is None else argv

        self.testcase_dir = self.get_testcase_dir()
        self.set_opts()

    @property
    def last_testcase_dir(self):
        return os.path.join(self.output_dir, "qsym-last")

    @property
    def status_file(self):
        return os.path.join(self.output_dir, "status")

    @property
    def log_file(self):
        return os.path.join(self.testcase_dir, "pin.log")

    @property
    def testcase_directory(self):
        return self.testcase_dir
        
    def check_elf32(self):
        # assume cmd[0] is always the target binary (?)
        if os.path.exists(self.cmd[0]):
            with open(self.cmd[0]) as f:
               d = f.read(5)
               return len(d) > 4 and d[4] == chr(01)
        return False

    def gen_cmd(self, timeout):
        cmd = []
        if timeout:
            cmd += ["timeout", "-k", str(5), str(timeout)]
        cmd += [PIN]

        # Hack for 16.04
        cmd += ["-ifeellucky"]

        # Check if target is 32bit ELF
        if self.check_elf32():
            cmd += ["-t", SO["ia32"]]
        else:
            cmd += ["-t", SO["intel64"]]

        # Add log file
        cmd += ["-logfile", self.log_file]
        cmd += ["-i", self.input_file] + self.source_opts
        cmd += ["-o", self.testcase_dir]
        cmd += self.argv

        if self.bitmap:
            cmd += ["-b", self.bitmap]
        return cmd + ["--"] + self.cmd

    def run(self, timeout=None):
        cmd = self.gen_cmd(timeout)
        start_time = time.time()

        l.debug("Executing %s" % ' '.join(cmd))
        proc = subprocess.Popen(cmd, stdin=subprocess.PIPE,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = proc.communicate(self.stdin)
        end_time = time.time()
        return ExecutorResult(
                start_time,
                end_time,
                proc.returncode,
                self.read_log_file())

    def read_log_file(self):
        if os.path.exists(self.log_file):
            with open(self.log_file, "rb") as f:
                return f.read()
        else:
            return ""

    def import_status(self):
        if not os.path.exists(self.status_file):
            return 0
        else:
            with open(self.status_file, "r") as f:
                return json.load(f)

    def export_status(self, status):
        with open(self.status_file, "wb") as f:
            json.dump(status, f)

    def get_testcase_dir(self):
        status = self.import_status()
        next_testcase_dir = os.path.join(
                self.output_dir, "qsym-out-%d" % status)
        assert not os.path.exists(next_testcase_dir)
        os.mkdir(next_testcase_dir)

        # Make last_testcase_dir to point to the next_testcase_dir
        if os.path.lexists(self.last_testcase_dir):
            os.remove(self.last_testcase_dir)
        os.symlink(os.path.abspath(next_testcase_dir), self.last_testcase_dir)

        # Update status file to point next_testcase_dir
        status += 1
        self.export_status(status)
        return next_testcase_dir

    def set_opts(self):
        self.cmd, self.stdin = utils.fix_at_file(self.cmd, self.input_file)
        if self.stdin is None:
            self.source_opts = ["-f", "1"]
        else:
            self.source_opts = ["-s", "1"]

    def get_testcases(self):
        for name in sorted(os.listdir(self.testcase_dir)):
            if name == "stat":
                continue
            if name == "pin.log":
                continue
            path = os.path.join(self.testcase_dir, name)
            yield path
