TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

HEADERS += \
    ./common_defs.h \
    ./Utils/parson.h \
    ./Utils/http.h \
    ./Utils/evloop.h \
    ./Utils/delay.hpp \
    ./Utils/numeric.hpp \
    ./Utils/log.h \
    ./Utils/nonce.hpp \
    ./Utils/cpuset.h \
    ./Utils/csvwriter.h \
    uart.h \
    sysconf.h \
    service.h \
    profiles.h \
    rtu.h \
    wsapi.h \
    models.h \
    persistent.h \
    baseinfo.h \
    Terminals/FC810.h \
    Terminals/FC890.h \
    collect.h \
    Terminals/HM01.h \
    mqtt.h \
    notification_service_routines.h \
    status.h \
    Terminals/FC109R.h \
    tcpclient.h \
    std_modbus_device.h \
    Terminals/H3U.h

SOURCES += \
    ./main.cpp \
    ./Utils/parson.cpp \
    ./Utils/http.cpp \
    ./Utils/evloop.cpp \
    ./Utils/log.cpp \
    ./Utils/cpuset.cpp \
    ./Utils/csvwriter.cpp \
    uart.cpp \
    sysconf.cpp \
    service.cpp \
    profiles.cpp \
    rtu.cpp \
    wsapi.cpp \
    models.cpp \
    persistent.cpp \
    baseinfo.cpp \
    Terminals/FC810.cpp \
    Terminals/FC890.cpp \
    collect.cpp \
    Terminals/HM01.cpp \
    mqtt.cpp \
    notification_service_routines.cpp \
    status.cpp \
    Terminals/FC109R.cpp \
    tcpclient.cpp \
    std_modbus_device.cpp \
    Terminals/H3U.cpp

DISTFILES += \
    ./Makefile \
    ../files/SerialRepeater \
    ../files/SerialRepeater.init

INCLUDEPATH += \
    ./ \
    ./Utils \
    ./Required/glib-2.0 \
    ./Required/glib-2.0/glib \
    ../../../../staging_dir/target-mipsel_24kec+dsp_uClibc-0.9.33.2/usr/include \
    ../../../../staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/include \
    ../../../../staging_dir/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/mipsel-openwrt-linux/include/c++/7.3.0
