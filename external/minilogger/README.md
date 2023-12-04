# minilogger

> A small logger with backtrace in signal handler

## Usage

* build option

Enable the following compile options if you need backtrace

```c
target_compile_options(mytarget PRIVATE -g -ggdb -gdwarf)
target_compile_options(mytarget PRIVATE -rdynamic -funwind-tables)
```

On ARM, you need an extra environment if you need backtrace

```console
UNW_ARM_UNWIND_METHOD=1 ./mytarget
```

the output will like:

```console
trace_cpp[12328]: trace_cpp version 1.1
trace_cpp[12328]: Aborting (signal 11) [./trace_cpp]
trace_cpp[12328]: ++++++++ backtrace ++++++++
trace_cpp[12328]: Frame #00: (signal_handler(int)+0x1c) [0x10d30]
trace_cpp[12328]: Frame #01: (sigset+0x14c) [0x10d30]
trace_cpp[12328]: Frame #02: (crash()+0x8) [0x10b14]
trace_cpp[12328]: Frame #03: (main+0x30) [0x109e0]
trace_cpp[12328]: Frame #04: (_fini+0xc) [0x109e0]
trace_cpp[12328]: +++++++++++++++++++++++++++
```

and also in syslog

```console
# tail -9 /var/log/messages
Mar  6 14:40:46 Puma daemon.info trace_cpp[11042]: trace_cpp version 1.1
Mar  6 14:40:46 Puma daemon.err trace_cpp[11042]: Aborting (signal 11) [./trace_cpp]
Mar  6 14:40:46 Puma daemon.err trace_cpp[11042]: ++++++++ backtrace ++++++++
Mar  6 14:40:46 Puma daemon.err trace_cpp[11042]: Frame #00: (signal_handler(int)+0x1c) [0x10d30]^M
Mar  6 14:40:46 Puma daemon.err trace_cpp[11042]: Frame #01: (sigset+0x14c) [0x10d30]^M
Mar  6 14:40:46 Puma daemon.err trace_cpp[11042]: Frame #02: (crash()+0x8) [0x10b14]^M
Mar  6 14:40:46 Puma daemon.err trace_cpp[11042]: Frame #03: (main+0x30) [0x109e0]^M
Mar  6 14:40:46 Puma daemon.err trace_cpp[11042]: Frame #04: (_fini+0xc) [0x109e0]^M
Mar  6 14:40:46 Puma daemon.err trace_cpp[11042]: +++++++++++++++++++++++++++
```
