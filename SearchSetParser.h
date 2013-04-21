///////////////////////////////////////////////////////////
// SearchSetParser.h
// jwarwick, 14 March 2004
//
// Parse a search set output file
//
///////////////////////////////////////////////////////////

#ifndef _SEARCHSETPARSER_H_
#define _SEARCHSETPARSER_H_

#include "SearchItem.h"

#include <QtXml>
#include <QMap.h>

class SearchSetContentHandler : public QXmlDefaultHandler
{
public:
  bool startElement(const QString &nameSpace, const QString &localName, 
		    const QString &qName, const QXmlAttributes &atts);
  bool endElement(const QString &nameSpace, const QString &localName,const QString &qName);
  bool characters(const QString &ch);
  
  
  QMap<QString, SearchItem*> m_downloadedItems;
  QMap<QString, SearchItem*> m_outstandingItems;
  QMap<QString, SearchItem*> m_needReferencedBy;
  QString m_maxDepth;
  QString m_patentsPerDay;
  QString m_queryString;
	QString m_addUSRefs;
	QString m_addReferencedBy;

private:
  QString m_currChars;
  QString m_currNumber;
  QString m_currDepth;
  SearchItem::ItemType m_currItemType;
  QString m_currRefPatentNumber;
  bool m_inOutstanding;
  bool m_inDownloaded;
  bool m_inNeedReferencedBy;
  
};

#endif // _SEARCHSETPARSER_H_
