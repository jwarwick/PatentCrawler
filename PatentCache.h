///////////////////////////////////////////////////////////
// PatentCache.h
// jwarwick, 7 March 2004
//
// Cache to store patents instead of redownloading
//
///////////////////////////////////////////////////////////

#ifndef _PATENTCACHE_H_
#define _PATENTCACHE_H_

#include <QObject>
#include <QStringList>
#include <QHash>

#include "USPTOPatent.h"

class PatentCache : public QObject
{
  Q_OBJECT
public:
  PatentCache(QObject *parent=0);
  
  void setCachePath(QString path);
  QString getCachePath();

  void clean();

  USPTOPatent* getPatent(QString patentNumber, bool &ok);
  bool getPatentPath(QString patentNumber, QString &path);
  bool addPatent(QString patentNumber, QString patentText);

  bool getReferencedBy(const QString patentNumber, QString &path, int offset=0);
  bool addReferencedBy(const QString patentNumber, QString referencedByText, int offset=0);

  bool addRefTarget(QString targetNumber, QString refPatentNumber);
  bool getReferencedByTarget(const QString patentNumber, QString &path);

private:  
  QString getFilename(QString patentNumber);
  QString getReferencedByFilename(QString patentNumber, int offset);
  QString getReferencedByTargetFilename(QString patentNumber);
  QString m_path;

  // cached directory listing
  QStringList m_dirEntries;
  void updateDirEntries();
  QStringList dirEntries();

  // cached patent objects
  QHash<QString, USPTOPatent*> m_patentHash;
};

#endif // _PATENTCACHE_H_
