TEMPLATE = app
CONFIG += debug_and_release

HEADERS += colorswatch.h mainwindow.h toolbar.h
SOURCES += colorswatch.cpp mainwindow.cpp toolbar.cpp main.cpp
build_all:!build_pass {
	CONFIG -= build_all
	CONFIG += release
}

RESOURCES += mainwindow.qrc

# install
target.path = $$[QT_INSTALL_DEMOS]/mainwindow
sources.files = $$SOURCES $$HEADERS $$FORMS $$RESOURCES *.png *.jpg *.pro
sources.path = $$[QT_INSTALL_DEMOS]/mainwindow
INSTALLS += target sources

symbian: include($$QT_SOURCE_TREE/demos/symbianpkgrules.pri)

CONFIG(debug, debug|release) {
	DESTDIR = ../bin/Debug
} else {
	DESTDIR = ../bin/Release
}
