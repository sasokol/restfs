# restfs v0.0.1
restfs provide REST API for save data.

# Requires

* cmake >=2.8
* gcc-c++ >= 4.8.5
* boost-thread >= 1.53
* boost-regex >= 1.53
* jsoncpp-devel >= 0.10.5
* elliptics-devel >= 2.26.10
* elliptics-client-devel >= 2.26.10
* fcgi-devel >= 2.4.0
* libpqxx-devel >= 4.0.1
* cryptopp-devel >= 5.6.2

# How to install

## Linux

```bash
git clone https://github.com/sasokol/restfs.git
cd restfs
mkdir build && cd build
cmake .. && make && make install
```

# How to run

```bash
restfsd &
```

It listens 127.0.0.1:9000 tcp by default. For configure see doc/conf/restfs.ini

