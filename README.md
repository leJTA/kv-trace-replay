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
  -i [ --ignore-timing ]           replay trace requests without respecting 
                                   their timestamps
  -N [ --max-requests ] arg (=-1)  maximum number of trace requests to replay 
                                   (-1 for all requests)
  -t [ --threads ] arg (=4)        number of sender threads
  -b [ --buffer-size ] arg (=4096) size of the request buffer
  -o [ --output-csv ] arg          export the statistics to the given csv file
  --cdf arg                        export the CDF to the given file
```

Trace client usage :
```shell
# ./build/bin/kv_trace_client --help

Options:
  -h [ --help ]                print this help message
  -p [ --port ] arg            listening port
  -b [ --db-path ] arg         rocksDB path
  --unique-db-key arg          specify an entry to use as the data source for 
                               all request (keys remain distinct)
  -t [ --threads ] arg (=1)    number of HTTP worker threads
  --memc-host arg (=127.0.0.1) Memcached host
  --memc-port arg (=11211)     Memcached port
```

DB preload usage :

```shell
# ./build/bin/kv_db_preload --help

Command line options:
  -h [ --help ]                   print this help message
  -d [ --data-file ] arg          data source file
  -f [ --trace-file ] arg         trace file
  -b [ --db-path ] arg            RocksDB database path
  -t [ --threads ] arg (=4)       number of writer threads
  -N [ --max-requests ] arg (=-1) maximum number of trace requests to process 
                                  (-1 for all requests)
  -S [ --value-size ] arg (=-1)   use a fixed value size in bytes for all 
                                  objects (-1 to use sizes from trace)
  -n [ --dry-run ]                perform a trial run without writing to the 
                                  database
  -v [ --verbose ]                print requests while preloading
```

## Trace format

It supports [Twitter](https://github.com/cacheMon/cache_dataset#twitter-twemcache-request-traces) trace file format.

<!-- [Meta](https://github.com/cacheMon/cache_dataset#meta-key-value-cache-traces). -->
