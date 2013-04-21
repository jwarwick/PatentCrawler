///////////////////////////////////////////////////////////
// USPTOSearch.h
// jwarwick, 7 March 2004
//
// USPTO search result parser
//
///////////////////////////////////////////////////////////

#ifndef _USPTOSEARCH_H_
#define _USPTOSEARCH_H_

#include <QObject>

class USPTOSearch : public QObject
{
  Q_OBJECT
public:
  USPTOSearch(QObject *parent=0);
  
  bool setContent(const QString &searchText);
  bool setSource(QString filename);
  bool writeSource(QString filename);
  
  QString getText();
  
  QStringList getResults();
  bool getNextResultsPageLink(QString &link);
  
  bool isPatent(QString &link);

private:
  QString m_text;
  
  void removeTags(QString &s);
  void removeExtraWhiteSpace(QString &s);
  
};

#endif // _USPTOSEARCH_H_
