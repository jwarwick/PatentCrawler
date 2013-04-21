
QT += xml network
CONFIG += qt warn_on thread

TEMPLATE = app
DESTDIR = ./bin

HEADERS += MainWindow.h \
           USPTOPatent.h \
           USPTOSearch.h \
           PatentCache.h \
           SearchItem.h \
           SearchSet.h \
           SearchSetParser.h \
           DownloadThread.h \
           PatentInterface.h \

SOURCES += main.cpp \
           MainWindow.cpp \
           USPTOPatent.cpp \
           USPTOSearch.cpp \
           PatentCache.cpp \
           SearchItem.cpp \
           SearchSet.cpp \
           SearchSetParser.cpp \
           DownloadThread.cpp \
           PatentInterface.cpp \

FORMS += MainWindow.ui

macx{
QMAKE_COPY = /usr/bin/install -p
QMAKE_COPY_FILE = /usr/bin/install -p
}

build_debug{
message("Configuring for debug")
CONFIG+= debug
}

build_install{
message("Configuring for install")
INSTALLS += target headers
target.path = ./Release
headers.path = ./Release

# create Frameworks directory in bundle
adddir.path = .
adddir.extra = install -d $$target.path/PatentCrawler.app/Contents/Frameworks
INSTALLS += adddir

# insert qt lib
addqt.path = .
addqt.extra = install -p /usr/local/qt/lib/libqt-mt.3.dylib $$target.path/PatentCrawler.app/Contents/Frameworks; install_name_tool -change libqt-mt.3.dylib @executable_path/../Frameworks/libqt-mt.3.dylib $$target.path/PatentCrawler.app/Contents/MacOS/PatentCrawler
INSTALLS += addqt

}


