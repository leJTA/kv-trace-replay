# kv-trace-replay

A high-performance C++ key-value trace player with request buffering and multi-threaded workload replay.

Build :

```shell
mkdir build
cmake -S . -B build
cmake --build build -j
```

Usage :

```shell
# ./build/bin/kv_trace_replay --help

Command line options:
  --help                         print this help message
  -f [ --trace-file ] arg        trace file
  -d [ --data-file ] arg         data source file
  -h [ --host ] arg (=localhost) host address
  -p [ --port ] arg (=9000)      host port
  -t [ --threads ] arg (=4)      number of worker threads
  --buffer-size arg (=4096)      size of the request buffer
```

## Trace format

It supports [Twitter](https://github.com/cacheMon/cache_dataset#twitter-twemcache-request-traces) trace file format.

<!-- [Meta](https://github.com/cacheMon/cache_dataset#meta-key-value-cache-traces). -->
