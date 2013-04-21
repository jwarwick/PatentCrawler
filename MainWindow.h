///////////////////////////////////////////////////////////
// MainWindow.h
// jwarwick, 22 Feb 2004
//
// Main display window 
//
///////////////////////////////////////////////////////////

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QFile>

#include "ui_MainWindow.h"

class QTimer;
class SearchSet;
class PatentInterface;

class MainWindow : public QMainWindow, public Ui::mainWindow
{
  Q_OBJECT
public:
  MainWindow(QWidget *parent=0, Qt::WFlags fl = Qt::Window);

  QString lastPath();
  void setLastPath(QString filename);

public slots:
  void handleSetCache();
  void handlePatentsPerDayChanged(int i);

  void handleNewSearch();
  void handleOpenSearch();
  void handleSaveSearch();
  void handleSaveSearchAs();
  void handleSaveSearch(QString filename);
  void handleImportAction();
  void handleImportListAction();
  void handleExport();
  void handleExportSpecial();
	void handleCleanCache();

  void handleSearchValuesChanged(int downloaded, int outstanding, int refs);

  void handlePatentDownloaded(const QString &patentNumber, const QString &patentLink);
  void handleSearchDownloaded(const QString &patentNumber, int depth, const QString &refPatentNumber);
  void handleRefByDownloaded(const QString &patentNumber, const QString &basePatentNumber, 
			     const QString &refPatentNumber, int offset);

  void handleStatusMessage(const QString &msg);

  void handleSearchButtonClicked();
  void handleStartButtonClicked();

  void handleTimeout();
  
protected:
  virtual void closeEvent(QCloseEvent *e);

private:
  QFile m_file;
  QTimer *m_timer;

  SearchSet *m_searchSet;
  PatentInterface *m_patentInterface;
  bool m_running;
  
  int m_patentsPerDay;

  void updateConfigDisplay();

  QString m_saveFileName;
  void handleNeedsSave();

  QMenu *fileMenu;
  QAction *newAction;
  QAction *openAction;
  QAction *cleanAction;
  QAction *saveAction;
  QAction *saveAsAction;
  QAction *importAction;
  QAction *importListAction;
  QAction *exitAction;

  int getTimeoutInterval();

  void setCaption();
  void setupMenus();
  void setButtonState();

	void addChildrenToSearch(const QString patentNumber);

	void updateSearchSetValues();
};

#endif // _MAINWINDOW_H
