///////////////////////////////////////////////////////////
// SearchSet.h
// jwarwick, 10 March 2004
//
// Storage for set of patents to download
//
///////////////////////////////////////////////////////////

#ifndef _SEARCHSET_H_
#define _SEARCHSET_H_

#include <QObject>
#include <QMap>

#include "SearchItem.h"

class SearchSet : public QObject
{
  Q_OBJECT
public:
  SearchSet(QObject *parent=0);
  
  void clear();

  QString queryString();
  int maxDepth();
  bool addUSRefs();
  bool addReferencedBy();


  bool writeSet(QString filename);
  bool readSet(QString filename);
  
  void addItem(QString patentNumber, int depth=0);
  void addItemLink(QString patentLink, int depth, const QString &refPatentNumber);
  void addItems(QStringList patentNumberList, int depth);
  
  void gotItem(QString patentNumber);
  void gotLinkItem(const QString &link, const QString &patentNumber);
  
  void addNeedReferencedBy(QString patentNumber, int depth);
  bool getNeedReferencedBy(QString &patentNumber);
  void gotReferencedBy(const QString &patentNumber);
  
  bool getOutStandingItem(QString &patentNumber, SearchItem::ItemType &itemType);
  int numberOutstanding();
  
  int numberDownloaded();
  QStringList getDownloadedItems();

	int numberCorePatentsDownloaded();
	QStringList getCorePatentsDownloaded();

  int getItemDepth(const QString patentNumber);
  QString getRefPatentNumber(QString patentNumber);
  
  bool isEmpty();

public slots:
  void setQueryString(const QString &query);
  void setMaxDepth(int depth);
  void setAddUSRefs(bool add);
  void setAddReferencedBy(bool add);

private:
  QString m_queryString;
  int m_maxDepth;

  bool m_addUSRefs;
  bool m_addReferencedBy;

  QMap<QString, SearchItem*> m_downloadedItems;
  QMap<QString, SearchItem*> m_outstandingItems;
  QMap<QString, SearchItem*> m_needReferencedBy;

signals:
  void modified(bool modified);
  void valuesChanged(int downloaded, int outstanding, int refs);

};

#endif // _SEARCHSET_H_
