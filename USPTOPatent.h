///////////////////////////////////////////////////////////
// USPTOPatent.h
// jwarwick, 22 Feb 2004
//
// USPTO patent parser
//
///////////////////////////////////////////////////////////

#ifndef _USPTOPATENT_H_
#define _USPTOPATENT_H_

#include <QObject>
#include <QStringlist>
#include <QTextStream>

#include "DownloadThread.h"
#include "ExportSettings.h"

class USPTOPatent : public QObject
{
  Q_OBJECT
public:
  USPTOPatent(QObject *parent=0);
  
  bool setContent(QString patentText);
  bool setSource(QString filename);
  bool writeSource(QString filename);
  
  bool isAvailable();		// is the patent available on the USPTO site
  // or is this page just a warning saying its unavailable
  
  void exportPatent(int depth, const QStringList &referencedBy, ExportSettings &settings, QTextStream &stream);
  void exportPatentSpecialCore(QTextStream &stream);
  void exportPatentSpecialReferencing(QTextStream &stream);
  void exportPatentSpecialReferencedBy(QTextStream &stream);
  void exportPatentSpecialForeign(QTextStream &stream);

  QString getText();
  
  QString getPatentNumber();			// PN Patent Number
  QString getTitle();					// TTL Title
  QString getIssueDate();				// ISD Issue Date
  QString getFileDate();				// APD Application Date
  
  QString getInventors();
  QString getNthInventorField(int n);
  
  QString getNthInventorName(int n);		// IN Inventor Name
  QString getNthInventorCity(int n);		// IC Inventor City
  QString getNthInventorState(int n);	// IS Inventor State
  QString getNthInventorCountry(int n);  // ICN Inventor Country
  
  QString getAssignee();
  QString getAssigneeName();			// AN Assignee Name
  QString getAssigneeCity();			// AC Assignee City
  QString getAssigneeState();			// AS Assignee State
  QString getAssigneeCountry();		// ACN Assignee Country
  
  QString getAbstract();				// ABST Abstract 
  QString getApplicationNumber();		// APN Application Serial Number
  
  QString getCurrentUSClass();		// CCL Current US Classification
  QString getNthUSClass(int n);			// return a single CCL item
  
  QString getInternationalClass();	// ICL International Classification
  QString getNthInternationalClass(int n);
  
  QString getApplicationType();		// APT Application Type
  QString getReissueData();			// REIS Reissue Data
  QString getForeignPriority();		// PRIR Foreign Priority
  QString getGovernmentInterest();	// GOVT Government Interest		
  
  QString getNumberClaims();			// the number of claims
  
  QString getFieldOfSearch();
  QString getPrimaryExaminer();
  QString getAssistantExaminer();
  QString getAttorney();
  
  QStringList getUSReferences();
  QStringList getForeignReferences();
  QStringList getOtherReferences();
  
  QString getNthForeignReference(int n);
  QString getNthForeignReferenceNumber(int n);
  QString getNthForeignReferenceDate(int n);
  QString getNthForeignReferenceCountry(int n);

private:
  QString m_text;
  
  bool isState(QString s);
  void buildStateList();
  QStringList m_stateList;
  
  void removeTags(QString &s);
  void removeExtraWhiteSpace(QString &s);

	// stored values
	QString m_patentNumber;
	QStringList m_USReferences;
	QStringList m_foreignReferences;
	QString m_inventors;
	QString m_currentUSClass;
	QString m_internationalClass;

};

#endif // _USPTOPATENT_H_
