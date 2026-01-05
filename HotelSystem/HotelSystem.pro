# ==================================================
# 1. 核心模块配置
# ==================================================
QT += core gui widgets sql serialport network
DEFINES += QT_NO_SSL QMQTT_NO_SSL

TARGET = HotelSystem
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
INCLUDEPATH += $$PWD/qmqtt/src/mqtt

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    loginpage.cpp \
    maininterface.cpp \
    roomstatuspage.cpp \
    checkinpage.cpp \
    checkoutpage.cpp \
    systemsetpage.cpp \
    cardmanagedialog.cpp \
    dbmanager.cpp \
    softkeyboard.cpp \
    rfidthread.cpp \
    rc522.cpp \
    cloudmanager.cpp \
    $$PWD/qmqtt/src/mqtt/qmqtt_client.cpp \
        $$PWD/qmqtt/src/mqtt/qmqtt_frame.cpp \
        $$PWD/qmqtt/src/mqtt/qmqtt_message.cpp \
        $$PWD/qmqtt/src/mqtt/qmqtt_network.cpp \
        $$PWD/qmqtt/src/mqtt/qmqtt_socket.cpp \
        $$PWD/qmqtt/src/mqtt/qmqtt_client_p.cpp \
        $$PWD/qmqtt/src/mqtt/qmqtt_router.cpp \
        $$PWD/qmqtt/src/mqtt/qmqtt_routesubscription.cpp \
        $$PWD/qmqtt/src/mqtt/qmqtt_timer.cpp

HEADERS += \
    mainwindow.h \
    loginpage.h \
    maininterface.h \
    roomstatuspage.h \
    checkinpage.h \
    checkoutpage.h \
    systemsetpage.h \
    cardmanagedialog.h \
    dbmanager.h \
    softkeyboard.h \
    rfidthread.h \
    rc522.h \
    cloudmanager.h \
    $$PWD/qmqtt/src/mqtt/qmqtt.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_client.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_frame.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_message.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_networkinterface.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_socketinterface.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_timerinterface.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_global.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_client_p.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_message_p.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_network_p.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_socket_p.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_timer_p.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_router.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_routesubscription.h \
        $$PWD/qmqtt/src/mqtt/qmqtt_routedmessage.h
/**
    MQTT_IP:地址
    MQTT_PORT：端口号
    MQTT_USERNAME:账号
    MQTT_PASSWORD：密码
    MQTT_TOPIC:订阅
*/
DEFINES += MQTT_IP=\\\"192.168.137.1\\\" \
            MQTT_PORT=1883 \
            MQTT_USERNAME=\\\"wy\\\" \
            MQTT_PASSWORD=\\\"wy123\\\" \
            MQTT_TOPIC=\\\"topic/hotel\\\"
