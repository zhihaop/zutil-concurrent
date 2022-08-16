# zutil-concurrent

A collection of Java style concurrent utils implemented in C. Including BlockingQueue, ThreadPool, etc.

## Utils

- BlockingQueue: A Java style Blocking Queue Implementation
    - ArrayBlockingQueue: bounded
    - LinkedBlockingQueue: bounded and unbounded
- ExecutorService: A Java style Executor Service Implementation
    - FixedThreadPoolExecutor: support shutdown

## Usage

See [main.c](src/main.c).

## Performance

### info

```bash
> lscpu

Architecture:            x86_64
  CPU op-mode(s):        32-bit, 64-bit
  Address sizes:         48 bits physical, 48 bits virtual
  Byte Order:            Little Endian
CPU(s):                  16
  On-line CPU(s) list:   0-15
Vendor ID:               AuthenticAMD
  Model name:            AMD Ryzen 7 5800U with Radeon Graphics
    CPU family:          25
    Model:               80
    Thread(s) per core:  2
    Core(s) per socket:  8
    Socket(s):           1
    Stepping:            0
    BogoMIPS:            3792.74
    Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse ss
                         e2 ht syscall nx mmxext fxsr_opt pdpe1gb rdtscp lm constant_tsc rep_good nopl tsc_reliable nons
                         top_tsc cpuid extd_apicid pni pclmulqdq ssse3 fma cx16 sse4_1 sse4_2 movbe popcnt aes xsave avx
                          f16c rdrand hypervisor lahf_lm cmp_legacy svm cr8_legacy abm sse4a misalignsse 3dnowprefetch o
                         svw topoext ssbd ibrs ibpb stibp vmmcall fsgsbase bmi1 avx2 smep bmi2 erms invpcid rdseed adx s
                         map clflushopt clwb sha_ni xsaveopt xsavec xgetbv1 xsaves clzero xsaveerptr arat npt nrip_save
                         tsc_scale vmcb_clean flushbyasid decodeassists pausefilter pfthreshold v_vmsave_vmload umip vae
                         s vpclmulqdq rdpid fsrm
  
  ......
  
> uname -r
5.10.102.1-microsoft-standard-WSL2

> gcc --version
gcc (Ubuntu 11.2.0-19ubuntu1) 11.2.0
Copyright (C) 2021 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

### scenarios

- **SPSC**: Single Producer Single Consumer
- **SPMC**: Single Producer Multi Consumers
- **MPSC**: Multi Producers Single Consumer
- **MPMC**: Multi Producers Multi Consumers

### throughput

- ops: r/w operations per second (offer + poll)
- queue_size: 32
- item_size: 20 bytes

#### ArrayBlockingQueue

| SPSC      | MPSC      | SPMC      | MPMC      |
|-----------|-----------|-----------|-----------|
| 25.6 Mops | 32.0 Mops | 33.6 Mops | 14.8 Mops |

#### LinkedBlockingQueue

| SPSC      | MPSC      | SPMC      | MPMC      |
|-----------|-----------|-----------|-----------|
| 48.6 Mops | 24.8 Mops | 50.0 Mops | 14.2 Mops |
