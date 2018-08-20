## Environment
- Tested on Ubuntun 14.04 and 16.04

## Installation

~~~~{.sh}
; install z3 and system deps
$ ./setup.sh

; install using virtual env
$ virtualenv venv
$ source venv/bin/activate
$ python setup.py install
~~~~

## Run hybrid fuzzing with AFL

~~~~{.sh}
; require to set the following environment variables
;   AFL_ROOT: afl directory (http://lcamtuf.coredump.cx/afl/)
;   INPUT: input seed files
;   OUTPUT: output directory
;   CMDLINE: command line for a testing program

; run AFL master
$ $(AFL_ROOT)/afl-fuzz -M afl-master -i $(INPUT) -o $(OUTPUT) -- $(CMDLINE)
; run AFL slave
$ $(AFL_ROOT)/afl-fuzz -S afl-slave -i $(INPUT) -o $(OUTPUT) -- $(CMDLINE)
; run QSYM
$ bin/run_qsym_afl.py -a afl-slave -o $(OUTPUT) -n qsym -- $(CMDLINE)
~~~~

## Run for testing

~~~~{.sh}
$ cd tests
$ python build.py
$ python -m pytest -n $(nproc)
~~~~

## Authors
- Insu Yun <insu@gatech.edu>
- Sangho Lee <sangho@gatech.edu>
- Meng Xu <meng.xu@gatech.edu>
- Yengjin Jang <yeongjin.jang@oregonstate.edu>
- Taesoo Kim <taesoo@gatech.edu>

## Publications
```
QSYM: A Practical Concolic Execution Engine Tailored for Hybrid Fuzzing

@inproceedings{yun:qsym,
  title        = {{QSYM: A Practical Concolic Execution Engine Tailored for Hybrid Fuzzing (to appear)}},
  author       = {Insu Yun and Sangho Lee and Meng Xu and Yeongjin Jang and Taesoo Kim},
  booktitle    = {Proceedings of the 27th USENIX Security Symposium (Security)},
  month        = aug,
  year         = 2018,
  address      = {Baltimore, MD},
}
```
