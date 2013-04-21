///////////////////////////////////////////////////////////
// SearchSet.cpp
// jwarwick, 10 March 2004
//
// Storage for set of patents to download
//
///////////////////////////////////////////////////////////

#include "SearchSet.h"
#include "SearchSetParser.h"

#include <QFile>
#include <QTextStream>

#define DEFAULT_DEPTH 0

SearchSet::SearchSet(QObject *parent)
  : QObject(parent)
{
  clear();
}

void SearchSet::clear()
{
  m_queryString = "";
  m_maxDepth = DEFAULT_DEPTH;
  m_addUSRefs = true;
  m_addReferencedBy = true;
  m_downloadedItems.clear();
  m_outstandingItems.clear();
  m_needReferencedBy.clear();

  emit valuesChanged(0, 0, 0);
  emit modified(false);
}

bool SearchSet::isEmpty()
{
	return(m_outstandingItems.isEmpty() && m_needReferencedBy.isEmpty());
}


void SearchSet::setQueryString(const QString &query)
{
  if(query != m_queryString){
    m_queryString = query;
    emit modified(true);
  }
}

QString SearchSet::queryString()
{
  return m_queryString;
}

void SearchSet::setMaxDepth(int depth)
{
  if(depth != m_maxDepth){
    m_maxDepth = depth;
    emit modified(true);
  }
}

int SearchSet::maxDepth()
{
  return m_maxDepth;
}

void SearchSet::setAddUSRefs(bool add)
{
	if(add != m_addUSRefs){
		m_addUSRefs = add;
		emit modified(true);
	}
}

void SearchSet::setAddReferencedBy(bool add)
{
	if(add != m_addReferencedBy){
		m_addReferencedBy = add;
		emit modified(true);
	}
}


bool SearchSet::addUSRefs()
{
	return m_addUSRefs;
}

bool SearchSet::addReferencedBy()
{
	return m_addReferencedBy;
}


bool SearchSet::writeSet(QString filename)
{

  QFile file(filename);
  if(!file.open(QIODevice::WriteOnly)){
    return(false);
  }
  
  QDomDocument doc("SearchSet");
  QDomElement root = doc.createElement("SearchSet");
  doc.appendChild(root);

  QDomElement queryTag = doc.createElement("Query");
  root.appendChild(queryTag);
  QDomText queryText = doc.createTextNode(queryString());
  queryTag.appendChild(queryText);

  QDomElement maxDepthTag = doc.createElement("MaxDepth");
  root.appendChild(maxDepthTag);
  QDomText maxDepthText = doc.createTextNode(QString::number(maxDepth()));
  maxDepthTag.appendChild(maxDepthText);

  QDomElement addUSRefsTag = doc.createElement("AddUSRefs");
  root.appendChild(addUSRefsTag);
  QDomText addUSRefsText = doc.createTextNode(addUSRefs()?"1":"0");
  addUSRefsTag.appendChild(addUSRefsText);

  QDomElement addRefByTag = doc.createElement("AddReferencedBy");
  root.appendChild(addRefByTag);
  QDomText addRefByText = doc.createTextNode(addReferencedBy()?"1":"0");
  addRefByTag.appendChild(addRefByText);


  QDomElement outstandingTag = doc.createElement("Outstanding");
  root.appendChild(outstandingTag);
  QMapIterator<QString, SearchItem*> outStandingIterator(m_outstandingItems);
  while(outStandingIterator.hasNext()){
    outStandingIterator.next();
    outstandingTag.appendChild(outStandingIterator.value()->getDomElement(doc));
  }
  
  QDomElement needRefByTag = doc.createElement("NeedReferencedBy");
  root.appendChild(needRefByTag);
  QMapIterator<QString, SearchItem*> referencedByIterator(m_needReferencedBy);
  while(referencedByIterator.hasNext()){
	referencedByIterator.next();
	needRefByTag.appendChild(referencedByIterator.value()->getDomElement(doc));
  }


  QDomElement downloadedTag = doc.createElement("Downloaded");
  root.appendChild(downloadedTag);
  QMapIterator<QString, SearchItem*> downloadedIterator(m_downloadedItems);
  while(downloadedIterator.hasNext()){
    downloadedIterator.next();
    downloadedTag.appendChild(downloadedIterator.value()->getDomElement(doc));
  }

  QTextStream stream(&file);
  stream << doc.toString() << endl;
  file.close();
  emit modified(false);
  return(true);
}

bool SearchSet::readSet(QString filename)
{
  m_downloadedItems.clear();
  m_outstandingItems.clear();
  
  SearchSetContentHandler handler;
  QFile xmlFile(filename);
  QXmlInputSource source(&xmlFile);
  QXmlSimpleReader reader;
  reader.setContentHandler(&handler);
  reader.parse(source);
  
  m_downloadedItems = handler.m_downloadedItems;
  m_outstandingItems = handler.m_outstandingItems;
  m_needReferencedBy = handler.m_needReferencedBy;
  
  setMaxDepth(handler.m_maxDepth.toInt());
  m_queryString = handler.m_queryString;

	m_addUSRefs = (handler.m_addUSRefs == "1");
	m_addReferencedBy = (handler.m_addReferencedBy == "1");

  emit valuesChanged(m_downloadedItems.count(), m_outstandingItems.count(), m_needReferencedBy.count());
  emit modified(false);
  return(true);
}

int SearchSet::numberOutstanding()
{
  return(m_outstandingItems.count());
}

void SearchSet::addItems(QStringList patentNumberList, int depth)
{
	QStringListIterator listIterator(patentNumberList);
	while(listIterator.hasNext()){
		addItem(listIterator.next(), depth);
	}
}

void SearchSet::addItem(QString patentNumber, int depth)
{

	// unissued US patents have the form 2001/0006380
	if(patentNumber.contains("/")){
		return;
	}

  SearchItem *item;
  if(m_downloadedItems.contains(patentNumber)){
    item = m_downloadedItems.value(patentNumber);
    if(depth < item->depth()){
      item->setDepth(depth);
    }
    return;
  }
  
  if(m_outstandingItems.contains(patentNumber)){
    item = m_outstandingItems.value(patentNumber);
    if(depth < item->depth()){
      item->setDepth(depth);
    }
    return;
  }
  
  SearchItem *s = new SearchItem(this);
  Q_CHECK_PTR(s);
  s->setPatentNumber(patentNumber);
  s->m_itemType = SearchItem::PATENT_ITEM;
  s->setDepth(depth);
  m_outstandingItems.insert(patentNumber, s);
  emit valuesChanged(m_downloadedItems.count(), m_outstandingItems.count(), m_needReferencedBy.count());
  emit modified(true);
}

void SearchSet::addItemLink(QString patentLink, int depth, const QString &refPatentNumber)
{
	SearchItem *s = new SearchItem(this);
  Q_CHECK_PTR(s);
  s->setPatentNumber(patentLink);
  s->m_itemType = SearchItem::PATENT_LINK;
  s->setDepth(depth);
  s->m_refPatentNumber = refPatentNumber;
  m_outstandingItems.insert(patentLink, s);
  emit valuesChanged(m_downloadedItems.count(), m_outstandingItems.count(), m_needReferencedBy.count());
  emit modified(true);
}

void SearchSet::gotItem(QString patentNumber)
{
  SearchItem *item;
  if(0 != (item = m_outstandingItems.take(patentNumber))){
    m_downloadedItems.insert(patentNumber, item);
    emit valuesChanged(m_downloadedItems.count(), m_outstandingItems.count(), m_needReferencedBy.count());
    emit modified(true);
  }
}

void SearchSet::gotLinkItem(const QString &link, const QString &patentNumber)
{
	SearchItem *item;
	if(0 != (item = m_outstandingItems.take(link))){
		item->m_itemType = SearchItem::PATENT_ITEM;
		item->setPatentNumber(patentNumber);
		item->m_refPatentNumber.clear();
		m_downloadedItems.insert(patentNumber, item);
		emit valuesChanged(m_downloadedItems.count(), m_outstandingItems.count(), m_needReferencedBy.count());
		emit modified(true);
	}	
}

bool SearchSet::getOutStandingItem(QString &patentNumber, SearchItem::ItemType &itemType)
{
  if(m_outstandingItems.isEmpty()){
    return false;
  }
  
  // first get any patents at depth 0
  QMapIterator<QString, SearchItem*> outstandingIterator(m_outstandingItems);
  while(outstandingIterator.hasNext()){
    outstandingIterator.next();
    if(0 == outstandingIterator.value()->depth()){
      patentNumber = outstandingIterator.value()->patentNumber();
      itemType = outstandingIterator.value()->m_itemType;
      return true;
    }
  }

  // next return the first patent we find
  outstandingIterator.toFront();
  while(outstandingIterator.hasNext()){
    outstandingIterator.next();
    patentNumber = outstandingIterator.value()->patentNumber();
    itemType = outstandingIterator.value()->m_itemType;
    return true;
  }

  return false;
}

void SearchSet::addNeedReferencedBy(QString patentNumber, int depth)
{
	if(!m_needReferencedBy.contains(patentNumber)){
		SearchItem *s = new SearchItem(this);
		Q_CHECK_PTR(s);
		s->setPatentNumber(patentNumber);
		s->setDepth(depth);
		m_needReferencedBy.insert(patentNumber, s);
		emit valuesChanged(m_downloadedItems.count(), m_outstandingItems.count(), m_needReferencedBy.count());
		emit modified(true);
	}
}

bool SearchSet::getNeedReferencedBy(QString &patentNumber)
{
	if(m_needReferencedBy.isEmpty()){
		return false;
	}

	QMapIterator<QString, SearchItem*> iterator(m_needReferencedBy);
	while(iterator.hasNext()){
		iterator.next();
		patentNumber = iterator.value()->patentNumber();
		return true;
	}

	return false;
}

void SearchSet::gotReferencedBy(const QString &patentNumber)
{
	SearchItem *item;
	if(0 != (item = m_needReferencedBy.take(patentNumber))){
		emit valuesChanged(m_downloadedItems.count(), m_outstandingItems.count(), m_needReferencedBy.count());
		emit modified(true);
  }
}


int SearchSet::numberDownloaded()
{
  return m_downloadedItems.count();
}

QStringList SearchSet::getDownloadedItems()
{
  QStringList downloadedItems;
  QMapIterator<QString, SearchItem*> downloadedIterator(m_downloadedItems);
  while(downloadedIterator.hasNext()){
    downloadedIterator.next();
    downloadedItems += downloadedIterator.value()->patentNumber();
  }
  return downloadedItems;
}

int SearchSet::numberCorePatentsDownloaded()
{
	QStringList items = getCorePatentsDownloaded();
	return items.count();
}

QStringList SearchSet::getCorePatentsDownloaded()
{
  QStringList downloadedItems;
  foreach(SearchItem *item, m_downloadedItems){
	if(0 == item->depth()){
		downloadedItems += item->patentNumber();
	}
  }

  return downloadedItems;
}


int SearchSet::getItemDepth(const QString patentNumber)
{
	SearchItem *item = m_downloadedItems.value(patentNumber);
	if(NULL != item){
		return item->depth();
	}

	item = m_outstandingItems.value(patentNumber);
	if(NULL != item){
		return item->depth();
	}

	item = m_needReferencedBy.value(patentNumber);
	if(NULL != item){
		return item->depth();
	}

	return -1;
}

QString SearchSet::getRefPatentNumber(QString patentNumber)
{
	QString refPatentNumber;
	refPatentNumber.clear();

	SearchItem *item = m_outstandingItems.value(patentNumber);
	if(NULL != item){
		refPatentNumber =  item->m_refPatentNumber;
	}

	return refPatentNumber;

}
