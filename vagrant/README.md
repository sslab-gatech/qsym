Using QSYM with vagrant
=======================

Pulling from Vagrant Cloud
--------------------------
* Follow directions in https://app.vagrantup.com/jakkdu/boxes/qsym 

Building locally
----------------
```sh
$ vagrant up
$ vagrant ssh
```

How to run the example
----------------------
```sh
# set up kernel configurations
$ echo 0|sudo tee /proc/sys/kernel/yama/ptrace_scope
$ echo core|sudo tee /proc/sys/kernel/core_pattern

$ gcc -o example example.c
$ mkdir input
$ python -c'print"A"*4096' > input/seed
$ ./example ./input/seed
# nothing will print out

# concolic execution with a single test case
$ mkdir tests
$ ./qsym/bin/run_qsym.py -i input/seed -o tests -- ./example @@
$ ./example tests/qsym-last/000000
Step 1 passed

# hybrid fuzzing
$ ./afl-2.52b/afl-gcc -o example-afl example.c

# terminal 1 (using vagrant ssh)
$ ./afl-2.52b/afl-fuzz -M afl-master -i input -o output -- ./example-afl @@

# terminal 2
$ ./afl-2.52b/afl-fuzz -S afl-slave -i input -o output -- ./example-afl @@

# terminal 3
$ ./qsym/bin/run_qsym_afl.py -a afl-slave -o output -n qsym -- ./example @@

# will find a crash in minutes
```
