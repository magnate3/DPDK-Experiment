git pull origin master
cd /data1/sungho/DPDK-Experiment/seastar-memcached/seastar-version/tcp-client
SOURCE_FILES="client.cc dpdk_memcached.cc "
SOURCE_FILES+="rijndael.cc "
SOURCE_FILES+="dpdk_string.cc "

export PATH=~/data1/sungho/usr/bin:$PATH

g++ -std=gnu++11 $SOURCE_FILES -o client -lmemcached -lmemcachedutil
