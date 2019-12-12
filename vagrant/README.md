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
# load system configuration
$ sudo sysctl --system

# compile the example
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

How to run LAVA-M
-----------------
Our VM image has pre-compiled LAVA-M applications. It is worth noting that we
used 64-bit LAVA-M applications. In 64-bit, LAVA-M's testcases fail to validate
all bugs in `uniq`, but only 20 out of 28. This is consistent with
[others](https://www.jianshu.com/p/31a048ccb2ad) (We used Google translation to
read this article).

```sh
# load system configuration
$ sudo sysctl --system

$ cd LAVA
$ ./install-lava.sh

# run experiements
$ ./run-lava.py run [app]

# collect results
$ ./run-lava.py collect [app]
```
