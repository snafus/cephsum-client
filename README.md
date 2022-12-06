# Cephsum-client
The client component of the cephsum server/client implmentation

## Usage
```
Usage: 
        cephsumclient [-h] [-p] [-h] [-s] [-d] --mode <mode> [--action <inget ] [--cksum <opts>]
```



## External dependencies
Message parsing depends on https://github.com/Tencent/rapidjson 
The License details can be found at https://github.com/Tencent/rapidjson/blob/master/license.txt 

## Installation from source
In a build directory run:
```
cmake3 ../
make
make package

sudo rpm -i cephserve-client-0.0.1-1.x86_64.rpm
```

The default destination is:
```
/usr/local/bin/cephsumclient
```
