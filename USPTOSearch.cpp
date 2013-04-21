
///////////////////////////////////////////////////////////
// USPTOSearch.cpp
// jwarwick, 7 March 2004
//
// USPTO search result parser 
//
///////////////////////////////////////////////////////////

#include <QFile>
#include <QRegexp>
#include <QTextStream>
#include <QStringList>

#include <QtDebug>

#include "USPTOSearch.h"
#include "USPTOPatent.h"

USPTOSearch::USPTOSearch(QObject *parent)
  : QObject(parent)
{
}

bool USPTOSearch::setContent(const QString &searchText)
{
	m_text = searchText;
	return true;
}

bool USPTOSearch::setSource(QString filename)
{
  QFile file(filename);
  if(file.open(QIODevice::ReadOnly)){
    QTextStream stream(&file);
    m_text = stream.readAll();
    file.close();
    return(true);
  }else{
    return(false);
  }
}

bool USPTOSearch::writeSource(QString filename)
{
  QFile file(filename);
  if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
    QTextStream stream(&file);
    stream << m_text;
    file.close();
    return(true);
  }else{
    return(false);
  }
}

void USPTOSearch::removeTags(QString &s)
{
  QRegExp tag("<.+>");
  tag.setMinimal(true);
  s.replace(tag, "");
}

void USPTOSearch::removeExtraWhiteSpace(QString &s)
{
  s = s.simplified();
}

QString USPTOSearch::getText()
{
  QString text = m_text;
  
  QRegExp rx("<!--.+-->");
  rx.setMinimal(true);
  text.replace(rx, "");
  
  QRegExp img("<img.*>");
  img.setCaseSensitivity(Qt::CaseInsensitive);
  img.setMinimal(true);
  text.replace(img, "");
  
  return(text);
}


bool USPTOSearch::isPatent(QString &link)
{
	QString text = getText();
	if(!text.contains("<TITLE>Single Document</TITLE>")){
		return false;
	}

	/*<HTML>
<HEAD>
<TITLE>Single Document</TITLE>
<META HTTP-EQUIV="REFRESH" CONTENT="1;URL=/netacgi/nph-Parser?Sect1=PTO2&Sect2=HITOFF&u=/netahtml/search-adv.htm&r=1&p=1&f=G&l=50&d=ptxt&S1=(('color+kinetics'.ASNM.)+AND+warwick.INZZ.)&OS=an/"color+kinetics"+and+in/warwick&RS=(AN/"color+kinetics"+AND+IN/warwick)">
</HEAD>
</HTML>
	*/
	
	QString regexp = "URL=(.*)\">";
	QRegExp nextListRx(regexp, Qt::CaseInsensitive);
	nextListRx.setMinimal(true);

	int pos = -1;
	if(-1 != (pos = nextListRx.indexIn(text))){
		link = nextListRx.cap(1);	
		return true;
	}

	link.clear();
	return false;
}

QStringList USPTOSearch::getResults()
{
  QStringList patents;
  QRegExp tableRx("<TABLE><TR><TD></TD><TD>PAT. NO.</TD><TD></TD><TD>Title</TD></TR>(.*)</TABLE>");
  tableRx.setMinimal(true);
  int pos = tableRx.indexIn(m_text);
  if(pos > 0){

    QString table = tableRx.cap(1);
    removeExtraWhiteSpace(table);
    
    // <TR><TD valign=top>1</TD>
    // <TD valign=top><A  HREF=/netacgi/nph-Parser?Sect1=PTO2&Sect2=HITOFF&u=/netahtml/search-adv.htm&r=1&p=1&f=G&l=50&d=ptxt&S1=Schempf.INZZ.&OS=IN/Schempf&RS=IN/Schempf> 6,675,888</A></TD>
    // <TD valign=baseline><IMG border=0 src="/netaicon/PTO/ftext.gif" alt="Full-Text"></TD>
    // <TD valign=top><A  HREF=/netacgi/nph-Parser?Sect1=PTO2&Sect2=HITOFF&u=/netahtml/search-adv.htm&r=1&p=1&f=G&l=50&d=ptxt&S1=Schempf.INZZ.&OS=IN/Schempf&RS=IN/Schempf> Method and system for moving equipment into and through an underground well
    // </A></TD>
    // </TR>

    // As of June 2006 the USPTO changed some of their formatting.  This is the old regexp
    //QRegExp rowRx("<TR>.*<A .*>(.*)</A>.*</A>.*</TR>");
    // This is the new regexp
    QRegExp rowRx("<TR>.*<A .*>(.*)</A>.*</A>.*</TD>");
    rowRx.setMinimal(true);
    
    pos = 0;
    while(pos >= 0){
      pos = rowRx.indexIn(table, pos);

      if(pos > -1){
		  QString r = rowRx.cap(1);
		  removeExtraWhiteSpace(r);
		  patents += r;
		  pos += rowRx.matchedLength();
	  }
    }
  }
  
  return(patents);
}


bool USPTOSearch::getNextResultsPageLink(QString &link)
{
	QString regexp = "href=([^<]*Next[^<]*)>";
	QRegExp nextListRx(regexp, Qt::CaseInsensitive);
	nextListRx.setMinimal(true);
	//bool valid = nextListRx.isValid();

	//QString simp = m_text.simplified();
	//int simLen = simp.length();
	int pos = -1;
	if(-1 != (pos = nextListRx.indexIn(m_text))){
		link = nextListRx.cap(1);	
		// weed out shopping cart links
		if(link.contains("Shopping")){
			link.clear();
			return false;
		}
		return true;
	}

	link.clear();
	return false;
}
