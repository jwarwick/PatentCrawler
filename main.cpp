#include <QApplication>

#include "MainWindow.h"

int main(int argc, char** argv)
{
  //Q_INIT_RESOURCE(application);

  QApplication app(argc, argv);

  MainWindow mainwindow;
  mainwindow.show();

  return app.exec();
}
