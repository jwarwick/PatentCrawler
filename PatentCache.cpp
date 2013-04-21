///////////////////////////////////////////////////////////
// PatentCache.cpp
// jwarwick, 7 March 2004
//
// Cache to store patents instead of redownloading
//
///////////////////////////////////////////////////////////

#include "PatentCache.h"

#include <QDir>
#include <QStringlist>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

#include <QtDebug>

PatentCache::PatentCache(QObject *parent)
  : QObject(parent)
{
  m_path = ".";
  updateDirEntries();
}

void PatentCache::setCachePath(QString path)
{
  m_path = path;
  updateDirEntries();
}

QString PatentCache::getCachePath()
{
  return(m_path);
}

void PatentCache::updateDirEntries()
{
	m_dirEntries.clear();

	QDir d(m_path);
	d.setFilter(QDir::Files | QDir::Readable);
	m_dirEntries = d.entryList();
}

QStringList PatentCache::dirEntries()
{
	return m_dirEntries;
}

void PatentCache::clean()
{
  QDir d(getCachePath());
  QStringList files = d.entryList();
  QStringListIterator i(files);
  while(i.hasNext()){
	  QString filename = getCachePath() + "/" + i.next();
	  QFileInfo fileInfo(filename);
	  if(fileInfo.fileName().endsWith(".html")){
		  QFile file(filename);
		  if(file.open(QIODevice::ReadOnly)){
			  QTextStream stream(&file);
			  QString text = stream.readAll();
			  if(text.contains("<TITLE>Sorry</TITLE>")){
				  file.close();
				  
				  int rtn = QMessageBox::question(0, tr("Cache Cleaning"),
					  tr("File %1 is corrupt.  Delete?").arg(fileInfo.fileName()), QMessageBox::Yes, QMessageBox::No);
				  if(QMessageBox::Yes == rtn){
					  file.remove();
				  }
			  }else{
				  file.close();
			  }
		  }
	  }
  }

  updateDirEntries();
  QMessageBox::information(0, tr("Cache Cleaning"), tr("Cleaning complete."));
}

QString PatentCache::getFilename(QString patentNumber)
{
  QString p = patentNumber.replace(",","");
  p += ".html";
  return(p);
}

USPTOPatent* PatentCache::getPatent(QString patentNumber, bool &ok)
{
	ok = false;
	USPTOPatent *patent;
	if(m_patentHash.contains(patentNumber)){
		patent = m_patentHash.value(patentNumber);
		ok = true;
		return patent;
	}

	QString path;
	if(getPatentPath(patentNumber, path)){
		patent = new USPTOPatent(this);
		Q_CHECK_PTR(patent);
		patent->setSource(path);
		m_patentHash.insert(patentNumber, patent);
		ok = true;
		return patent;
	}

	ok = false;
	return NULL;
}

bool PatentCache::getPatentPath(QString patentNumber, QString &path)
{
  QString p = getFilename(patentNumber);
  
  //QDir d(m_path);
  //QStringList files = d.entryList();
	QStringList files = dirEntries();
  if(files.contains(p)){
    path = m_path + "/" + p;
    return(true);
  }
  return(false);
}

bool PatentCache::addPatent(QString patentNumber, QString patentText)
{
  QString patentFilename = getCachePath() + "/" + getFilename(patentNumber);

  QFile file(patentFilename);
  if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
    return false;
  }

  QTextStream out(&file);
  out << patentText << endl;
  file.close();

  updateDirEntries();

	// add to cache
	USPTOPatent *patent = new USPTOPatent(this);
	Q_CHECK_PTR(patent);
	patent->setSource(patentFilename);
	m_patentHash.insert(patentNumber, patent);

  return true;
}

QString PatentCache::getReferencedByFilename(QString patentNumber, int offset)
{
	QString p = patentNumber.replace(",","");
	if(0 != offset){
	  p += tr("-%1").arg(offset);
	}
	p += ".html";
	p = "RefBy-" + p;
	return(p);
}

bool PatentCache::getReferencedBy(const QString patentNumber, QString &path, int offset)
{
	QString p = getReferencedByFilename(patentNumber, offset);
  
	//QDir d(m_path);
	//QStringList files = d.entryList();
	QStringList files = dirEntries();
	if(files.contains(p)){
		path = m_path + "/" + p;
		return(true);
	}
	return(false);
}


bool PatentCache::addReferencedBy(const QString patentNumber, QString referencedByText, int offset)
{
	QString patentFilename = getCachePath() + "/" + getReferencedByFilename(patentNumber, offset);

	QFile file(patentFilename);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
		return false;
	}

	QTextStream out(&file);
	out << referencedByText << endl;
	file.close();

	updateDirEntries();
	return true;
}

QString PatentCache::getReferencedByTargetFilename(QString patentNumber)
{
	QString p = patentNumber.replace(",","");
	p += "-Target.html";
	p = "RefBy-" + p;
	return(p);
}

bool PatentCache::addRefTarget(QString targetNumber, QString refPatentNumber)
{
	QString patentFileName = getCachePath() + "/" + getFilename(targetNumber);
	QFile file(patentFileName);

	QString newFileName = getCachePath() + "/" + getReferencedByTargetFilename(refPatentNumber);
	bool rtn = file.copy(newFileName);
	updateDirEntries();
	return rtn;
}

bool PatentCache::getReferencedByTarget(const QString patentNumber, QString &path)
{
	QString p = getReferencedByTargetFilename(patentNumber);
  
	//QDir d(m_path);
	//QStringList files = d.entryList();
	QStringList files = dirEntries();
	if(files.contains(p)){
		path = m_path + "/" + p;
		return(true);
	}
	return(false);
}
