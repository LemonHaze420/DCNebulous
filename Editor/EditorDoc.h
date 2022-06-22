// EditorDoc.h : interface of the CEditorDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITORDOC_H__0ECBFEC2_89DD_41AF_9A66_E2711A2CF652__INCLUDED_)
#define AFX_EDITORDOC_H__0ECBFEC2_89DD_41AF_9A66_E2711A2CF652__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../tower.h"
#include <afxtempl.h>

#define MAX_TOWER_HEIGHT    500


class CEditorDoc : public CDocument
{
protected: // create from serialization only
	CEditorDoc();
	DECLARE_DYNCREATE(CEditorDoc)

// Attributes
public:
    TowerRing m_Tower[MAX_TOWER_HEIGHT];
    CList<FILEMONSTER,FILEMONSTER&> m_Monsters;

    CString m_strTextures;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	int GetTowerHeight();
	virtual ~CEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CEditorDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORDOC_H__0ECBFEC2_89DD_41AF_9A66_E2711A2CF652__INCLUDED_)
