// EditorView.h : interface of the CEditorView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITORVIEW_H__CCD53046_3C82_4321_9941_EFB4DC820994__INCLUDED_)
#define AFX_EDITORVIEW_H__CCD53046_3C82_4321_9941_EFB4DC820994__INCLUDED_

#include "..\tower.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../tower.h"

class CEditorView : public CScrollView
{
protected: // create from serialization only
	CEditorView();
	DECLARE_DYNCREATE(CEditorView)

// Attributes
public:
	CEditorDoc* GetDocument();

    TowerBrick m_CurrentBrickType;
    unsigned char m_CurrentMonsterType;
    TowerBrick m_CurrentModType;

    enum { EditMode_Bricks, EditMode_Monster, EditMode_Mod } m_EditMode;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    CImageList m_imgBricks;
    CImageList m_imgMonsters;
    CImageList m_imgMods;

// Generated message map functions
protected:
	int GetModTypeIndex( TowerBrick b );
	TowerBrick ChangeBrick( TowerBrick brick, TowerBrick newtype );
	BOOL GetIndices( int x, int y, int& iBrick, int& iRing );
	BOOL m_bCapture;
	TowerBrick* GetBrick( int x, int y );
	//{{AFX_MSG(CEditorView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBCrak();
	afx_msg void OnBDoor();
	afx_msg void OnBExit();
	afx_msg void OnBLift();
	afx_msg void OnBMvlf();
	afx_msg void OnBMvrt();
	afx_msg void OnBNorm();
	afx_msg void OnBPlat();
	afx_msg void OnBStop();
	afx_msg void OnBStrt();
	afx_msg void OnBSupp();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnMonsterLeftright();
	afx_msg void OnMonsterRover();
	afx_msg void OnMonsterRoverbadass();
	afx_msg void OnMonsterUpdown();
	afx_msg void OnModNospin();
	afx_msg void OnMonsterRoverangry();
	afx_msg void OnModDisablespin();
	afx_msg void OnModEnablespin();
	afx_msg void OnBShot();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in EditorView.cpp
inline CEditorDoc* CEditorView::GetDocument()
   { return (CEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORVIEW_H__CCD53046_3C82_4321_9941_EFB4DC820994__INCLUDED_)
