///////////////////////////////////////////////////////////
// SearchItem.h
// jwarwick, 10 March 2004
//
// A single patent item stored in a search set
//
///////////////////////////////////////////////////////////

#ifndef _SEARCHITEM_H_
#define _SEARCHITEM_H_

#include <QObject>
#include <QTextStream>
#include <QDomDocument>

class SearchItem : public QObject
{
  Q_OBJECT
public:
  SearchItem(QObject *parent=0);
  
  QDomElement getDomElement(QDomDocument &doc);
  
  int depth();
  void setDepth(int newDepth);


  QString patentNumber();
  void setPatentNumber(QString patentNumber);

  QString m_refPatentNumber;

  typedef enum{
	  PATENT_ITEM,
	  PATENT_LINK,
  }ItemType;

  ItemType m_itemType;

private:
  int m_depth;
  QString m_patentNumber;

};

#endif // _SEARCHITEM_H_

