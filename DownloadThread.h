#ifndef _DOWNLOADTHREAD_H_
#define _DOWNLOADTHREAD_H_

#include <QObject>
#include <QHttp>
#include <QTextStream>

class DownloadThread : public QObject
{
  Q_OBJECT
public:
  DownloadThread(QObject *parent=0);

  void downloadPatent(QString &patentNumber);
  void downloadPatentByLink(const QString &link, int depth=0);

  void downloadSearchResults(const QString &searchStr, int depth=0);
  void downloadSearchResultsByLink(const QString &link, int depth=0);

  void downloadSearchReferencedBy(const QString patentNumber);
  void downloadRefByResultsByLink(const QString patentNumber, const QString &link, int offset);

  void abortDownload();

private:
  QHttp *m_http;

  QStringList m_stateList;

  int m_getRequestId;
  int m_searchRequestId;
  int m_getReferencedByRequestId;

  bool m_doingRequest;
  bool m_wasAborted;
  
  QString m_patentNumber;
  QString m_patentLink;
  
  int m_currentSearchDepth;
  
  int downloadSearch(const QString searchString, int depth);
  
private slots:
  void handleStateChanged(int state);
  void handleRequestFinished(int id, bool error);
  
signals:
  void downloadedPatent(const QString &patentNumber, const QString &link, QString &text);
  void downloadedSearch(const QString &searchText, int depth, const QString &refPatentNumber);
  void downloadedReferencedBy(const QString &patentNumber, const QString &searchText, 
			      int offset, const QString &refPatentNumber);

  void dataReadProgress(int done, int total);
  void stateChanged(const QString &state);
  
  void errorMessage(const QString &msg);
  void statusMessage(const QString &msg);
};

#endif // _DOWNLOADTHREAD_H_
