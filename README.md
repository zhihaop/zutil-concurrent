# zutil-concurrent

A collection of Java style concurrent utils implemented in C. Including BlockingQueue, ThreadPool, CountDownLatch,
ThreadLocal, etc.

## Utils

- [ThreadLocal](include/ThreadLocal.h)
- Synchronizer
    - [ReentrantLock](include/ReentrantLock.h)
    - [Condition](include/Condition.h)
    - [CountDownLatch](include/CountDownLatch.h)
- [BlockingQueue](include/BlockingQueue.h)
    - [ArrayBlockingQueue](include/ArrayBlockingQueue.h): bounded
    - [LinkedBlockingQueue](include/LinkedBlockingQueue.h): bounded and unbounded
- [ExecutorService](include/ExecutorService.h)
    - [FixedThreadPoolExecutor](include/FixedThreadPoolExecutor.h)

## Usage

See [main.c](test/main.c).

## Performance

### source code

See [benchmarkQueue.c](test/benchmarkQueue.c)

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
- queue_size: 1024
- item_size: 8 bytes

#### ArrayBlockingQueue (offer_threads=16, poll_threads=16, hyper-threading)

| SPSC      | MPSC     | SPMC     | MPMC     |
|-----------|----------|----------|----------|
| 14.7 Mops | 4.4 Mops | 4.2 Mops | 9.4 Mops |

#### LinkedBlockingQueue (offer_threads=16, poll_threads=16, hyper-threading)

| SPSC      | MPSC     | SPMC     | MPMC     |
|-----------|----------|----------|----------|
| 20.1 Mops | 6.8 Mops | 6.7 Mops | 6.3 Mops |

#### ArrayBlockingQueue (offer_threads=8, poll_threads=8)

| SPSC      | MPSC     | SPMC     | MPMC     |
|-----------|----------|----------|----------|
| 14.8 Mops | 6.7 Mops | 7.8 Mops | 9.1 Mops |

#### LinkedBlockingQueue (offer_threads=8, poll_threads=8)

| SPSC      | MPSC     | SPMC      | MPMC     |
|-----------|----------|-----------|----------|
| 19.3 Mops | 9.9 Mops | 10.2 Mops | 7.9 Mops |
