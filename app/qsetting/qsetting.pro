
QT      += widgets multimedia

TARGET = qsetting
TEMPLATE = app

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        qtaudio.cpp \
        qtbt.cpp \
        qtfactory.cpp \
        qtinputdialog.cpp \
        qtkeyboard.cpp \
        qtupdate.cpp \
        qtwifi.cpp \

HEADERS += \
        mainwindow.h \
        qtaudio.h \
        qtbt.h \
        qtfactory.h \
        qtinputdialog.h \
        qtkeyboard.h \
        qtupdate.h \
        qtwifi.h \

FORMS += \
    qtkeyboard.ui \

LIBS += -lDeviceIo

RESOURCES += res.qrc
