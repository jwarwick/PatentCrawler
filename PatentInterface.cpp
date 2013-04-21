#include "PatentInterface.h"
#include "Settings.h"
#include "DownloadThread.h"
#include "PatentCache.h"
#include "USPTOPatent.h"
#include "USPTOSearch.h"

#include <QSettings>

PatentInterface::PatentInterface(QObject *parent)
  : QObject(parent)
{

  QSettings settings;
  QString cachePath = settings.value(CACHE_SETTING, "./").toString();

  m_cache = new PatentCache(this);
  Q_CHECK_PTR(m_cache);
  m_cache->setCachePath(cachePath);

  m_downloadThread = new DownloadThread(this);
  Q_CHECK_PTR(m_downloadThread);
  connect(m_downloadThread, SIGNAL(downloadedPatent(const QString&, const QString &, QString &)),
	  this, SLOT(handleDownloadedPatent(const QString &, const QString &, QString &)));
  connect(m_downloadThread, SIGNAL(downloadedSearch(const QString &, int, const QString &)),
	  this, SLOT(handleDownloadedSearch(const QString &, int, const QString &)));
  connect(m_downloadThread, SIGNAL(downloadedReferencedBy(const QString &,const QString &, int, const QString &)),
	  this, SLOT(handleDownloadedReferencedBy(const QString &, const QString &, int, const QString &)));
  
  connect(m_downloadThread, SIGNAL(statusMessage(const QString &)),
	  this, SIGNAL(statusMessage(const QString &)));
  
  connect(m_downloadThread, SIGNAL(errorMessage(const QString &)),
	  this, SIGNAL(errorMessage(const QString &)));
  connect(m_downloadThread, SIGNAL(stateChanged(const QString &)),
	  this, SIGNAL(stateChanged(const QString &)));
  
}


void PatentInterface::setCachePath(QString &cachePath)
{
  Q_CHECK_PTR(m_cache);
  if(NULL != m_cache){
    m_cache->setCachePath(cachePath);
  }
}

QString PatentInterface::getCachePath()
{
  QString rtn;
  if(NULL != m_cache){
    rtn = m_cache->getCachePath();
  }
  return rtn;
}

void PatentInterface::cleanCache()
{
  Q_CHECK_PTR(m_cache);
  if(NULL != m_cache){
    m_cache->clean();
  }
}

bool PatentInterface::patentInCache(QString &patentNumber)
{
  QString pathInCache;
  bool inCache = m_cache->getPatentPath(patentNumber, pathInCache);
  return inCache;
}

bool PatentInterface::referencedByInCache(QString &patentNumber)
{
  QString pathInCache;
  bool inCache = m_cache->getReferencedBy(patentNumber, pathInCache);
  return inCache;
}

void PatentInterface::addRefTargetToCache(QString targetNumber, QString refPatentNumber)
{
  if(NULL != m_cache){
    m_cache->addRefTarget(targetNumber, refPatentNumber);
  }
}



void PatentInterface::exportPatent(QString &patentNumber, int depth, ExportSettings &settings, QTextStream &stream)
{
	/*
  QString patentPath;
  if(m_cache->getPatentPath(patentNumber, patentPath)){
    USPTOPatent patent(this);
    if(patent.setSource(patentPath)){
		QStringList referencedBy;
		if(referencedByInCache(patentNumber)){
			referencedBy = getReferencedBy(patentNumber);
		}
      patent.exportPatent(depth, referencedBy, settings, stream);
    }
  }
  */

	USPTOPatent *patent = NULL;
	bool ok = false;
	patent = m_cache->getPatent(patentNumber, ok);
	if(ok){
		QStringList referencedBy;	
		if(referencedByInCache(patentNumber)){
			referencedBy = getReferencedBy(patentNumber);
		}
		patent->exportPatent(depth, referencedBy, settings, stream);
	}

}

void PatentInterface::exportPatentSpecialCore(QString &patentNumber, QTextStream &stream)
{
	/*
  QString patentPath;
  if(m_cache->getPatentPath(patentNumber, patentPath)){
    USPTOPatent patent(this);
    if(patent.setSource(patentPath)){
      patent.exportPatentSpecialCore(stream);
    }
  }else{
	stream << patentNumber << "\tNot in cache" << endl;
  }
  */

	USPTOPatent *patent;
	bool ok = false;
	patent = m_cache->getPatent(patentNumber, ok);
	if(ok){
		patent->exportPatentSpecialCore(stream);
	}else{
		stream << patentNumber << "\tNot in cache" << endl;
	}

}

void PatentInterface::exportPatentSpecialReferencing(QString &patentNumber, QTextStream &stream)
{
	/*
	QString patentPath;
	if(m_cache->getPatentPath(patentNumber, patentPath)){
		USPTOPatent patent(this);
		if(patent.setSource(patentPath)){
			patent.exportPatentSpecialReferencing(stream);
		}
	}else{
		stream << "\t" << patentNumber << "\tNot in cache" << endl;
	}
	*/

	USPTOPatent *patent = NULL;
	bool ok = false;
	patent = m_cache->getPatent(patentNumber, ok);
	if(ok){
		patent->exportPatentSpecialReferencing(stream);
	}else{
		stream << "\t" << patentNumber << "\tNot in cache" << endl;
	}
}

void PatentInterface::exportPatentSpecialReferencedBy(QString &patentNumber, QTextStream &stream)
{
	/*
	QString patentPath;
	if(m_cache->getPatentPath(patentNumber, patentPath)){
		USPTOPatent patent(this);
		if(patent.setSource(patentPath)){
			patent.exportPatentSpecialReferencedBy(stream);
		}
	}else{
		stream << "\t\t" << patentNumber << "\tNot in cache" << endl;
	}
	*/

	USPTOPatent *patent = NULL;
	bool ok = false;
	patent = m_cache->getPatent(patentNumber, ok);
	if(ok){
		patent->exportPatentSpecialReferencedBy(stream);
	}else{
		stream << "\t\t" << patentNumber << "\tNot in cache" << endl;
	}
}

void PatentInterface::exportPatentSpecialForeignRefs(QString &patentNumber, QTextStream &stream)
{
	/*
  QString patentPath;
  if(m_cache->getPatentPath(patentNumber, patentPath)){
    USPTOPatent patent(this);
    if(patent.setSource(patentPath)){
      patent.exportPatentSpecialForeign(stream);
    }
  }else{
	stream << patentNumber << "\tNot in cache" << endl;
  }
  */

	USPTOPatent *patent = NULL;
	bool ok = false;
	patent = m_cache->getPatent(patentNumber, ok);
	if(ok){
		patent->exportPatentSpecialForeign(stream);
	}else{
		stream << patentNumber << "\tNot in cache" << endl;
	}
}

QStringList PatentInterface::getUSReferences(const QString patentNumber)
{
  QStringList usRefs;
  QString patentPath;
  Q_CHECK_PTR(m_cache);
  if(NULL != m_cache){
    if(m_cache->getPatentPath(patentNumber, patentPath)){
      USPTOPatent p(this);
      p.setSource(patentPath);
      
      if(p.isAvailable()){
	    usRefs = p.getUSReferences();
      }
    }
  }
  return usRefs;
}

QStringList PatentInterface::getForeignReferences(const QString patentNumber)
{
  QStringList foreignRefs;
  QString patentPath;
  Q_CHECK_PTR(m_cache);
  if(NULL != m_cache){
    if(m_cache->getPatentPath(patentNumber, patentPath)){
      USPTOPatent p(this);
      p.setSource(patentPath);
      
      if(p.isAvailable()){
	foreignRefs = p.getForeignReferences();
      }
    }
  }
	return foreignRefs;
}

QStringList PatentInterface::getReferencedBy(const QString patentNumber)
{
	QStringList refs;
	QString path;
	Q_CHECK_PTR(m_cache);
	if(NULL != m_cache){
	  if(m_cache->getReferencedBy(patentNumber, path)){
	    USPTOSearch search;
	    search.setSource(path);
	    QString url;
	    if(search.isPatent(url)){
	      QString targetPath;
	      if(m_cache->getReferencedByTarget(patentNumber, targetPath)){
		USPTOPatent patent;
		patent.setSource(targetPath);
		refs.append(patent.getPatentNumber());
	      }
	    }else{
	      refs = search.getResults();
	      QString link;
	      int offset = 1;
	      bool repeat = true;
	      while(repeat){
		repeat = search.getNextResultsPageLink(link);
		if(repeat){
		  if(!m_cache->getReferencedBy(patentNumber, path, offset++)){
		    repeat = false;
		    break;
		  }
		  search.setSource(path);
		  refs += search.getResults();
		}
	      }
	    }
	  }
	}
	return refs;
}

void PatentInterface::downloadPatent(QString &patentNumber, bool forceDownload)
{
  if(!forceDownload && patentInCache(patentNumber)){
    emit patentDownloaded(patentNumber, "");
    return;
  }	

  Q_CHECK_PTR(m_downloadThread);
  if(NULL != m_downloadThread){
    m_downloadThread->downloadPatent(patentNumber);
  }
}

void PatentInterface::downloadPatentByLink(const QString &link, int depth)
{
  Q_CHECK_PTR(m_downloadThread);
  if(NULL != m_downloadThread){
    m_downloadThread->downloadPatentByLink(link, depth);
  }
}

void PatentInterface::downloadSearchResults(const QString &searchStr, int depth)
{
  Q_CHECK_PTR(m_downloadThread);
  if(NULL != m_downloadThread){
    m_downloadThread->downloadSearchResults(searchStr, depth);
  }
}

void PatentInterface::downloadSearchResultsByLink(const QString &link, int depth)
{
  Q_CHECK_PTR(m_downloadThread);
  if(NULL != m_downloadThread){
    m_downloadThread->downloadSearchResultsByLink(link, depth);
  }
}

void PatentInterface::downloadSearchReferencedBy(const QString patentNumber)
{

  // check for cached results first
  QString path;
  if(m_cache->getReferencedBy(patentNumber, path)){
    USPTOSearch search;
    if(search.setSource(path)){
      QString text = search.getText();
      emit statusMessage(tr("Waiting"));
      emit refByDownloaded(text, patentNumber, "", 0);
      return;
    }
  }

  Q_CHECK_PTR(m_downloadThread);
  if(NULL != m_downloadThread){
    m_downloadThread->downloadSearchReferencedBy(patentNumber);
  }
}

void PatentInterface::downloadRefByResultsByLink(const QString patentNumber, const QString &link, int offset)
{
  // check for cached results first
  QString path;
  if(m_cache->getReferencedBy(patentNumber, path, offset)){
    USPTOSearch search;
    if(search.setSource(path)){
      QString text = search.getText();
      emit statusMessage(tr("Waiting"));
      emit refByDownloaded(text, patentNumber, "", offset);
      return;
    }
  }

  Q_CHECK_PTR(m_downloadThread);
  if(NULL != m_downloadThread){
    m_downloadThread->downloadRefByResultsByLink(patentNumber, link, offset);
  }
}

void PatentInterface::handleDownloadedPatent(const QString &patentNumber, 
					     const QString &link, 
					     QString &text)
{
  if(!m_cache->addPatent(patentNumber, text)){
    emit errorMessage(tr("Failed to add %1 to cache").arg(patentNumber));
  }
  
  emit patentDownloaded(patentNumber, link);
}

void PatentInterface::handleDownloadedSearch(const QString &searchText, int depth, 
					     const QString &refPatentNumber)
{
  emit searchDownloaded(searchText, depth, refPatentNumber);
}

void PatentInterface::handleDownloadedReferencedBy(const QString &patentNumber,
						   const QString &searchText, 
						   int offset, 
						   const QString &refPatentNumber)
{
  if(!m_cache->addReferencedBy(patentNumber, searchText, offset)){
    emit errorMessage(tr("Failed to add RefBy %1-%2 to cache").arg(patentNumber).arg(offset));
  }

  emit refByDownloaded(searchText, patentNumber, refPatentNumber, offset);
}
