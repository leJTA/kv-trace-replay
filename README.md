# kv-trace-replay

A high-performance C++ key-value trace player with request buffering and multi-threaded workload replay.

Build :

```shell
mkdir build
cmake -S . -B build
cmake --build build -j
```

Trace replay usage :

```shell
# ./build/bin/kv_trace_replay --help

Command line options:
  --help                           print this help message
  -f [ --trace-file ] arg          trace file
  -d [ --data-file ] arg           data source file
  -h [ --host ] arg (=localhost)   host address
  -p [ --port ] arg (=9000)        host port
  -x [ --protocol ] arg (=http)    request protocol: http, tcp or console
  -t [ --threads ] arg (=4)        number of sender threads
  -b [ --buffer-size ] arg (=4096) size of the request buffer
  -o [ --output-csv ] arg          export the statistics to the given csv file
```

Trace preload usage :

```shell
# ./build/bin/kv_trace_preload --help
Command line options:
  -h [ --help ]           print this help message
  -d [ --data-file ] arg  data source file
  -f [ --trace-file ] arg trace file
  -b [ --db-path ] arg    RocksDB database path
  -v [ --verbose ]        print requests while preloading
```

## Trace format

It supports [Twitter](https://github.com/cacheMon/cache_dataset#twitter-twemcache-request-traces) trace file format.

<!-- [Meta](https://github.com/cacheMon/cache_dataset#meta-key-value-cache-traces). -->
