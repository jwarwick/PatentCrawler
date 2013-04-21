///////////////////////////////////////////////////////////
// SearchSetParser.h
// jwarwick, 14 March 2004
//
// Parse a search set output file
//
///////////////////////////////////////////////////////////

#include "SearchSetParser.h"

bool SearchSetContentHandler::startElement(const QString &, 
					   const QString &localName, 
					   const QString &, 
					   const QXmlAttributes &)
{
  m_currChars = "";
  
  char str[256];
  strcpy(str, localName.toLatin1());
  
  if("Outstanding" == localName){
    m_inOutstanding = true;
	m_inDownloaded = false;
	m_inNeedReferencedBy = false;
  }else if("Downloaded" == localName){
	  m_inOutstanding = false;
	m_inDownloaded = true;
	m_inNeedReferencedBy = false;
  }else if("NeedReferencedBy" == localName){
	     m_inOutstanding = false;
	m_inDownloaded = false;
	m_inNeedReferencedBy = true;
  }
  
  return(true);
}

bool SearchSetContentHandler::characters(const QString &ch)
{
  m_currChars += ch;
  return(true);
}

bool SearchSetContentHandler::endElement(const QString &, 
					 const QString &localName,
					 const QString &)
{
  char str[256];
  strcpy(str, localName.toLatin1());
  
  if("PatentNumber" == localName){
    m_currNumber = m_currChars;
  }else if("Depth" == localName){
    m_currDepth = m_currChars;
  }else if("ItemType" == localName){
	  m_currItemType = (SearchItem::ItemType)(m_currChars.toInt());
  }else if("RefPatentNumber" == localName){
	  m_currRefPatentNumber = m_currChars;
  }else if("Item" == localName){
    SearchItem *s = new SearchItem();
    Q_CHECK_PTR(s);
    s->setPatentNumber(m_currNumber);
    s->setDepth(m_currDepth.toInt());
	s->m_itemType = m_currItemType;
	s->m_refPatentNumber = m_currRefPatentNumber;
	m_currRefPatentNumber.clear();
	
	if(m_inOutstanding){
      m_outstandingItems.insert(m_currNumber, s);
    }else if(m_inDownloaded){
      m_downloadedItems.insert(m_currNumber, s);
	}else if(m_inNeedReferencedBy){
		m_needReferencedBy.insert(m_currNumber, s);
	}
  }else if("Outstanding" == localName){
    m_inOutstanding = false;
  }else if("Downloaded" == localName){
	  m_inDownloaded = false;
  }else if("NeedReferencedBy" == localName){
	  m_inNeedReferencedBy = false;
  }else if("MaxDepth" == localName){
    m_maxDepth = m_currChars;
  }else if("PatentsDay" == localName){
    m_patentsPerDay = m_currChars;
  }else if("Query" == localName){
	m_queryString = m_currChars;
  }else if("AddUSRefs" == localName){
	m_addUSRefs = m_currChars;
  }else if("AddReferencedBy" == localName){
	m_addReferencedBy = m_currChars;
  }
  
  return(true);
}


