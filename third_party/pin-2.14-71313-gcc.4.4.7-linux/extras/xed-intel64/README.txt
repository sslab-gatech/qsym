Documentation:

Internal Intel: http://sde.intel.com/xed
External World: http://software.intel.com/en-us/articles/pintool-downloads

Send bugs to mark.charney@intel.com


================================================================================

If you are programming: 

decoder: look at examples/xed-ex1.cpp
encoder: look at examples/xed-ex5-enc.c  (for the high-level encoder API)

================================================================================

Examples of using hte command line tool:

% obj/xed -h
Copyright (C) 2003-2014, Intel Corporation. All rights reserved.
XED version: [6.26.0-5-gb6a5c90 2014-04-17]

Usage: obj/xed [options]
One of the following is required:
	-i input_file             (decode file)
	-ir raw_input_file        (decode a raw unformatted binary file)
	-ih hex_input_file        (decode a raw unformatted ASCII hex file)
	-d hex-string             (decode one instruction, must be last)
	-ide input_file           (decode/encode file)
	-e instruction            (encode, must be last)
	-ie file-to-assemble      (assemble the contents of the file)
	-de hex-string            (decode-then-encode, must be last)

Optional arguments:

	-v N          (0=quiet, 1=errors, 2=useful-info, 3=trace,
	               5=very verbose)
	-xv N         (XED engine verbosity, 0...99)

	-chip-check CHIP   (count instructions that are not valid for CHIP)
	-chip-check-list   (list the valid chips)

	-s section    (target section for file disassembly,
	               PECOFF and ELF formats only)

	-n N          (number of instructions to decode. Default 100M,
	               accepts K/M/G qualifiers)
 
	-perftail N   (number of instructions to skip in performance 
	               measurements. Default 0, accepts K/M/G qualifiers)
 
	-b addr       (Base address offset, for DLLs/shared libraries.
	               Use 0x for hex addresses)
	-as addr      (Address to start disassembling.
	               Use 0x for hex addresses)
	-ae addr      (Address to end   disassembling.
	               Use 0x for hex addresses)
	-no-resync    (Disable symbol-based resynchronization algorithm
	               for disassembly)
	-ast          (Show the AVX/SSE transition classfication)

	-I            (Intel syntax for disassembly)
	-A            (ATT SYSV syntax for disassembly)
	-xml          (XML formatting)
	-nwm          (Format AVX512 without curly braces for writemasks, include k0)
	-emit         (Output __emit statements for the Intel compiler)
	-dot FN       (Emit a register dependence graph file in dot format.
	               Best used with -as ADDR -ae ADDR to limit graph size.)

	-r            (for REAL_16 mode, 16b addressing (20b addresses),
	               16b default data size)
	-16           (for LEGACY_16 mode, 16b addressing,
	               16b default data size)
	-32           (for LEGACY_32 mode, 32b addressing,
	               32b default data size -- default)
	-64           (for LONG_64 mode w/64b addressing
	               Optional on windows/linux)
	-mpx          (Turn on MPX mode for disassembly, default is off)
	-s32          (32b stack addressing, default, not in LONG_64 mode)
	-s16          (16b stack addressing, not in LONG_64 mode)


================================================================================

Examples of use:

Disassemble a file 

% obj/xed -i /bin/ls  > dis.txt
% obj/xed -i foo.o    > dis.txt


Disassemble an instruction:

% obj/xed -64 -d 00 00
0000
ICLASS: ADD   CATEGORY: BINARY   EXTENSION: BASE  IFORM: ADD_MEMb_GPR8   ISA_SET: I86
SHORT: add byte ptr [rax], al


Encode an instruction (only some features available)

% obj/xed -64 -e vaddps xmm1 xmm2 xmm3
Request: VADDPS MODE:2, REG0:XMM1, REG1:XMM2, REG2:XMM3, SMODE:2
OPERAND ORDER: REG0 REG1 REG2 
Encodable! C5E858CB
.byte 0xc5,0xe8,0x58,0xcb


Decode and then re-encode an instruction:

% obj/xed -64 -de C5E858CB
C5E858CB
ICLASS: VADDPS   CATEGORY: AVX   EXTENSION: AVX  IFORM: VADDPS_XMMdq_XMMdq_XMMdq   ISA_SET: AVX
SHORT: vaddps xmm1, xmm2, xmm3
Encodable! C5E858CB
Identical re-encoding


Find out detailed information about an instruction:


% obj/xed-ex1 -64 C5E858CB
Attempting to decode: c5 e8 58 cb 
iclass VADDPS	category AVX	ISA-extension AVX	ISA-set AVX
instruction-length 4
operand-width 32
effective-operand-width 32
effective-address-width 64
stack-address-width 64
iform-enum-name VADDPS_XMMdq_XMMdq_XMMdq
iform-enum-name-dispatch (zero based) 1
iclass-max-iform-dispatch 10
Operands
#   TYPE               DETAILS        VIS  RW       OC2 BITS BYTES NELEM ELEMSZ   ELEMTYPE
#   ====               =======        ===  ==       === ==== ===== ===== ======   ========
0   REG0             REG0=XMM1   EXPLICIT   W        DQ  128    16     4     32     SINGLE
1   REG1             REG1=XMM2   EXPLICIT   R        DQ  128    16     4     32     SINGLE
2   REG2             REG2=XMM3   EXPLICIT   R        DQ  128    16     4     32     SINGLE
Memory Operands
  MemopBytes = 0
ATTRIBUTES: MXCSR 
EXCEPTION TYPE: AVX_TYPE_2
Vector length: 128  


================================================================================
