#!/usr/bin/env python3
import os
import argparse
import sys
import subprocess
import time
import re

ROOT = os.path.abspath(os.path.dirname(__file__))
LAVA_M_DIR = os.path.join(ROOT, "LAVA-M")
AFL_FUZZ = os.path.join(ROOT, "../afl-2.52b/afl-fuzz")
QSYM = os.path.join(ROOT, "../qsym/bin/run_qsym_afl.py")

OPTS = {
    'base64': ['-d'],
    'md5sum': ['-c'],
    'who': [],
    'uniq': []
}

def sh(cmd, **kwargs):
    print("Executing: %s" % ' '.join(cmd))
    return subprocess.Popen(cmd, **kwargs)

def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument('cmd', choices=['run', 'collect'])
    p.add_argument('app', choices=OPTS.keys())
    return p.parse_args()

def get_app_dir(args):
    return os.path.join(LAVA_M_DIR, args.app)

def get_exe(args, for_afl):
    if for_afl:
        dn = "afl-lava-install"
    else:
        dn = "lava-install"

    app_dir = get_app_dir(args)
    exe = os.path.join(app_dir, "coreutils-8.24-lava-safe", dn, "bin", args.app)
    if not os.path.exists(exe):
        print("[-] Build LAVA-M")
        sys.exit(-1)
    return exe

def run(args):
    app_dir = get_app_dir(args)
    afl_exe = get_exe(args, True)
    exe = get_exe(args, False)

    output_dir = os.path.join(app_dir, "fuzzer_output")
    if os.path.exists(output_dir):
        print("[-] Output directory exists: %s" % output_dir)
        sys.exit(-1)

    input_dir = os.path.join(app_dir, "fuzzer_input")

    procs = []
    os.putenv("AFL_NO_UI", "1")

    # Run AFL master
    cmd = [AFL_FUZZ, "-M", "master",
            "-i", input_dir,
            "-o", output_dir,
            "--", afl_exe] + OPTS[args.app] + ["@@"]
    procs.append(sh(cmd))

    # Run AFL slave
    cmd = [AFL_FUZZ, "-S", "slave",
            "-i", input_dir,
            "-o", output_dir,
            "--", afl_exe] + OPTS[args.app] + ["@@"]
    procs.append(sh(cmd))

    # Run QSYM
    print('[+] Wait until AFL is initialized')
    time.sleep(5)
    cmd = [QSYM, "-a", "slave",
            "-n", "qsym",
            "-o", output_dir,
            "--", exe] + OPTS[args.app] + ["@@"]
    procs.append(sh(cmd))

    print("[+] Wait for 5 hours")

    time.sleep(5 * 60 * 60) # sleep 5 hours

    print("[+] Good... let's kill everything")
    for p in procs:
        p.terminate()
        try:
            p.wait(timeout=5)
        except subprocess.TimeoutExpired:
            p.kill()
            p.wait()

def collect(args):
    app_dir = get_app_dir(args)
    exe = get_exe(args, False)

    crash_dir = os.path.join(app_dir, "fuzzer_output/slave/crashes")
    if not os.path.exists(crash_dir):
        print("[-] Output directory does not exist: %s" % crash_dir)
        print("[-] Run the experiment first")
        sys.exit(-1)

    devnull = open("/dev/null", "wb")

    bugs = set()
    for name in sorted(os.listdir(crash_dir)):
        if not name.startswith("id:"):
            continue
        path = os.path.join(crash_dir, name)
        p = sh([exe] + OPTS[args.app] + [path], stdout=subprocess.PIPE, stderr=devnull)
        try:
            stdout, stderr = p.communicate(timeout=1)
            matches = re.findall(b"Successfully triggered bug (\d+), crashing now!", stdout)
            bugs |= set(matches)
        except subprocess.TimeoutExpired:
            continue

    founded_txt = os.path.join(app_dir, "fuzzer_output/founded.txt")
    with open(founded_txt, "wb") as f:
        for bug in sorted(bugs):
            f.write(bug)
            f.write(b"\n")

    print("[+] Found %d bugs. Check %s for found bugs" % (len(bugs), founded_txt))

def main():
    args = parse_args()
    if not os.path.exists(LAVA_M_DIR):
        print("[-] Check if you have LAVA-M")
        sys.exit(-1)

    if args.cmd == 'run':
        run(args)
    else:
        collect(args)


if __name__ == '__main__':
    main()
