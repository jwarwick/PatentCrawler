///////////////////////////////////////////////////////////
// SearchItem.cpp
// jwarwick, 10 March 2004
//
// A single patent item stored in a search set
//
///////////////////////////////////////////////////////////

#include "SearchItem.h"
#include <QTextStream>

SearchItem::SearchItem(QObject *parent)
  : QObject(parent)
{
  m_depth = 5000;
  m_itemType = PATENT_ITEM;
  m_refPatentNumber = "";
}

int SearchItem::depth()
{
  return m_depth;
}

void SearchItem::setDepth(int newDepth)
{
  Q_ASSERT(newDepth <= m_depth);
  m_depth = newDepth;
}

QString SearchItem::patentNumber()
{
  return m_patentNumber;
}

void SearchItem::setPatentNumber(QString patentNumber)
{
  m_patentNumber = patentNumber;
}

QDomElement SearchItem::getDomElement(QDomDocument &doc)
{
  QDomElement tag = doc.createElement("Item");
  
  QDomElement numberTag = doc.createElement("PatentNumber");
  tag.appendChild(numberTag);
  QDomText numberText = doc.createTextNode(patentNumber());
  numberTag.appendChild(numberText);

  QDomElement itemTag = doc.createElement("ItemType");
  tag.appendChild(itemTag);
  QDomText itemText = doc.createTextNode(QString::number(m_itemType));
  itemTag.appendChild(itemText);

  QDomElement depthTag = doc.createElement("Depth");
  tag.appendChild(depthTag);
  QDomText depthText = doc.createTextNode(QString::number(depth()));
  depthTag.appendChild(depthText);

  QDomElement refTag = doc.createElement("RefPatentNumber");
  tag.appendChild(refTag);
  QDomText refText = doc.createTextNode(m_refPatentNumber);
  refTag.appendChild(refText);

  return tag;
}
