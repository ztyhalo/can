#-------------------------------------------------
#
# Project created by QtCreator 2018-05-28T13:26:43
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = libcan
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

COMMLIB_PATH = /home/zty/max/commonlib/commlib/commonlib

INCLUDEPATH += .\
            $${COMMLIB_PATH}/epoll\
            $${COMMLIB_PATH}/prodata\
            $${COMMLIB_PATH}/prodata/sem\
            $${COMMLIB_PATH}/reflect\
            $${COMMLIB_PATH}/timer\
            $${COMMLIB_PATH}/zprint\
            $${COMMLIB_PATH}/mutex\
            $${COMMLIB_PATH}/sigslot\
            ./canbus



linux-g++-64{
    LIBS += $${COMMLIB_PATH}/../../build-commonlib-unknown-Debug/libcommonlib.a
}
linux-arm-gnueabi-g++{
    LIBS += $${COMMLIB_PATH}/../../build-commonlib-arm_linux-Debug/libcommonlib.a
}

SOURCES += main.cpp \
    canbus/can_bus.cpp \
    canpro/can_protocol.cpp

HEADERS += \
    canbus/can_bus.h \
    canbus/can_relate.h \
    canpro/can_protocol.h
