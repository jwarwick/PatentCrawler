#include "DownloadThread.h"
#include "USPTOPatent.h"
#include "USPTOSearch.h"
#include "Settings.h"

#include <QtDebug>

DownloadThread::DownloadThread(QObject *parent)
  : QObject(parent)
{
  m_getRequestId = -1;
  m_searchRequestId = -1;
  m_getReferencedByRequestId = -1;
  m_currentSearchDepth = 0;
  m_doingRequest = false;
  m_wasAborted = false;

  m_http = new QHttp(PATENT_SERVER, PATENT_PORT, this);
  Q_CHECK_PTR(m_http);
  connect(m_http, SIGNAL(dataReadProgress(int, int)),
	  this, SIGNAL(dataReadProgress(int, int)));
  connect(m_http, SIGNAL(stateChanged(int)),
	  this, SLOT(handleStateChanged(int)));
  connect(m_http, SIGNAL(requestFinished(int, bool)),
	  this, SLOT(handleRequestFinished(int, bool)));

  m_stateList.append(tr("Unconnected"));
  m_stateList.append(tr("Host Lookup"));
  m_stateList.append(tr("Connecting"));
  m_stateList.append(tr("Sending")); 
  m_stateList.append(tr("Reading"));
  m_stateList.append(tr("Connected"));
  m_stateList.append(tr("Closing"));
}

void DownloadThread::downloadPatent(QString &patentNumber)
{
  if(m_doingRequest){
    emit statusMessage(tr("Still doing previous request..."));
    return;
  }

  emit statusMessage(tr("Downloading patent: %1").arg(patentNumber));
  m_patentNumber = patentNumber;
  m_patentLink.clear();

  // USPTO recommended link to a patent number - goes to a refresh page
  //QString pathStr = "/netacgi/nph-Parser?patentnumber=" + QString::number(patentNumber);
  // where the refresh page goes
  //QString pathStr = tr("/netacgi/nph-Parser?Sect1=PTO1&Sect2=HITOFF&d=PALL&p=1&u=/netahtml/srchnum.htm&r=1&f=G&l=50&s1=%1.WKU.&OS=PN/%1&RS=PN/%1").arg(patentNumber);
    // post June 2006 updated download URL
  QString pathStr = tr("/netacgi/nph-Parser?Sect1=PTO1&Sect2=HITOFF&d=PALL&p=1&u=/netahtml/PTO/srchnum.htm&r=1&f=G&l=50&s1=%1.PN.&OS=PN/%1&RS=PN/%1").arg(patentNumber);

  m_doingRequest = true;
  m_getRequestId = m_http->get(pathStr);
}

void DownloadThread::downloadPatentByLink(const QString &link, int depth)
{
	if(m_doingRequest){
		emit statusMessage(tr("Still doing previous request..."));
		return;
	}

	emit statusMessage(tr("Downloading patent (by link): %1").arg(link));
  
	m_currentSearchDepth = depth;
	m_patentLink = link;
	m_patentNumber.clear();
	m_doingRequest = true;  
	m_getRequestId = m_http->get(link);
}


void DownloadThread::downloadSearchResults(const QString &searchString, int depth)
{
  m_searchRequestId = downloadSearch(searchString, depth);
}
	
void DownloadThread::downloadSearchReferencedBy(const QString patentNumber)
{
  m_patentNumber = patentNumber;
  m_getReferencedByRequestId = downloadSearch(tr("ref/%1").arg(patentNumber), 0);
}

void DownloadThread::downloadRefByResultsByLink(const QString patentNumber, const QString &link, int offset)
{
	if(m_doingRequest){
		emit statusMessage(tr("Still doing previous request..."));
		return;
	}

	m_patentNumber = patentNumber;
	emit statusMessage(tr("Downloading RefBy (by link): %1").arg(link));
  
	m_currentSearchDepth = offset;
	m_doingRequest = true;  
	m_getReferencedByRequestId = m_http->get(link);
}

int DownloadThread::downloadSearch(const QString searchString, int depth)
{
	if(m_doingRequest){
		emit statusMessage(tr("Still doing previous request..."));
		return -1;
	}

	emit statusMessage(tr("Downloading search: %1").arg(searchString));

	m_currentSearchDepth = depth;

	// cleanup search string
	QString s = searchString.simplified();
	s = s.replace(" ", "+");
	s = s.replace("\\", "%2F");
	s = s.replace("\"", "%22");
  
	QString pathStr = tr("/netacgi/nph-Parser?Sect1=PTO2&Sect2=HITOFF&u=%2Fnetahtml%2Fsearch-adv.htm&r=0&p=1&f=S&l=50&Query=%1&d=ptxt").arg(s);
  
	m_doingRequest = true;  
	return(m_http->get(pathStr));
}


void DownloadThread::downloadSearchResultsByLink(const QString &link, int depth)
{
	if(m_doingRequest){
		emit statusMessage(tr("Still doing previous request..."));
		return;
	}

	emit statusMessage(tr("Downloading search (by link): %1").arg(link));
  
	m_currentSearchDepth = depth;
	m_doingRequest = true;  
	m_searchRequestId = m_http->get(link);
}

void DownloadThread::abortDownload()
{
  m_wasAborted = true;
  m_http->abort();
}

void DownloadThread::handleStateChanged(int state)
{
  QString newState = m_stateList.at(state);
  emit stateChanged(newState);
}

void DownloadThread::handleRequestFinished(int id, bool error)
{
 	if(id == m_getRequestId || id == m_searchRequestId || id == m_getReferencedByRequestId){
		m_doingRequest = false;
	}

	if(m_wasAborted){
		m_wasAborted = false;
		if(error){
			emit statusMessage(tr("Download canceled"));
			return;
		}
	}

	// XXX - put requests that generate an error into a separate list
	// so we don't get stuck trying to download a bad value

  if(error){
    emit errorMessage(m_http->errorString());
    return;
  }

  QHttpResponseHeader header = m_http->lastResponse();
  if(header.isValid()){
	  // HTTP status codes:
	  // 1xx - Informational
	  // 2xx - Successful
	  // 3xx - Redirection
	  // 4xx - Client Error
	  // 5xx - Server Error
	  int statusCode = header.statusCode();

	  // we expect nothing less than success...
	  if(statusCode >= 300){
		  // XXX - put these in the error list as well
		  emit errorMessage(tr("Error: %1 - %2").arg(statusCode).arg(header.reasonPhrase()));
		  return;
	  }

	  
  }

  QByteArray data = m_http->readAll();
  QString text = data;

  // check for 'Too many users' error page
  if(text.contains("<TITLE>Sorry</TITLE>")){
	emit errorMessage(tr("Too many users, request blocked"));
	return;
  }

  // no errors
  emit errorMessage(tr("None"));

  if(id == m_getRequestId){
	  if(m_patentNumber.isEmpty()){
		  USPTOPatent patent;	
		  patent.setContent(text);
		  if(!patent.isAvailable()){
			  return;
		  }
		  m_patentNumber = patent.getPatentNumber();
	  }

   emit statusMessage(tr("Waiting"));
   emit downloadedPatent(m_patentNumber, m_patentLink, text);
  
  }else if(id == m_searchRequestId){
	  emit statusMessage(tr("Waiting"));
	  emit downloadedSearch(text, m_currentSearchDepth, "");
  
  }else if(id == m_getReferencedByRequestId){
	  QString refPatentNumber = "";
	  USPTOSearch search;
	  search.setContent(text);
	  QString url;
	  if(search.isPatent(url)){
	    refPatentNumber = m_patentNumber;
	  }
	  
	  emit statusMessage(tr("Waiting"));
	  emit downloadedReferencedBy(m_patentNumber, text, m_currentSearchDepth, refPatentNumber);
  }

}
