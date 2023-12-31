#!/bin/sh


echo "-- start make depend:"

SHDIR=$(dirname `readlink -f $0`)
# 脚本的执行目录在build中，所有cd到根目录
echo "make_depend.sh execute dir:" $SHDIR

OPENSSL_DIR=./openssl

CRYPTO_DIR=./crypto
CRYPTOPP_DIR=./crypto/cryptopp/
LIBEVENT_DIR=./libevent


PROTOBUF_DIR=./protobuf

BOOST_DIR=./boost

LIBFMT_DIR=./libfmt
SPDLOG_DIR=./spdlog

EVMONE_DIR=./evmone


COMPILE_NUM=`cat /proc/cpuinfo| grep  "processor" | wc -l`;

#libevent
# cd $SHDIR
# if [ -d ${LIBEVENT_DIR} ]; 
# then 
#     echo "libevent compile";
# else
#     tar zxvf ./3rd/libevent-2.1.12-stable.tar.gz;
#     mv libevent-2.1.12-stable libevent;
#     cd ${LIBEVENT_DIR} && cmake . && make -j$COMPILE_NUM;
# fi;


# openssl
cd $SHDIR
if [ -d ${OPENSSL_DIR} ];
then 
    echo "openssl compile";
else
    tar -xvf ./3rd/openssl-3.0.5.tar.gz;
    mv openssl-3.0.5 openssl;
    cd ${OPENSSL_DIR} && ./Configure && make -j$COMPILE_NUM && make install;
fi;

# cryptopp
cd $SHDIR
if [ -d ${CRYPTOPP_DIR} ]; 
then 
    echo "cryptopp compile";
else
    mkdir -p ${CRYPTO_DIR};
    unzip ./3rd/cryptopp-CRYPTOPP_8_2_0.zip -d ./;
    mv cryptopp-CRYPTOPP_8_2_0 cryptopp;
    mv cryptopp ${CRYPTO_DIR};
    cd ${CRYPTOPP_DIR} && make -j$COMPILE_NUM;
fi;


# protobuf
cd $SHDIR
if [ -d ${PROTOBUF_DIR} ]; 
then 
    echo "protobuf compile";
else
    unzip ./3rd/protobuf-cpp-3.21.9.zip -d ./;
    mv protobuf-3.21.9 protobuf;
    cd ${PROTOBUF_DIR} && ./autogen.sh && ./configure && make -j$COMPILE_NUM;
fi;

# boost
# cd $SHDIR
# if [ -d ${BOOST_DIR} ]; 
# then 
#     echo "boost ok";
# else
#     tar -zxvf  ./3rd/boost-1.72.tar.gz ./;
# fi;

# libfmt
cd $SHDIR
if [ -d ${LIBFMT_DIR} ]; \
then \
    echo "libfmt compile";\
else\
    tar -xvf ./3rd/libfmt-7.1.3.tar.gz ;\
    mv fmt-7.1.3 libfmt;\
    cd ${LIBFMT_DIR} && cmake . && make -j$COMPILE_NUM;\
fi;\

# spdlog
cd $SHDIR
if [ -d ${SPDLOG_DIR} ]; \
then \
    echo "spdlog compile";\
else\
    tar -xvf ./3rd/spdlog-1.8.2.tar.gz ;\
    mv spdlog-1.8.2 spdlog;\
    cd ${SPDLOG_DIR} && fmt_DIR=../libfmt cmake -DSPDLOG_FMT_EXTERNAL=yes . && make -j$COMPILE_NUM;\
fi;\

# evmone
# cd $SHDIR
# if [ -d ${EVMONE_DIR} ]; \
# then \
#     echo "evmone compile";\
# else\
#     tar -xvf ./3rd/evmone.tar.gz ;\
#     cd ${EVMONE_DIR} && cmake -S . -B build -DEVMONE_TESTING=ON -DBUILD_SHARED_LIBS=OFF -DEVMONE_TESTING=ON -DEVMC_INSTALL=ON && cd build && make -j$COMPILE_NUM;\
# fi;\

cd $1
echo "-- make depend done"




