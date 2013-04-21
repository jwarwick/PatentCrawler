///////////////////////////////////////////////////////////
// USPTOPatent.cpp
// jwarwick, 22 Feb 2004
//
// USPTO patent parser 
//
///////////////////////////////////////////////////////////

#include <QFile>
#include <QRegexp>

#include <QtDebug>

#include "USPTOPatent.h"

USPTOPatent::USPTOPatent(QObject *parent)
  : QObject(parent)
{
  buildStateList();
}


bool USPTOPatent::setContent(QString patentText)
{
  m_text = patentText;
  return true;
}

bool USPTOPatent::setSource(QString filename)
{
  QFile file(filename);
  if(file.open(QIODevice::ReadOnly)){
    QTextStream stream(&file);
    setContent(stream.readAll());
    file.close();
    return true;
  }else{
    return false;
  }
}

bool USPTOPatent::writeSource(QString filename)
{
  QFile file(filename);
  if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
    QTextStream stream(&file);
    stream << m_text;
    file.close();
    return true;
  }else{
    return false;
  }
}

void USPTOPatent::removeTags(QString &s)
{
  QRegExp tag("<.+>");
  tag.setMinimal(true);
  s.replace(tag, "");
	s.replace("&nbsp;", " ");
}

void USPTOPatent::removeExtraWhiteSpace(QString &s)
{
  s = s.simplified();
}

bool USPTOPatent::isAvailable()
{
  QRegExp rx("Full text is not available for this patent");
  int pos = rx.indexIn(m_text);
  if(pos >= 0){
    return(false);
  }
  
  QRegExp rx2("No patents have matched your query");
  pos = rx2.indexIn(m_text);
  if(pos >= 0){
    return(false);
  }
  
  return(true);
}

void USPTOPatent::exportPatent(int depth, const QStringList &referencedBy,
			       ExportSettings &settings, QTextStream &stream)
{
  if(m_text.isEmpty()){
    return;
  }
  if(!isAvailable()){
    return;
  }
  
  /*
    PN Patent Number
    TTL Title
    ISD Issue Date
    APD Application Date
    
    IN Inventor Name
    IC Inventor City
    IS Inventor State
    ICN Inventor Country
    
    AN Assignee Name
    AC Assignee City
    AS Assignee State
    ACN Assignee Country
    
    CCL Current US Classification
    ICL International Classification
    APN Application Serial Number
    
    APT Application Type
    REIS Reissue Data
    PRIR Foreign Priority
    GOVT Government Interest
    
    ABST Abstract (this is the one long field in the set- it's  important that we get the whole thing.) 
    
    # of claims
  */

  QString fieldSep = "\t";
  
  if(settings.patentNumber)
	  stream << getPatentNumber() << fieldSep;

  if(settings.inSearch){
	  if(0 == depth){
		  stream << "True" << fieldSep;
	  }else{
		  stream << "False" << fieldSep;
	  }
  }
  
  if(settings.patentTitle)
	  stream << getTitle() << fieldSep;

   if(settings.applicationDate)
	  stream << getFileDate() << fieldSep; 
   
   if(settings.issueDate)
	  stream << getIssueDate() << fieldSep;

   if(settings.applicationNumber)
	  stream << getApplicationNumber() << fieldSep;
  
  if(settings.applicationType)
	  stream << getApplicationType() << fieldSep;
  
  if(settings.reissueData)
	  stream << getReissueData() << fieldSep;
  
  //stream << getInventorName() << fieldSep;
  //stream << getInventorCity() << fieldSep;
  //stream << getInventorState() << fieldSep;
  //stream << getInventorCountry() << fieldSep;
  
  int i;
  for(i=0;i<settings.numberOfInventors;i++){
    stream << getNthInventorName(i) << fieldSep;
    stream << getNthInventorCity(i) << fieldSep;
    stream << getNthInventorState(i) << fieldSep;
    stream << getNthInventorCountry(i) << fieldSep;
  }
  
  if(settings.assigneeName)
	  stream << getAssigneeName() << fieldSep;
  
  if(settings.assigneeCity)
	  stream << getAssigneeCity() << fieldSep;
  
  if(settings.assigneeState)
	  stream << getAssigneeState() << fieldSep;
  
  if(settings.assigneeCountry)
	  stream << getAssigneeCountry() << fieldSep;

  //stream << getCurrentUSClass() << fieldSep;
  //stream << getInternationalClass() << fieldSep;
  
  for(i=0;i<settings.numberOfUSClasses;i++){
    stream << getNthUSClass(i) << fieldSep;
  }
  
  for(i=0;i<settings.numberOfInternationalClasses;i++){
    stream << getNthInternationalClass(i) << fieldSep;
  }
  
  if(settings.fieldOfSearch)
	  stream << getFieldOfSearch() << fieldSep;
  
  if(settings.foreignPriority)
	  stream << getForeignPriority() << fieldSep;
  
  if(settings.governmentInterest)
	  stream << getGovernmentInterest() << fieldSep;
  
  if(settings.abstract)
	  stream << getAbstract() << fieldSep;
  
  if(settings.numberClaims)
	  stream << getNumberClaims() << fieldSep;

  QString ref;
  if(settings.usReferences){
	QStringListIterator usRefIt(getUSReferences());
	while(usRefIt.hasNext()){
		ref = usRefIt.next();
		ref.replace(",", "");
		stream << ref << ", ";
	}
	stream << fieldSep;
  }

  if(settings.foreignReferences){
	QStringListIterator foreignRefIt(getForeignReferences());
	while(foreignRefIt.hasNext()){
		ref = foreignRefIt.next();
		ref.replace(",", "");
		stream << ref << ", ";
	}
	stream << fieldSep;
  }

  if(settings.otherReferences){
	QStringListIterator otherRefIt(getOtherReferences());
	while(otherRefIt.hasNext()){
		ref = otherRefIt.next();
		ref.replace(",", "");
		stream << ref << ", ";
	}
	stream << fieldSep;
  }

  if(settings.referencingPatents){
	QStringListIterator refByIt(referencedBy);
	while(refByIt.hasNext()){
		ref = refByIt.next();
		ref.replace(",", "");
		stream << ref << ", ";
	}
	stream << fieldSep;
  }

  if(settings.primaryExaminer)
	  stream << getPrimaryExaminer() << fieldSep;

  if(settings.assistantExaminer)
	  stream << getAssistantExaminer() << fieldSep;

  if(settings.attorney)
	  stream << getAttorney() << fieldSep;

  stream << endl;
}

void USPTOPatent::exportPatentSpecialCore(QTextStream &stream)
{
	QString fieldSep = "\t";

	if(m_text.isEmpty()){
		stream << getPatentNumber() << endl;
		return;
	}

	if(!isAvailable()){
		stream << getPatentNumber() << endl;
		return;
	}

	stream << getPatentNumber() << fieldSep << fieldSep << fieldSep << fieldSep;

	stream << getIssueDate() << fieldSep;
	stream << getFileDate() << fieldSep;
	stream << getAssigneeName() << fieldSep << fieldSep;

	/*
	QString location = tr("%1, %2").arg(getAssigneeCity()).arg(getAssigneeState());
	if(getAssigneeCity().isEmpty() && getAssigneeState().isEmpty()){
		location = "";
	}
	*/

	QString city = getAssigneeCity();
	QString state = getAssigneeState();
	QString location = city;
	if(!state.isEmpty()){
	  location += tr(", %1").arg(state);
	}

	stream << location << fieldSep;
	stream << getAssigneeCountry() << fieldSep;

	stream << getNthInventorName(0) << fieldSep;
	/*location = tr("%1, %2").arg(getNthInventorCity(0)).arg(getNthInventorState(0));
	if(getNthInventorCity(0).isEmpty() && getNthInventorState(0).isEmpty()){
		location = "";
	}
	*/
	city = getNthInventorCity(0);
	state = getNthInventorState(0);
	location = city;
	if(!state.isEmpty()){
	  location += tr(", %1").arg(state);
	}

	stream << location << fieldSep;
	stream << getNthInventorCountry(0) << fieldSep;

	stream << fieldSep << fieldSep;

	stream << getNthUSClass(0) << fieldSep;
	QString usClass;
	QString nextClass;
	int n=1;
	do{
		nextClass = getNthUSClass(n++);
		usClass += tr(";%1").arg(nextClass);
	}while(!nextClass.isEmpty());
	stream << usClass << fieldSep;

	stream << getInternationalClass() << fieldSep;
	stream << getFieldOfSearch() << fieldSep;
	stream << getTitle() << fieldSep << fieldSep;

	QStringList otherRefsList = getOtherReferences();
	if(!otherRefsList.isEmpty()){
		stream << "1. " << otherRefsList.at(0) << fieldSep;
	}else{
		stream << fieldSep;
	}

	stream << getPrimaryExaminer() << fieldSep;
	stream << endl;

	// add a line for each additional author
	int inventorIndex = 1;
	QString inventorName;
	QString otherRef;
	QString outputString;
	do{
		outputString = "";

		for(int i=0;i<10;i++){
			outputString += fieldSep;
		}
		inventorName = getNthInventorName(inventorIndex);
		outputString += inventorName;
		outputString += fieldSep;
		/*
		location = tr("%1, %2").arg(getNthInventorCity(inventorIndex)).arg(getNthInventorState(inventorIndex));
		if(getNthInventorCity(inventorIndex).isEmpty() && getNthInventorState(inventorIndex).isEmpty()){
			location = "";
		}
		*/

		city = getNthInventorCity(inventorIndex);
		state = getNthInventorState(inventorIndex);
		location = city;
		if(!state.isEmpty()){
		  location += tr(", %1").arg(state);
		}

		if(!inventorName.isEmpty()){
			outputString += location;
			outputString += fieldSep;
			outputString += getNthInventorCountry(inventorIndex);
			outputString += fieldSep;
		}else{
			outputString += fieldSep;
			outputString += fieldSep;
		}

		for(int j=0;j<8;j++){
			outputString += fieldSep;
		}

		if(inventorIndex < otherRefsList.size()){
			otherRef = tr("%1. %2").arg(inventorIndex+1).arg(otherRefsList.at(inventorIndex));
		}else{
			otherRef = "";
		}
		outputString += otherRef;

		if(!inventorName.isEmpty() || !otherRef.isEmpty()){
			stream << outputString << endl;
		}

		inventorIndex++;
	}while(!inventorName.isEmpty() || !otherRef.isEmpty());
}

void USPTOPatent::exportPatentSpecialReferencing(QTextStream &stream)
{
	QString fieldSep = "\t";

	stream << fieldSep;

	if(m_text.isEmpty()){
		stream << getPatentNumber() << endl;
		return;
	}

	if(!isAvailable()){
		stream << getPatentNumber() << endl;
		return;
	}

	stream << getPatentNumber() << fieldSep << fieldSep << fieldSep;
	stream << getIssueDate() << fieldSep;
	stream << getFileDate() << fieldSep;
	stream << getAssigneeName() << fieldSep << fieldSep;

	/*
	QString location = tr("%1, %2").arg(getAssigneeCity()).arg(getAssigneeState());
	if(getAssigneeCity().isEmpty() && getAssigneeState().isEmpty()){
		location = "";
	}
	*/

	QString city = getAssigneeCity();
	QString state = getAssigneeState();
	QString location = city;
	if(!state.isEmpty()){
	  location += tr(", %1").arg(state);
	}

	stream << location << fieldSep;
	stream << getAssigneeCountry() << fieldSep;

	for(int i=0;i<5;i++){
		stream << fieldSep;
	}

	stream << getNthUSClass(0) << fieldSep;
	QString usClass;
	QString nextClass;
	int n=1;
	do{
		nextClass = getNthUSClass(n++);
		usClass += tr(";%1").arg(nextClass);
	}while(!nextClass.isEmpty());
	stream << usClass << fieldSep;

	stream << getInternationalClass() << fieldSep;

	stream << endl;
}

void USPTOPatent::exportPatentSpecialReferencedBy(QTextStream &stream)
{
	QString fieldSep = "\t";

	stream << fieldSep << fieldSep;

	if(m_text.isEmpty()){
		stream << getPatentNumber() << endl;
		return;
	}

	if(!isAvailable()){
		stream << getPatentNumber() << endl;
		return;
	}

	stream << getPatentNumber() << fieldSep << fieldSep;
	stream << getIssueDate() << fieldSep;
	stream << getFileDate() << fieldSep;
	stream << getAssigneeName() << fieldSep << fieldSep;

	/*
	QString location = tr("%1, %2").arg(getAssigneeCity()).arg(getAssigneeState());
	if(getAssigneeCity().isEmpty() && getAssigneeState().isEmpty()){
		location = "";
		}
	*/
	
	QString city = getAssigneeCity();
	QString state = getAssigneeState();
	QString location = city;
	if(!state.isEmpty()){
	  location += tr(", %1").arg(state);
	}

	stream << location << fieldSep;
	stream << getAssigneeCountry() << fieldSep;

	for(int i=0;i<5;i++){
		stream << fieldSep;
	}

	stream << getNthUSClass(0) << fieldSep;
	QString usClass;
	QString nextClass;
	int n=1;
	do{
		nextClass = getNthUSClass(n++);
		usClass += tr(";%1").arg(nextClass);
	}while(!nextClass.isEmpty());
	stream << usClass << fieldSep;

	stream << getInternationalClass() << fieldSep;

	stream << endl;
}

void USPTOPatent::exportPatentSpecialForeign(QTextStream &stream)
{
	QString fieldSep = "\t";
	QString outputString;
	QString foreignRef;
	int n=0;
	do{
		outputString = fieldSep + fieldSep + fieldSep;
		foreignRef = getNthForeignReferenceNumber(n);
		outputString += foreignRef + fieldSep;
		outputString += getNthForeignReferenceDate(n);
		outputString += fieldSep;
		QString tmp = getNthForeignReferenceCountry(n);
		tmp = tmp.simplified();
		outputString += tmp;

		if(!foreignRef.isEmpty()){
			stream << outputString;
			stream << endl;
		}
		n++;
	}while(!foreignRef.isEmpty());
}


QString USPTOPatent::getText()
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

QString USPTOPatent::getPatentNumber()
{
	if(!m_patentNumber.isEmpty()){
		return m_patentNumber;
	}

	QString number;
  QRegExp rx("<TITLE>.+: (.+)</TITLE>");
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    number = rx.cap(1);
  }else{
	number = "No match";
  }
  m_patentNumber = number;
  return m_patentNumber;
}

QString USPTOPatent::getApplicationType()
{
  QString patentNumber = getPatentNumber();
  
  // reissue
  if(patentNumber.startsWith("R")){
    return(QString::number(2));
  }
  
  // there is no type 3
  
  // design
  if(patentNumber.startsWith("D")){
    return(QString::number(4));
  }
  
  if(patentNumber.startsWith("T")){
    return(QString::number(5));
  }
  
  if(patentNumber.startsWith("PP")){
    return(QString::number(6));
  }
  
  if(patentNumber.startsWith("H")){
    return(QString::number(7));
  }
  
  // utility
  return(QString::number(1));
}

QString USPTOPatent::getTitle()
{
  QRegExp rx("(<font .*)</font>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    return(match);
  }
  return("No match");
}

QString USPTOPatent::getIssueDate()
{
  QRegExp rx("<TD ALIGN=\"RIGHT\" WIDTH=\"50%\"> <B>(.*)</B></TD>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    return(match);
  }
  return("No match");
}

QString USPTOPatent::getFileDate()
{
  QRegExp rx("Filed:(.*)</B>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    return(match);
  }
  return("No match");
}

QString USPTOPatent::getInventors()
{
	if(!m_inventors.isEmpty()){
		return m_inventors;
	}

  QRegExp rx("Inventors:(.*)</TR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    m_inventors = match;
	return(match);
  }
  return("");	
}

QString USPTOPatent::getNthInventorField(int n)
{
  QString inventors = getInventors();
  QStringList invSplit = inventors.split(")");
  if(n >= invSplit.count()){
    return("");
  }
  
  QString inv = invSplit[n];
  if(inv.startsWith(";")){
    inv = inv.right(inv.length()-1);
  }
  inv += ")";
  return(inv);
}

QString USPTOPatent::getNthInventorName(int n)
{
  QString invField = getNthInventorField(n);	
  if(invField.isEmpty()){
    return("");
  }
  
  QRegExp rx("(.*)\\(.*\\)");
  rx.setMinimal(true);
  int pos = rx.indexIn(invField);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    return(match);
  }
  return("");
  
}

QString USPTOPatent::getNthInventorCity(int n)
{
  QString invField = getNthInventorField(n);	
  if(invField.isEmpty()){
    return("");
  }
  
  QRegExp rx(".*\\((.*,)*(.*),.*\\)");
  rx.setMinimal(true);
  int pos = rx.indexIn(invField);
  if(pos >= 0){
    QString match = rx.cap(2);
    removeTags(match);
    removeExtraWhiteSpace(match);
    return(match);
  }
  
  return("");
}

QString USPTOPatent::getNthInventorState(int n)
{
  QString invField = getNthInventorField(n);	
  if(invField.isEmpty()){
    return("");
  }
  
  QRegExp rx(".*\\(.*([A-Z][A-Z]).*\\)");
  rx.setMinimal(true);
  int pos = rx.indexIn(invField);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    if(isState(match)){
      return(match);
    }else{
      return("");
    }
  }
  return("");
}

QString USPTOPatent::getNthInventorCountry(int n)
{
  QString invField = getNthInventorField(n);	
  if(invField.isEmpty()){
    return("");
  }
  
  QRegExp rx(".*\\(.*([A-Z][A-Z]).*\\)");
  rx.setMinimal(true);
  int pos = rx.indexIn(invField);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    if(isState(match)){
      return("US");
    }else{
      return(match);
    }
  }
  
  return("");
}


QString USPTOPatent::getAssignee()
{
  QRegExp rx("Assignee:(.*)</TR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    return(match);
  }
  return("");	
}

QString USPTOPatent::getAssigneeName()
{
  QString str = getAssignee();
  QRegExp rx("(.*)\\(.*\\)");
  rx.setMinimal(true);
  int pos = rx.indexIn(str);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    return(match);
  }
  return("");	
}

QString USPTOPatent::getAssigneeCity()
{
  QString str = getAssignee();
  QRegExp rx(".*\\((.*,)*(.*),.*\\)");
  rx.setMinimal(true);
  int pos = rx.indexIn(str);
  if(pos >= 0){
    QString match = rx.cap(2);
    removeTags(match);
    removeExtraWhiteSpace(match);
    return(match);
  }
  return("");
}

QString USPTOPatent::getAssigneeState()
{
  QString str = getAssignee();
  QRegExp rx(".*\\(.*([A-Z][A-Z]).*\\)");
  rx.setMinimal(true);
  int pos = rx.indexIn(str);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    if(isState(match)){
      return(match);
    }else{
      return("");
    }
  }
  return("");
}

QString USPTOPatent::getAssigneeCountry()
{
  QString str = getAssignee();
  QRegExp rx(".*\\(.*([A-Z][A-Z]).*\\)");
  rx.setMinimal(true);
  int pos = rx.indexIn(str);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    if(isState(match)){
      return("US");
    }else{
      return(match);
    }
  }
  return("");
}

QString USPTOPatent::getApplicationNumber()
{
  QRegExp rx("Appl. No.:(.*)</TR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    return(match);
  }
  return("No match");	
}

QString USPTOPatent::getAbstract()
{
  QRegExp rx("Abstract(.*)</P>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    
    return(match);
  }
  return("No match");
}

QString USPTOPatent::getCurrentUSClass()
{
	if(!m_currentUSClass.isEmpty()){
		return m_currentUSClass;
	}

  QRegExp rx("Current U.S. Class:(.*)</TR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    
	m_currentUSClass = match;
    return(match);
  }
  return("No match");
}

QString USPTOPatent::getNthUSClass(int n)
{
  QString classes = getCurrentUSClass();
  QStringList splitClasses = classes.split(";");
  if(n >= splitClasses.count()){
    return("");
  }
  return(splitClasses.at(n));
}

QString USPTOPatent::getInternationalClass()
{
	if(!m_internationalClass.isEmpty()){
		return m_internationalClass;
	}

  QRegExp rx("Intern'l Class:(.*)</TR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    
	m_internationalClass = match;
    return(match);
  }
  return("No match");
}

QString USPTOPatent::getNthInternationalClass(int n)
{
  QString classes = getInternationalClass();
  QStringList splitClasses = classes.split(";");
  if(n >= splitClasses.count()){
    return("");
  }
  return(splitClasses.at(n));
}

QString USPTOPatent::getFieldOfSearch()
{
  QRegExp rx("Field of Search:(.*)</TR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    
    return(match);
  }
  return("No match");
}

QString USPTOPatent::getPrimaryExaminer()
{
  QRegExp rx("Primary Examiner:(.*)<BR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    
    return(match);
  }
  return("No match");
}

QString USPTOPatent::getAssistantExaminer()
{
  QRegExp rx("Assistant Examiner:(.*)<BR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    
    return(match);
  }
  return("No match");
}

QString USPTOPatent::getAttorney()
{
  QRegExp rx("Firm:(.*)<BR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos > 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    
    return(match);
  }
  return("No match");
}

QStringList USPTOPatent::getUSReferences()
{
	if(!m_USReferences.isEmpty()){
		return m_USReferences;
	}

  QStringList patents;
  QRegExp tableRx(">U.S. Patent Documents(.*)(</table>|Foreign Patent Documents)");
  tableRx.setMinimal(true);
  int pos = tableRx.indexIn(m_text);
  if(pos > 0){
    QString table = tableRx.cap(1);
    // <TR><TD WIDTH="25%"><a href="/netacgi/nph-Parser?Sect2=PTO1&Sect2=HITOFF&p=1&u=%2Fnetahtml%2FindexIn-bool.html&r=1&f=G&l=50&d=PALL&RefSrch=yes&Query=PN%2F3704446">3704446</a></TD><TD WIDTH="25%">Nov., 1972</TD><TD WIDTH="25%" ALIGN="LEFT">Walter</TD><TD WIDTH="25%" ALIGN="RIGHT">315/77.</TD></TR>

    // unawarded us patent ref
    //<TR><TD WIDTH="25%">2001/0006380</TD><TD WIDTH="25%">Jul., 2001</TD><TD WIDTH="25%" ALIGN="LEFT">Jeong.</TD><TD WIDTH="25%" ALIGN="RIGHT"></TD></TR>
 
   //QRegExp rowRx("<TR><.*><.*>(.*)<.*><.*><.*>.*<.*><.*>.*<.*><.*>.*<.*></TR>");
    QRegExp rowRx("<TR>.*<a.*>(.*)</a>.*</TR>");
    rowRx.setMinimal(true);
    
    pos = 0;
    while(pos >= 0){
      pos = rowRx.indexIn(table, pos);
      if(pos > -1){
		  QString match = rowRx.cap(1);
		  // don't add unissued patent ref of the form 2001/0006380
		  if(!match.contains("/")){
			  patents += rowRx.cap(1);
		  }
		  pos += rowRx.matchedLength();
      }
    }
  }
  
  m_USReferences = patents;
  return(m_USReferences);
}

QStringList USPTOPatent::getForeignReferences()
{
	if(!m_foreignReferences.isEmpty()){
		return m_foreignReferences;
	}

  QStringList patents;
  QRegExp tableRx("Foreign Patent Documents(.*)</table>", Qt::CaseInsensitive);
  tableRx.setMinimal(true);
  int pos = tableRx.indexIn(m_text);
  if(pos > 0){
    QString table = tableRx.cap(1);

    // as of June 2006, the format of the foreign refs changed
    QRegExp rowRx;
    if(table.contains("<TD WIDTH=", Qt::CaseInsensitive)){
        // old version
        //<TR><TD WIDTH="25%">90/02864</TD><TD WIDTH="25%">Mar., 1990</TD><TD WIDTH="25%" ALIGN="LEFT">WO</TD><TD WIDTH="25%" ALIGN="RIGHT">.</TD></TR>		
        rowRx = QRegExp("<TR><.*>(.*)<.*><.*>(.*)<.*><.*>(.*)<.*><.*>.*<.*></TR>");
    }else{
        // new version
        //<TR><TD align=left></TD><TD align=left>2 178 432</TD><TD></TD><TD align=left>Dec., 1996</TD><TD></TD><TD align=left>CA</TD></TR>
        rowRx = QRegExp("<TR>.*>.*>.*>(.*)<.*>.*>.*>.*>(.*)<.*>.*>.*>.*>(.*)<.*></TR>", Qt::CaseInsensitive);
    }
    rowRx.setMinimal(true);
    
    pos = 0;
    while(pos >= 0){
		pos = rowRx.indexIn(table, pos);
		if(pos > -1){
            if(!rowRx.cap(1).isEmpty()){
			    QString str;
			    str += rowRx.cap(1).simplified();
			    str += "\t";
			    str += rowRx.cap(2).simplified();
			    str += "\t";
			    str += rowRx.cap(3).simplified();
        
			    patents += str;
            }
		    pos += rowRx.matchedLength();
		}
	}
  }
  
  m_foreignReferences = patents;
  return(patents);
}

QString USPTOPatent::getNthForeignReference(int n)
{
  QStringList refs = getForeignReferences();
  if(n >= refs.count()){
    return("");
  }
  return(refs.at(n));
}

QString USPTOPatent::getNthForeignReferenceNumber(int n)
{
	QString ref = getNthForeignReference(n);
	if(ref.isEmpty()){
		return("");
	}
	QStringList splitRefs = ref.split("\t");
	return(splitRefs.at(0));
}

QString USPTOPatent::getNthForeignReferenceDate(int n)
{
	QString ref = getNthForeignReference(n);
	if(ref.isEmpty()){
		return("");
	}
	QStringList splitRefs = ref.split("\t");
	return(splitRefs.at(1));
}

QString USPTOPatent::getNthForeignReferenceCountry(int n)
{
	QString ref = getNthForeignReference(n);
	if(ref.isEmpty()){
		return("");
	}
	QStringList splitRefs = ref.split("\t");
	return(splitRefs.at(2));
}

QStringList USPTOPatent::getOtherReferences()
{
  QStringList patents;
  QRegExp tableRx("Other References(.*</TD>)");
  tableRx.setMinimal(true);
  int pos = tableRx.indexIn(m_text);
  if(pos > 0){
    QString table = tableRx.cap(1);
    QRegExp rowRx("<br>(.*)(<br>|</TD>)");
    rowRx.setMinimal(true);
    
    pos = 0;
    while(pos >= 0){
		pos = rowRx.indexIn(table, pos);
		if(pos > -1){
			QString str = rowRx.cap(1);
			patents += str.simplified();
			pos += rowRx.matchedLength();
			pos -= 4; // don't strip next <br>
		}
	}
  }
  
  return(patents);
}


QString USPTOPatent::getReissueData()
{
  QRegExp rx("Reissue of:(.*)</TR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos >= 0){
    QString match = rx.cap(1);
    match.replace("<TD>", " ");
    removeTags(match);
    removeExtraWhiteSpace(match);
    
    return(match);
  }
  return("");
  
}

QString USPTOPatent::getForeignPriority()
{
  QString txt;
  QRegExp tableRx("Foreign Application Priority Data(.*)</TABLE>");
  tableRx.setMinimal(true);
  int pos = tableRx.indexIn(m_text);
  if(pos > 0){
    QString table = tableRx.cap(1);
    
    QRegExp rowRx("<TR>(.*)</TR>");
    rowRx.setMinimal(true);
    
    pos = 0;
    while(pos >= 0){
      pos = rowRx.indexIn(table, pos);
      if(pos > -1){
	QString tmp = rowRx.cap(1);
	tmp = tmp.replace("<TD>", " ");
	removeTags(tmp);
	removeExtraWhiteSpace(tmp);
	txt += tmp + ";";
	
	pos += rowRx.matchedLength();
      }
    }
  }
  
  return(txt);
}

QString USPTOPatent::getGovernmentInterest()
{
  QRegExp rx("Goverment Interests.*<HR>(.*)<HR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos >= 0){
    QString match = rx.cap(1);
    removeTags(match);
    removeExtraWhiteSpace(match);
    
    return(match);
  }
  return("");
}

QString USPTOPatent::getNumberClaims()
{
  QRegExp rx("<CENTER><B><I>Claims.*<HR>(.*)<HR>");
  rx.setMinimal(true);
  int pos = rx.indexIn(m_text);
  if(pos >= 0){
    QString match = rx.cap(1);
    
    QRegExp claimRx("<BR><BR>([0-9]+)");
    pos = claimRx.indexIn(match, -1);
    if(pos >= 0){
      QString numClaims = claimRx.cap(1);
      removeExtraWhiteSpace(numClaims);
      return(numClaims);
    }
  }
  return("0");
}

bool USPTOPatent::isState(QString s)
{
  removeExtraWhiteSpace(s);
  return(m_stateList.contains(s));
}

void USPTOPatent::buildStateList()
{
  m_stateList += "AL";
  m_stateList += "AK";
  m_stateList += "AS";
  m_stateList += "AZ";
  m_stateList += "AR";
  m_stateList += "CA";
  m_stateList += "CO";
  m_stateList += "CT";
  m_stateList += "DE";
  m_stateList += "DC";
  m_stateList += "FM";
  m_stateList += "FL";
  m_stateList += "GA";
  m_stateList += "GU";
  m_stateList += "HI";
  m_stateList += "ID";
  m_stateList += "IL";
  m_stateList += "IN";
  m_stateList += "IA";
  m_stateList += "KS";
  m_stateList += "KY";
  m_stateList += "LA";
  m_stateList += "ME";
  m_stateList += "MH";
  m_stateList += "MD";
  m_stateList += "MA";
  m_stateList += "MI";
  m_stateList += "MN";
  m_stateList += "MS";
  m_stateList += "MO";
  m_stateList += "MT";
  m_stateList += "NE";
  m_stateList += "NV";
  m_stateList += "NH";
  m_stateList += "NJ";
  m_stateList += "NM";
  m_stateList += "NY";
  m_stateList += "NC";
  m_stateList += "ND";
  m_stateList += "MP";
  m_stateList += "OH";
  m_stateList += "OK";
  m_stateList += "OR";
  m_stateList += "PW";
  m_stateList += "PA";
  m_stateList += "PR";
  m_stateList += "RI";
  m_stateList += "SC";
  m_stateList += "SD";
  m_stateList += "TN";
  m_stateList += "TX";
  m_stateList += "UT";
  m_stateList += "VT";
  m_stateList += "VI";
  m_stateList += "VA";
  m_stateList += "WA";
  m_stateList += "WV";
  m_stateList += "WI";
  m_stateList += "WY";
}
