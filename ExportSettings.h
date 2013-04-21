#ifndef _EXPORTSETTINGS_H_
#define _EXPORTSETTINGS_H_

typedef struct{
	bool inSearch;
	bool patentNumber;
    bool issueDate;
    bool applicationNumber;
    bool patentTitle;
    bool applicationDate;
    bool applicationType;
    int numberOfInventors;
	bool inventorName;
    bool inventorCity;
	bool inventorState;
	bool inventorCountry;
    bool assigneeName;
    bool assigneeCity;
    bool assigneeState;
    bool assigneeCountry;
    bool abstract;
    bool numberClaims;
    bool fieldOfSearch;
    bool reissueData;
    int numberOfUSClasses;
    int numberOfInternationalClasses;
    bool governmentInterest;
    bool foreignPriority;
	bool usReferences;
	bool foreignReferences;
    bool otherReferences;
	bool referencingPatents;
    bool primaryExaminer;
    bool assistantExaminer;
    bool attorney;
} ExportSettings;

#endif // _EXPORTSETTINGS_H_
