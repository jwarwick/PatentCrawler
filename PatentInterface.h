#ifndef _PATENTINTERFACE_H_
#define _PATENTINTERFACE_H_

#include <QObject>
#include <QTextStream>

class DownloadThread;
class PatentCache;

#include "ExportSettings.h"

class PatentInterface : public QObject
{
  Q_OBJECT
public:
  PatentInterface(QObject *parent=0);

  // Cache Functions
  void setCachePath(QString &cachePath);
  QString getCachePath();
  void cleanCache();
  
  void addRefTargetToCache(QString targetNumber, QString refPatentNumber);
  
  bool patentInCache(QString &patentNumber);
  bool referencedByInCache(QString &patentNumber);

  // Download Functions
  void downloadPatent(QString &patentNumber, bool forceDownload=false);
  void downloadPatentByLink(const QString &link, int depth=0);

  void downloadSearchResults(const QString &searchStr, int depth=0);
  void downloadSearchResultsByLink(const QString &link, int depth=0);
  void downloadSearchReferencedBy(const QString patentNumber);
  void downloadRefByResultsByLink(const QString patentNumber, const QString &link, int offset);

  // Export Functions
  void exportPatent(QString &patentNumber, int depth, ExportSettings &settings, QTextStream &stream);
  
  void exportPatentSpecialCore(QString &patentNumber, QTextStream &stream);
  void exportPatentSpecialReferencing(QString &patentNumber, QTextStream &stream);
  void exportPatentSpecialReferencedBy(QString &patentNumber, QTextStream &stream);
  void exportPatentSpecialForeignRefs(QString &patentNumber, QTextStream &stream);
  
  QStringList getUSReferences(const QString patentNumber);
  QStringList getReferencedBy(const QString patentNumber);
  QStringList getForeignReferences(const QString patentNumber);

private:
  DownloadThread *m_downloadThread;
  PatentCache *m_cache;

private slots:
  void handleDownloadedPatent(const QString &patentNumber, const QString &link, QString &text);
  void handleDownloadedSearch(const QString &searchText, int depth, const QString &refPatentNumber);
  void handleDownloadedReferencedBy(const QString &patentNumber, const QString &searchText, 
				    int depth, const QString &refPatentNumber);

signals:
  void patentDownloaded(const QString &patentNumber, const QString &link);
  void searchDownloaded(const QString &searchText, int depth, const QString &refPatentNumber);
  void refByDownloaded(const QString &searchText, const QString &basePatentNumber, const QString &refPatentNumber, int offset);

  void dataReadProgress(int done, int total);
  void stateChanged(const QString &state);
  
  void errorMessage(const QString &msg);
  void statusMessage(const QString &msg);
};

#endif // _PATENTINTERFACE_H_
