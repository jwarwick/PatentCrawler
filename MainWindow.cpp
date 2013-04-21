///////////////////////////////////////////////////////////
// MainWindow.h
// jwarwick, 22 Feb 2004
//
// Main display window 
//
///////////////////////////////////////////////////////////

#include "MainWindow.h"


#include <QMessagebox>
#include <QLineedit>
#include <QFile>
#include <QCombobox>
#include <QLabel>
#include <QSettings>
#include <QTimer>
#include <QAction>
#include <QSpinbox>
#include <QApplication>
#include <QStatusBar>

#include <QtDebug>
#include <QTextStream>
#include <QFileDialog>
#include <QProgressDialog>
#include <QDateTime>
#include <QPushbutton>
#include <QMenu>
#include <QMenuBar>
#include <QCloseEvent>
#include <QPushButton>

#include "USPTOPatent.h"
#include "USPTOSearch.h"
#include "SearchSet.h"
#include "PatentInterface.h"
#include "Settings.h"

MainWindow::MainWindow(QWidget *parent, Qt::WFlags fl)
  : QMainWindow(parent, fl)
{
  setupUi(this);
  setupMenus();
  setCaption();
  
  QStatusBar *statusBar = new QStatusBar(this);
  Q_CHECK_PTR(statusBar);
  setStatusBar(statusBar);

  // restore window size and location
  QCoreApplication::setOrganizationName("Dopeaco");
  QCoreApplication::setOrganizationDomain("dopeaco.com");
  QCoreApplication::setApplicationName("PatentCrawler");
  QSettings settings;

  QPoint pos = settings.value(APP_POS_SETTING, QPoint(50, 50)).toPoint();
  QSize size = settings.value(APP_SIZE_SETTING, QSize(400, 400)).toSize();
  resize(size);
  move(pos);

  // Configuration options interface
  connect(setCacheButton, SIGNAL(clicked()),
	  this, SLOT(handleSetCache()));
  connect(patentSpinBox, SIGNAL(valueChanged(int)),
	  this, SLOT(handlePatentsPerDayChanged(int)));

  // Search settings
  m_running = false;
  connect(startButton, SIGNAL(clicked()),
	  this, SLOT(handleStartButtonClicked()));
			
  connect(searchButton, SIGNAL(clicked()),
	  this, SLOT(handleSearchButtonClicked()));

  // export settings
  connect(exportButton, SIGNAL(clicked()),
	  this, SLOT(handleExport()));
	connect(exportSpecialButton, SIGNAL(clicked()),
		this, SLOT(handleExportSpecial()));


  m_timer = new QTimer(this);
  Q_CHECK_PTR(m_timer);
  connect(m_timer, SIGNAL(timeout()),
	  this, SLOT(handleTimeout()));
  m_timer->setSingleShot(true);
  
  m_searchSet = new SearchSet(this);
  Q_CHECK_PTR(m_searchSet);

  connect(m_searchSet, SIGNAL(modified(bool)),
	  this, SLOT(setWindowModified(bool)));
  connect(m_searchSet, SIGNAL(valuesChanged(int, int, int)),
	  this, SLOT(handleSearchValuesChanged(int, int, int)));


  m_patentInterface = new PatentInterface(this);
  Q_CHECK_PTR(m_patentInterface);

  connect(m_patentInterface, SIGNAL(patentDownloaded(const QString&, const QString &)),
	  this, SLOT(handlePatentDownloaded(const QString &, const QString &)));
  connect(m_patentInterface, SIGNAL(searchDownloaded(const QString &, int, const QString &)),
	  this, SLOT(handleSearchDownloaded(const QString &, int, const QString &)));
  connect(m_patentInterface, SIGNAL(refByDownloaded(const QString &, const QString &, const QString &, int)),
	  this, SLOT(handleRefByDownloaded(const QString &, const QString &, const QString &, int)));
  
  connect(m_patentInterface, SIGNAL(statusMessage(const QString &)),
	  this, SLOT(handleStatusMessage(const QString &)));
  
  connect(m_patentInterface, SIGNAL(errorMessage(const QString &)),
	  errorLabel, SLOT(setText(const QString &)));
  connect(m_patentInterface, SIGNAL(stateChanged(const QString &)),
	  stateLabel, SLOT(setText(const QString &)));


  int value = settings.value(PATENTS_PER_DAY_SETTING, DEFAULT_PATENTS_PER_DAY).toInt();

  if(!settings.contains(MAX_PATENTS_PER_DAY_SETTING)){
    settings.setValue(MAX_PATENTS_PER_DAY_SETTING, DEFAULT_MAX_PATENTS_PER_DAY);
  }

  int maxValue = settings.value(MAX_PATENTS_PER_DAY_SETTING, DEFAULT_MAX_PATENTS_PER_DAY).toInt();
  patentSpinBox->setMaximum(maxValue);
  if(value > maxValue){
    value = maxValue;
  }
  m_patentsPerDay = value;
  updateConfigDisplay();

  m_searchSet->clear();
  setButtonState();
}


void MainWindow::setupMenus()
{
  newAction = new QAction(tr("&New"), this);
  Q_CHECK_PTR(newAction);
  newAction->setShortcut(tr("Ctrl+N"));
  newAction->setStatusTip(tr("Create a new search"));
  connect(newAction, SIGNAL(triggered()),
	  this, SLOT(handleNewSearch()));

  openAction = new QAction(tr("&Open..."), this);
  Q_CHECK_PTR(openAction);
  openAction->setShortcut(tr("Ctrl+O"));
  openAction->setStatusTip(tr("Open a saved search"));
  connect(openAction, SIGNAL(triggered()),
	  this, SLOT(handleOpenSearch()));

 cleanAction = new QAction(tr("Clean Cache"), this);
  Q_CHECK_PTR(cleanAction);
  cleanAction->setStatusTip(tr("Delete invalid cache files"));
  connect(cleanAction, SIGNAL(triggered()),
	  this, SLOT(handleCleanCache()));


  saveAction = new QAction(tr("&Save"), this);
  Q_CHECK_PTR(saveAction);
  saveAction->setShortcut(tr("Ctrl+S"));
  saveAction->setStatusTip(tr("Save this search"));
  connect(saveAction, SIGNAL(triggered()),
	  this, SLOT(handleSaveSearch()));

  saveAsAction = new QAction(tr("Save &As..."), this);
  Q_CHECK_PTR(saveAsAction);
  saveAsAction->setShortcut(tr("Ctrl+A"));
  saveAsAction->setStatusTip(tr("Save this search to a new file"));
  connect(saveAsAction, SIGNAL(triggered()),
	  this, SLOT(handleSaveSearchAs()));

  importAction = new QAction(tr("Import search page(s)..."), this);
  Q_CHECK_PTR(importAction);
  importAction->setShortcut(tr("Ctrl+I"));
  importAction->setStatusTip(tr("Import one or more html patent search result page"));
  connect(importAction, SIGNAL(triggered()),
	  this, SLOT(handleImportAction()));

  importListAction = new QAction(tr("Import patent list..."), this);
  Q_CHECK_PTR(importListAction);
  importListAction->setStatusTip(tr("Import a tab delimited list of core patents"));
  connect(importListAction, SIGNAL(triggered()),
	  this, SLOT(handleImportListAction()));

	exitAction = new QAction(tr("E&xit"), this);
	Q_CHECK_PTR(exitAction);
	exitAction->setStatusTip(tr("Quit the application"));
	connect(exitAction, SIGNAL(triggered()),
		this, SLOT(close()));

  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAction);
  fileMenu->addAction(openAction);
  fileMenu->addSeparator();
  fileMenu->addAction(cleanAction);
  fileMenu->addSeparator();
  fileMenu->addAction(saveAction);
  fileMenu->addAction(saveAsAction);
  fileMenu->addSeparator();
  fileMenu->addAction(importAction);
  fileMenu->addAction(importListAction);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);

}

void MainWindow::closeEvent(QCloseEvent *e)
{
  handleNeedsSave();

  // save window geometry settings
  QSettings settings;
  QPoint p = pos();
  if(p.x() < 0) p.setX(0);
  if(p.y() < 0) p.setY(0);
  settings.setValue(APP_POS_SETTING, p);
  settings.setValue(APP_SIZE_SETTING, size());

  e->accept();
}

void MainWindow::setCaption()
{
  QString caption = tr("PatentCrawler v%1 ").arg(VERSION_NUMBER);
  if(!m_saveFileName.isEmpty()){
    QFileInfo info(m_saveFileName);
    caption += tr("- %1").arg(info.fileName());
  }
  caption += tr("[*]");
  setWindowTitle(caption);
}

void MainWindow::handleStatusMessage(const QString &msg)
{
	statusLabel->setText(msg);
	statusLabel->setToolTip(msg);
}

void MainWindow::handleNewSearch()
{
  handleNeedsSave();
  m_saveFileName = "";
  m_searchSet->clear();
  searchEdit->clear();
  forwardReferencesCheck->setChecked(true);
  backReferencesCheck->setChecked(true);
  setCaption();
  setButtonState();
}

void MainWindow::handleOpenSearch()
{
  
  QString s = QFileDialog::getOpenFileName(this,
					   tr("Select a search file to open"),
					   lastPath(),
					   tr("Search Files (*.pts);;All Files(*)"));
  
  if(s.isEmpty()){
    return;
  }
  
  handleNeedsSave();
  m_searchSet->clear();

  m_saveFileName = s;
  setLastPath(m_saveFileName);
  m_searchSet->readSet(m_saveFileName);
  setCaption();
  setButtonState();
  searchEdit->setText(m_searchSet->queryString());
  forwardReferencesCheck->setChecked(m_searchSet->addUSRefs());
  backReferencesCheck->setChecked(m_searchSet->addReferencedBy());
}

void MainWindow::handleSaveSearch()
{
  if(m_saveFileName.isEmpty()){
    handleSaveSearchAs();
  }else{
    handleSaveSearch(m_saveFileName);
  }
}

void MainWindow::handleSaveSearchAs()
{
  QString s = QFileDialog::getSaveFileName(this,
					   "",
					   lastPath(),
					   tr("Search Files (*.pts);;All Files(*)"));
  
  if(s.isEmpty()){
    return;
  }

  if(!s.endsWith(".pts")){
    s += ".pts";
  }

  handleSaveSearch(s);
  setCaption();
}

void MainWindow::handleSaveSearch(QString filename)
{
  m_saveFileName = filename;
  setLastPath(m_saveFileName);
  m_searchSet->writeSet(m_saveFileName);
}


void MainWindow::handleNeedsSave()
{
  if(isWindowModified()){
    int rtn = QMessageBox::question(this, tr("Patent Crawler"),
				    tr("The document contains unsaved changes.\nDo you want to save them?"),
				    tr("&Save"), tr("&Discard"),
				    0,0);
    if(0 == rtn){
      handleSaveSearch();
    }
  }
}

void MainWindow::handleCleanCache()
{
  if(NULL != m_patentInterface){
    m_patentInterface->cleanCache();
  }
}

void MainWindow::handleSetCache()
{
  QString s = QFileDialog::getExistingDirectory(this,
						tr("Select a directory for patent storage"),
						lastPath());
  if(!s.isEmpty()){
    if(NULL != m_patentInterface){
      m_patentInterface->setCachePath(s);
    }
    
    QSettings settings;
    settings.setValue(CACHE_SETTING, s);
  }
  updateConfigDisplay();
}

void MainWindow::handlePatentsPerDayChanged(int value)
{
  if(value != m_patentsPerDay){
    m_patentsPerDay = value;
    
    QSettings settings;
    settings.setValue(PATENTS_PER_DAY_SETTING, value);
    
    updateConfigDisplay();
  }
}


void MainWindow::handleExport()
{
  QString s = QFileDialog::getSaveFileName(this,
					   "",
					   lastPath(),
					   tr("Tab delimited files (*.tab);;All Files(*)"));
  if(s.isEmpty()){
    return;
  }
  
  if(!s.endsWith(".tab")){
    s += ".tab";
  }

	ExportSettings settings;
	settings.inSearch = (Qt::Checked == inSearchCheck->checkState());
	settings.patentNumber = (Qt::Checked == patentNumberCheck->checkState());
    settings.issueDate = (Qt::Checked == issueDateCheck->checkState());
    settings.applicationNumber = (Qt::Checked == applicationNumberCheck->checkState());
    settings.patentTitle = (Qt::Checked == patentTitleCheck->checkState());
    settings.applicationDate = (Qt::Checked == applicationDateCheck->checkState());
    settings.applicationType = (Qt::Checked == applicationTypeCheck->checkState());
    settings.numberOfInventors = inventorNumberSpin->value();
	settings.inventorName = (Qt::Checked == inventorNameCheck->checkState());
    settings.inventorCity = (Qt::Checked == inventorCityCheck->checkState());
	settings.inventorState = (Qt::Checked == inventorStateCheck->checkState());
	settings.inventorCountry = (Qt::Checked == inventorCountryCheck->checkState());
    settings.assigneeName = (Qt::Checked == assigneeNameCheck->checkState());
    settings.assigneeCity = (Qt::Checked == assigneeCityCheck->checkState());
    settings.assigneeState = (Qt::Checked == assigneeStateCheck->checkState());
    settings.assigneeCountry = (Qt::Checked == assigneeCountryCheck->checkState());
    settings.abstract = (Qt::Checked == abstractCheck->checkState());
    settings.numberClaims = (Qt::Checked == numberClaimsCheck->checkState());
    settings.fieldOfSearch = (Qt::Checked == fieldSearchCheck->checkState());
    settings.reissueData = (Qt::Checked == reissueDataCheck->checkState());
    settings.numberOfUSClasses = usClassNumberSpin->value();
    settings.numberOfInternationalClasses = internationalClassNumberSpin->value();
    settings.governmentInterest = (Qt::Checked == governmentInterestCheck->checkState());
    settings.foreignPriority = (Qt::Checked == foreignPriorityCheck->checkState());
	settings.usReferences = (Qt::Checked == usReferencesCheck->checkState());
	settings.foreignReferences = (Qt::Checked == foreignReferencesCheck->checkState());
    settings.otherReferences = (Qt::Checked == otherReferencesCheck->checkState());
	settings.referencingPatents = (Qt::Checked == backReferencesCheck->checkState());
    settings.primaryExaminer = (Qt::Checked == primaryExaminerCheck->checkState());
    settings.assistantExaminer = (Qt::Checked == assistantExaminerCheck->checkState());
    settings.attorney = (Qt::Checked == attorneyCheck->checkState());


  QString path;
  QFile file(s);
  if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
    QTextStream stream(&file);
    
    QProgressDialog dialog(tr("Exporting patents..."), tr("Cancel"),
			   0,
			   m_searchSet->numberDownloaded(),
			   this);
    
    QStringList downloadedPatents = m_searchSet->getDownloadedItems();
    QStringListIterator downloadIterator(downloadedPatents);
    int progressValue = 0;
    while(downloadIterator.hasNext()){
      dialog.setValue(progressValue++);
      qApp->processEvents();
      
      if(dialog.wasCanceled()){
		  break;
	  }

      QString patentNumber = downloadIterator.next();
	  int depth = m_searchSet->getItemDepth(patentNumber);
      m_patentInterface->exportPatent(patentNumber, depth, settings, stream);
    }

    dialog.close();
    file.close();

    QMessageBox::information(this, tr("Done"),
			     tr("Finished exporting data"));
  }else{
    QMessageBox::warning(this, tr("Export Error"),
			 tr("Failed to open export file"));
  }
}

// format output specifically like Jaegul requires
void MainWindow::handleExportSpecial()
{
	QString s = QFileDialog::getSaveFileName(this,
						"",
						lastPath(),
						tr("Tab delimited files (*.tab);;All Files(*)"));
	if(s.isEmpty()){
		return;
	}
	  
	if(!s.endsWith(".tab")){
		s += ".tab";
	}

	QString path;
	QFile file(s);
	if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
		QTextStream stream(&file);
	    
		QStringList downloadedCorePatents = m_searchSet->getCorePatentsDownloaded();
		downloadedCorePatents.sort();

		int maxRange = downloadedCorePatents.count();
		foreach(QString patent, downloadedCorePatents){
			QStringList referencing = m_patentInterface->getUSReferences(patent);
			maxRange += referencing.count();

			QStringList refBy = m_patentInterface->getReferencedBy(patent);
			maxRange += refBy.count();
		}

		QProgressDialog dialog(tr("Exporting patents..."), tr("Cancel"),
				0,
				//m_searchSet->numberDownloaded(),
				maxRange,
				this);
	    
		// first - write the column headings
		stream << "CORE Patents\t" << "Referencing\t" << "Referenced By\t" << "CITING-FOREIGN\t"
			<< "Grant Year\t" << "Applied Year\t" << "Assignee\t" << "Assignee Type\t" 
			<< "Assignee LOCATION\t" << "Assignee Country\t" << "Inventor Name\t"
			<< "Inventor Location\t" << "Inventor Country\t" << "TECHNOLOGY CATEGORIES\t"
			<< "Tech Type\t" << "US Main Class\t" << "US Class\t" << "International\t"
			<< "Field of Search\t" << "Patent Title\t" << "TYPES OF PATENT\t"
			<< "REFERENCES\t" << "Primary Examiner\t" << endl;

		//QStringList downloadedPatents = m_searchSet->getDownloadedItems();
		int progressValue = 0;
		//int depth;
		foreach(QString patentNumber, downloadedCorePatents){
			//depth = m_searchSet->getItemDepth(patentNumber);
			
			dialog.setValue(progressValue++);
			dialog.setLabelText(tr("Exporting Patent %1").arg(patentNumber));
			qApp->processEvents();
		      
			if(dialog.wasCanceled()){
				break;
			}

			//if(0 == depth){
				m_patentInterface->exportPatentSpecialCore(patentNumber, stream);
				
				QStringList referencing = m_patentInterface->getUSReferences(patentNumber);
				referencing.sort();

				//dialog.setRange(0, dialog.maximum() + referencing.count());

				foreach(QString referencingNum, referencing){
					dialog.setValue(progressValue++);
					dialog.setLabelText(tr("Exporting  %1 Ref %2").arg(patentNumber).arg(referencingNum));
					qApp->processEvents();
					if(dialog.wasCanceled()){
						break;
					}

					m_patentInterface->exportPatentSpecialReferencing(referencingNum, stream);
				}

				QStringList refBy = m_patentInterface->getReferencedBy(patentNumber);
				refBy.sort();

				//dialog.setRange(0, dialog.maximum() + refBy.count());
				
				foreach(QString refByNum, refBy){
					dialog.setLabelText(tr("Exporting %1 RefBy %2").arg(patentNumber).arg(refByNum));
					dialog.setValue(progressValue++);
					qApp->processEvents();

					if(dialog.wasCanceled()){
						break;
					}

					m_patentInterface->exportPatentSpecialReferencedBy(refByNum, stream);
				}

				m_patentInterface->exportPatentSpecialForeignRefs(patentNumber, stream);
			}
		//}

		dialog.close();
		file.close();

		QMessageBox::information(this, tr("Done"),
					tr("Finished exporting data"));
	}else{
		QMessageBox::warning(this, tr("Export Error"),
				tr("Failed to open export file"));
	}
}

void MainWindow::handleImportAction()
{

  QStringList s = QFileDialog::getOpenFileNames(this,
						tr("Select one or more patent searches to import"),
						lastPath(),
						tr("Web Pages (*.html *.htm);;All Files (*)"));

  // make a copy, because the docs say to
  QStringList filenames = s;
  
  if(filenames.isEmpty()){
    return;
  }
  
  QStringListIterator fileIterator(filenames);
  while(fileIterator.hasNext()){
    USPTOSearch search(this);
    search.setSource(fileIterator.next());
    QStringList results = search.getResults();
	m_searchSet->addItems(results, 0);
  }
  
  setButtonState();
}

void MainWindow::handleImportListAction()
{

  QStringList s = QFileDialog::getOpenFileNames(this,
						tr("Select one or more patent lists to import"),
						lastPath(),
						tr("All Files (*)"));

  // make a copy, because the docs say to
  QStringList filenames = s;
  
  if(filenames.isEmpty()){
    return;
  }
  
  QStringListIterator fileIterator(filenames);
  while(fileIterator.hasNext()){
    QFile file(fileIterator.next());
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
      QMessageBox::warning(this, tr("Import Error"),
			   tr("Failed to open file."));
      continue;
    }

    QTextStream in(&file);
    QString contents = in.readAll();
    file.close();
    QStringList results = contents.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    if(results.isEmpty()){
      QMessageBox::warning(this, tr("Import Error"),
			   tr("Failed to find any patents listed in this file."));
      continue;
    }
    m_searchSet->addItems(results, 0);
  }
  
  updateSearchSetValues();
  setButtonState();
}

void MainWindow::handleSearchButtonClicked()
{
	QString text = searchEdit->text();
	if(text.isEmpty()){
		QMessageBox::warning(this, tr("Search Error"), tr("No search string specified."));
		return;
	}

	if(NULL != m_searchSet){
		m_searchSet->setQueryString(text);
	}
	updateSearchSetValues();

	if(NULL != m_patentInterface){
		m_patentInterface->downloadSearchResults(searchEdit->text());
	}
}


void MainWindow::handleStartButtonClicked()
{
  if(m_running){
    m_timer->stop();
    m_running = false;
  }else{
    m_running = true;
    handleTimeout();
  }

  updateSearchSetValues();
  setButtonState();
}

void MainWindow::updateSearchSetValues()
{
	if(NULL != m_searchSet){
		m_searchSet->setMaxDepth(1);
		m_searchSet->setAddReferencedBy(Qt::Checked == backReferencesCheck->checkState());
		m_searchSet->setAddUSRefs(Qt::Checked == forwardReferencesCheck->checkState());
	}
}


void MainWindow::handlePatentDownloaded(const QString &patentNumber, const QString &patentLink)
{
  if(NULL != m_searchSet){
	  if(!patentLink.isEmpty()){
		  QString refPatentNumber = m_searchSet->getRefPatentNumber(patentLink);
		  m_searchSet->gotLinkItem(patentLink, patentNumber);
		  if(!refPatentNumber.isEmpty()){
			m_patentInterface->addRefTargetToCache(patentNumber, refPatentNumber);
		  }
	  }else{
		m_searchSet->gotItem(patentNumber);
	  }
	addChildrenToSearch(patentNumber);
  }
}


void MainWindow::handleSearchDownloaded(const QString &searchText, int depth, const QString &refPatentNumber)
{
	if(NULL == m_searchSet){
		return;
	}

	USPTOSearch search;
	search.setContent(searchText);
	
	QString link;
	if(search.isPatent(link)){
		m_searchSet->addItemLink(link, depth, refPatentNumber);
		QString num = refPatentNumber;
		m_searchSet->gotReferencedBy(num);
	}else{
		QStringList results = search.getResults();
		m_searchSet->addItems(results, depth);

		// check for the next page of results
		QString nextPageLink;
		if(search.getNextResultsPageLink(nextPageLink)){
			m_patentInterface->downloadSearchResultsByLink(nextPageLink);
		}
	}

	setButtonState();
}

void MainWindow::handleRefByDownloaded(const QString &searchText, const QString &basePatentNumber, 
				       const QString &refPatentNumber, int offset)
{

	if(NULL == m_searchSet){
		return;
	}

	int refDepth = m_searchSet->getItemDepth(basePatentNumber) + 1;
	m_searchSet->gotReferencedBy(basePatentNumber);

	USPTOSearch search;
	search.setContent(searchText);
	
	QString link;
	if(search.isPatent(link)){
		m_searchSet->addItemLink(link, refDepth, refPatentNumber);
	}else{
		QStringList results = search.getResults();
		m_searchSet->addItems(results, refDepth);

		// check for the next page of results
		QString nextPageLink;
		if(search.getNextResultsPageLink(nextPageLink)){
			m_patentInterface->downloadRefByResultsByLink(basePatentNumber, nextPageLink, offset+1);
		}
	}

	setButtonState();
}


void MainWindow::handleTimeout()
{
  bool inCache = false;
  bool gotItem = false;

  QString patentNumber;
  SearchItem::ItemType itemType;
  bool loopAgain = true;
  int depth;
  do{
	  loopAgain = true;

	  // check for patents to download
	do{
		inCache = false;
		gotItem = m_searchSet->getOutStandingItem(patentNumber, itemType);
		if(gotItem && itemType == SearchItem::PATENT_ITEM){
			statusBar()->showMessage(tr("Evaluating %1").arg(patentNumber), 2000);
			inCache = m_patentInterface->patentInCache(patentNumber);
			if(inCache){
				m_searchSet->gotItem(patentNumber);
				addChildrenToSearch(patentNumber);
				qApp->processEvents();
			}
		}
	}while(inCache);
	if(gotItem){
		if(SearchItem::PATENT_ITEM == itemType){
			m_patentInterface->downloadPatent(patentNumber);
		}else if(SearchItem::PATENT_LINK == itemType){
			m_patentInterface->downloadPatentByLink(patentNumber);
		}
		loopAgain = false;
	}else{
		// no more patents to download, get referencedBy
		do{
			inCache = false;
			gotItem = m_searchSet->getNeedReferencedBy(patentNumber);
			if(gotItem){
				statusBar()->showMessage(tr("Evaluating References To %1").arg(patentNumber), 2000);
				inCache = m_patentInterface->referencedByInCache(patentNumber);
				if(inCache){
					QStringList refs = m_patentInterface->getReferencedBy(patentNumber);
					depth = m_searchSet->getItemDepth(patentNumber);
					m_searchSet->addItems(refs, depth + 1);
					m_searchSet->gotReferencedBy(patentNumber);
					qApp->processEvents();
				}
			}
		}while(inCache);
		if(gotItem){
			depth = m_searchSet->getItemDepth(patentNumber);
			m_patentInterface->downloadSearchReferencedBy(patentNumber);
			loopAgain = false;
		}
	}

	if(loopAgain){
		loopAgain = !m_searchSet->isEmpty();
	}

  }while(loopAgain);

  if(m_searchSet->isEmpty()){
	  QMessageBox::information(this, tr("PatentCrawler"),
		  tr("Finished downloading patents.  You may now export data."));
	  m_running = false;
	  setButtonState();
	  statusLabel->setText(tr("Done"));
      return;
  }
 
  int msInterval = getTimeoutInterval();
  QDateTime nextTime = QDateTime::currentDateTime().addSecs(msInterval/1000);
  nextDownloadLabel->setText(nextTime.toString());

  int numOutstanding = m_searchSet->numberOutstanding() - 1;
  QDateTime finishTime = QDateTime::currentDateTime().addSecs((msInterval/1000) * numOutstanding);
  remainingTimeLabel->setText(finishTime.toString());

  m_timer->start(msInterval);

  setButtonState();
  return;
}

void MainWindow::addChildrenToSearch(const QString patentNumber)
{
	int depth = m_searchSet->getItemDepth(patentNumber);
	if(-1 == depth){
		return;
	}
	if((depth + 1) <= m_searchSet->maxDepth()){
		if(forwardReferencesCheck->isChecked()){
			if(NULL != m_patentInterface){
				QStringList usRefs = m_patentInterface->getUSReferences(patentNumber);
				if(NULL != m_searchSet){
					m_searchSet->addItems(usRefs, depth + 1);
				}
			}
		}

		if(backReferencesCheck->isChecked()){
			if(NULL != m_patentInterface){
				m_searchSet->addNeedReferencedBy(patentNumber, depth+1);
			}
		}
	}
}

void MainWindow::handleSearchValuesChanged(int downloaded, int outstanding, int refs)
{
  totalPatentsLabel->setText(tr("%1").arg(downloaded + outstanding));
  downloadedPatentsLabel->setText(tr("%1").arg(downloaded));
  remainingPatentsLabel->setText(tr("%1").arg(outstanding));
  remainingReferencesLabel->setText(tr("%1").arg(refs));
}

void MainWindow::updateConfigDisplay()
{
  if(NULL != m_patentInterface){
    QString path = m_patentInterface->getCachePath();
	cachePathLabel->setText(path);
	cachePathLabel->setToolTip(path);
  }

  if(patentSpinBox->value() != m_patentsPerDay){
    patentSpinBox->setValue(m_patentsPerDay);
  }

  double ms = getTimeoutInterval();
  double minutes = ms/(1000.0 * 60.0);
  rateLabel->setText(tr("%1 minutes between patents").arg(minutes));
}

QString MainWindow::lastPath()
{
  QSettings settings;
  QString path = settings.value(LAST_PATH_SETTING, QDir::homePath()).toString();
  return path;
}

void MainWindow::setLastPath(QString filename)
{
  QFileInfo fileInfo(filename);
  QSettings settings;
  settings.setValue(LAST_PATH_SETTING, fileInfo.absolutePath());
}

int MainWindow::getTimeoutInterval()
{
  int msPerDay = 24 * 60 * 60 * 1000;
  int msPerPatent = msPerDay / m_patentsPerDay;
  return(msPerPatent);
}

void MainWindow::setButtonState()
{
  if(m_running){
    startButton->setText(tr("Stop"));
  }else{
    startButton->setText(tr("Start"));
  }

  if(m_searchSet->isEmpty()){
    startButton->setEnabled(false);
  }else{
    startButton->setEnabled(true);
  }

  if(m_searchSet->numberDownloaded() > 0){
    exportButton->setEnabled(true);
	exportSpecialButton->setEnabled(true);
  }else{
    exportButton->setEnabled(false);
	exportSpecialButton->setEnabled(false);
  }
}
