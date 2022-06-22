// EditorView.cpp : implementation of the CEditorView class
//

#include "stdafx.h"
#include "Editor.h"

#include "EditorDoc.h"
#include "EditorView.h"

#include "MemDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorView

IMPLEMENT_DYNCREATE(CEditorView, CScrollView)

BEGIN_MESSAGE_MAP(CEditorView, CScrollView)
	//{{AFX_MSG_MAP(CEditorView)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_COMMAND(IDC_B_CRAK, OnBCrak)
	ON_COMMAND(IDC_B_DOOR, OnBDoor)
	ON_COMMAND(IDC_B_EXIT, OnBExit)
	ON_COMMAND(IDC_B_LIFT, OnBLift)
	ON_COMMAND(IDC_B_MVLF, OnBMvlf)
	ON_COMMAND(IDC_B_MVRT, OnBMvrt)
	ON_COMMAND(IDC_B_NORM, OnBNorm)
	ON_COMMAND(IDC_B_PLAT, OnBPlat)
	ON_COMMAND(IDC_B_STOP, OnBStop)
	ON_COMMAND(IDC_B_STRT, OnBStrt)
	ON_COMMAND(IDC_B_SUPP, OnBSupp)
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_COMMAND(ID_MONSTER_LEFTRIGHT, OnMonsterLeftright)
	ON_COMMAND(ID_MONSTER_ROVER, OnMonsterRover)
	ON_COMMAND(ID_MONSTER_ROVERBADASS, OnMonsterRoverbadass)
	ON_COMMAND(ID_MONSTER_UPDOWN, OnMonsterUpdown)
	ON_COMMAND(ID_MOD_NOSPIN, OnModNospin)
	ON_COMMAND(ID_MONSTER_ROVERANGRY, OnMonsterRoverangry)
	ON_COMMAND(ID_MOD_DISABLESPIN, OnModDisablespin)
	ON_COMMAND(ID_MOD_ENABLESPIN, OnModEnablespin)
	ON_COMMAND(IDC_B_SHOT, OnBShot)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorView construction/destruction

CEditorView::CEditorView()
{
    m_imgBricks.Create( IDB_BRICKS, 32, 0, 0xFF00FF );
    m_imgMonsters.Create( IDB_MONSTERS, 32, 0, 0xFF00FF );
    m_imgMods.Create( IDB_MODS, 32, 0, 0xFF00FF );

    m_CurrentBrickType = B_PLAT;
    m_CurrentModType = B_NOSPINNY;
    m_CurrentMonsterType = MONSTER_TYPE_LEFTRIGHT;

    m_EditMode = EditMode_Bricks;

    m_bCapture = FALSE;
}

CEditorView::~CEditorView()
{
}

BOOL CEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CEditorView drawing

void CEditorView::OnDraw(CDC* pDC)
{
	CEditorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

    CDC& dc = *pDC;

    RECT rc; GetClientRect( &rc );
    dc.FillRect( &rc, CBrush::FromHandle((HBRUSH)::GetStockObject(WHITE_BRUSH) ) );

    //draw all rings
    int iRing = MAX_TOWER_HEIGHT;
    while( iRing-- )
    {
        for( int iBrick = 0; iBrick < BRICKS_PER_RING; iBrick++ )
        {
            m_imgBricks.Draw( &dc, (pDoc->m_Tower[iRing][iBrick] & B_TYPE_MASK), CPoint( iBrick*32, (MAX_TOWER_HEIGHT-iRing-1)*16 ), ILD_NORMAL );
        }
    }

    //draw all monsters
    POSITION p = pDoc->m_Monsters.GetHeadPosition();
    while( p )
    {
        FILEMONSTER& monster = pDoc->m_Monsters.GetNext(p);
        m_imgMonsters.Draw( &dc, monster.type, CPoint( (monster.iBrick*32), (MAX_TOWER_HEIGHT-monster.iRing)*16-32) , ILD_TRANSPARENT );
    }

    //draw all brick mods
    iRing = MAX_TOWER_HEIGHT;
    while( iRing-- )
    {
        for( int iBrick = 0; iBrick < BRICKS_PER_RING; iBrick++ )
        {
            if( pDoc->m_Tower[iRing][iBrick] & B_NOSPINNY )
                m_imgMods.Draw( &dc, GetModTypeIndex(B_NOSPINNY), CPoint( iBrick*32, (MAX_TOWER_HEIGHT-iRing-1)*16 ), ILD_NORMAL );
            if( pDoc->m_Tower[iRing][iBrick] & B_DISABLESPINNY )
                m_imgMods.Draw( &dc, GetModTypeIndex(B_DISABLESPINNY), CPoint( iBrick*32, (MAX_TOWER_HEIGHT-iRing-1)*16 ), ILD_NORMAL );
            if( pDoc->m_Tower[iRing][iBrick] & B_ENABLESPINNY )
                m_imgMods.Draw( &dc, GetModTypeIndex(B_ENABLESPINNY), CPoint( iBrick*32, (MAX_TOWER_HEIGHT-iRing-1)*16 ), ILD_NORMAL );
        }
    }



    dc.SetTextColor( 0x000000 );
    dc.SetBkColor( 0xFFFFFF );

    CFont* pFont = dc.SelectObject( CFont::FromHandle((HFONT)::GetStockObject(ANSI_VAR_FONT)));

    //draw current editing type & controls
    switch( m_EditMode )
    {
        case EditMode_Bricks:
            m_imgBricks.Draw( &dc, m_CurrentBrickType&B_TYPE_MASK, CPoint((BRICKS_PER_RING*32)+10, 4+GetScrollPosition().y), ILD_NORMAL );
            dc.TextOut( (BRICKS_PER_RING*32)+10,35+GetScrollPosition().y, CString("Left: Paint Block") );
            dc.TextOut( (BRICKS_PER_RING*32)+10,55+GetScrollPosition().y, CString("Right: Pick Up") );
            break;

        case EditMode_Monster:
            m_imgMonsters.Draw( &dc, m_CurrentMonsterType, CPoint( (BRICKS_PER_RING*32)+10, 4+GetScrollPosition().y ), ILD_TRANSPARENT );
            dc.TextOut( (BRICKS_PER_RING*32)+10,35+GetScrollPosition().y, CString("Left: Add") );
            dc.TextOut( (BRICKS_PER_RING*32)+10,55+GetScrollPosition().y, CString("Right: Delete") );
            break;

        case EditMode_Mod:
            m_imgMods.Draw( &dc, GetModTypeIndex(m_CurrentModType), CPoint( (BRICKS_PER_RING*32)+10, 4+GetScrollPosition().y), ILD_TRANSPARENT );
            dc.TextOut( (BRICKS_PER_RING*32)+10,35+GetScrollPosition().y, CString("Left: Add") );
            dc.TextOut( (BRICKS_PER_RING*32)+10,55+GetScrollPosition().y, CString("Right: Delete") );
            break;
    }

    dc.SelectObject( pFont );

}

void CEditorView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = BRICKS_PER_RING*32;
    sizeTotal.cy = MAX_TOWER_HEIGHT*16;
	SetScrollSizes(MM_TEXT, sizeTotal);
}

/////////////////////////////////////////////////////////////////////////////
// CEditorView diagnostics

#ifdef _DEBUG
void CEditorView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CEditorView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CEditorDoc* CEditorView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEditorDoc)));
	return (CEditorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEditorView message handlers

BOOL CEditorView::OnEraseBkgnd(CDC* pDC) 
{
    FillOutsideRect( pDC, CBrush::FromHandle( (HBRUSH)::GetStockObject(WHITE_BRUSH)));
/*
    RECT rc; GetClientRect( &rc );
    rc.left += BRICKS_PER_RING*32;
    rc.top += GetScrollPosition().x;
    rc.bottom += GetScrollPosition().x;
    pDC->FillRect( &rc, CBrush::FromHandle( (HBRUSH)::GetStockObject(WHITE_BRUSH)));
*/
    return TRUE;
}

TowerBrick* CEditorView::GetBrick(int x, int y)
{
    int iBrick, iRing;

    if( GetIndices( x, y, iBrick, iRing ) )
    {
	    CEditorDoc* pDoc = GetDocument();
	    ASSERT_VALID(pDoc);

        return &pDoc->m_Tower[iRing][iBrick];
    }
    return NULL;
}

void CEditorView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    switch( m_EditMode )
    {
        case EditMode_Bricks:
        {
            TowerBrick* pBrick = GetBrick( point.x, point.y );
            if( pBrick )
            {
                *pBrick = ChangeBrick( *pBrick, m_CurrentBrickType );

                if( (m_CurrentBrickType & B_TYPE_MASK) == B_DOOR )
                {
                    int iBrick, iRing;
                    if( GetIndices( point.x, point.y, iBrick, iRing ) )
                    {
                        CEditorDoc* pDoc = GetDocument();
                        ASSERT_VALID(pDoc);

                        iBrick = (iBrick+(BRICKS_PER_RING>>1)) % BRICKS_PER_RING;
                        pDoc->m_Tower[iRing][iBrick] = ChangeBrick( pDoc->m_Tower[iRing][iBrick], B_DOOR );
                    }
                }

                RedrawWindow();

                SetCapture();
                m_bCapture = TRUE;
            }
            break;
        }
    }
    
	CScrollView::OnLButtonDown(nFlags, point);
}

void CEditorView::OnMouseMove(UINT nFlags, CPoint point) 
{
    switch( m_EditMode )
    {
        case EditMode_Bricks:
            if( m_bCapture )
            {
                TowerBrick* pBrick = GetBrick( point.x, point.y );
                if( pBrick )
                {
                    *pBrick = ChangeBrick( *pBrick, m_CurrentBrickType );

                    if( (m_CurrentBrickType & B_TYPE_MASK) == B_DOOR )
                    {
                        int iBrick, iRing;
                        if( GetIndices( point.x, point.y, iBrick, iRing ) )
                        {
                            CEditorDoc* pDoc = GetDocument();
                            ASSERT_VALID(pDoc);

                            iBrick = (iBrick+(BRICKS_PER_RING>>1)) % BRICKS_PER_RING;
                            pDoc->m_Tower[iRing][iBrick] = ChangeBrick( pDoc->m_Tower[iRing][iBrick], B_DOOR );
                        }
                    }
                }

                RedrawWindow();
            }
            break;
    }
	
	CScrollView::OnMouseMove(nFlags, point);
}

void CEditorView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    switch( m_EditMode )
    {
        case EditMode_Bricks:
            if( m_bCapture ) { ReleaseCapture(); m_bCapture = FALSE; }
            break;

        case EditMode_Monster:
        {
            FILEMONSTER monster;
            monster.delta = 1;
            monster.reserved = 0;
            monster.type = m_CurrentMonsterType;

            int iBrick, iRing;
            GetIndices( point.x, point.y, iBrick, iRing );

            monster.iBrick = iBrick;
            monster.iRing = iRing;

            CEditorDoc* pDoc = GetDocument();
            ASSERT_VALID(pDoc);

            pDoc->m_Monsters.AddTail( monster );
            RedrawWindow();
            break;
        }

        case EditMode_Mod:
        {
            TowerBrick* pBrick = GetBrick( point.x, point.y );
            if( pBrick ) *pBrick |= m_CurrentModType;
            RedrawWindow();
            break;
        }
    }
	CScrollView::OnLButtonUp(nFlags, point);
}

void CEditorView::OnRButtonUp(UINT nFlags, CPoint point) 
{
    switch( m_EditMode )
    {
        case EditMode_Bricks:
        {
	        TowerBrick* pBrick = GetBrick( point.x, point.y );
            if( pBrick )
            {
                m_CurrentBrickType = *pBrick;
                RedrawWindow();
            }
            break;
        }

        case EditMode_Monster:
        {
            CEditorDoc* pDoc = GetDocument();
            ASSERT_VALID(pDoc);

            int iBrick, iRing;
            GetIndices( point.x, point.y, iBrick, iRing );

            POSITION p = pDoc->m_Monsters.GetHeadPosition();
            while( p )
            {
                POSITION pThis = p;
                FILEMONSTER& monster = pDoc->m_Monsters.GetNext(p);
                if( monster.iBrick == iBrick && monster.iRing == iRing )
                {
                    pDoc->m_Monsters.RemoveAt( pThis );
                    break;
                }
            }

            RedrawWindow();
            break;
        }
        case EditMode_Mod:
        {
            TowerBrick* pBrick = GetBrick( point.x, point.y );
            if( pBrick ) *pBrick &= ~m_CurrentModType;
            RedrawWindow();
            break;
        }
    }
	
	CScrollView::OnRButtonUp(nFlags, point);
}

TowerBrick CEditorView::ChangeBrick(TowerBrick brick, TowerBrick newtype)
{
    return (brick & B_MOD_MASK) | newtype;
}


BOOL CEditorView::GetIndices(int x, int y, int &iBrick, int &iRing)
{
    CRect rc( 0, 0, BRICKS_PER_RING*32, MAX_TOWER_HEIGHT*16 );

    x += GetScrollPosition().x;
    y += GetScrollPosition().y;


    if( rc.PtInRect( CPoint(x,y) ) )
    {
        iBrick = x / 32;
        iRing = MAX_TOWER_HEIGHT-(y/16)-1;
        return TRUE;
    }

    return FALSE;

}

void CEditorView::OnBCrak() 
{
	m_CurrentBrickType = B_CRAK;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBDoor() 
{
	m_CurrentBrickType = B_DOOR;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBExit() 
{
	m_CurrentBrickType = B_EXIT;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBLift() 
{
	m_CurrentBrickType = B_LIFT;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBMvlf() 
{
	m_CurrentBrickType = B_MVLF;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBMvrt() 
{
	m_CurrentBrickType = B_MVRT;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBNorm() 
{
	m_CurrentBrickType = B_NORM;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBPlat() 
{
	m_CurrentBrickType = B_PLAT;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBStop() 
{
	m_CurrentBrickType = B_STOP;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBStrt() 
{
	m_CurrentBrickType = B_STRT;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBSupp() 
{
	m_CurrentBrickType = B_SUPP;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnBShot() 
{
	m_CurrentBrickType = B_SHOT;
    m_EditMode = EditMode_Bricks;
	RedrawWindow();
}

void CEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CScrollView::OnSize(nType, cx, cy);
	
    SetScrollPos( SB_VERT, GetScrollLimit( SB_VERT ) );
}

void CEditorView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
    RedrawWindow();	
}

void CEditorView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
    RedrawWindow();	
}

void CEditorView::OnMonsterLeftright() 
{
    m_CurrentMonsterType = MONSTER_TYPE_LEFTRIGHT;
    m_EditMode = EditMode_Monster;
	RedrawWindow();
}

void CEditorView::OnMonsterRover() 
{
    m_CurrentMonsterType = MONSTER_TYPE_ROVER;
    m_EditMode = EditMode_Monster;
	RedrawWindow();
}

void CEditorView::OnMonsterRoverbadass() 
{
    m_CurrentMonsterType = MONSTER_TYPE_ROVERBADASS;
    m_EditMode = EditMode_Monster;
	RedrawWindow();
}

void CEditorView::OnMonsterUpdown() 
{
    m_CurrentMonsterType = MONSTER_TYPE_UPDOWN;
    m_EditMode = EditMode_Monster;
	RedrawWindow();
}

void CEditorView::OnMonsterRoverangry() 
{
    m_CurrentMonsterType = MONSTER_TYPE_ROVERANGRY;
    m_EditMode = EditMode_Monster;
	RedrawWindow();
}


void CEditorView::OnModNospin() 
{
    m_CurrentModType = B_NOSPINNY;
    m_EditMode = EditMode_Mod;
    RedrawWindow();
}


int CEditorView::GetModTypeIndex(TowerBrick b)
{
    switch( b & B_MOD_MASK )
    {
        case B_LIFTSTALK: return 0;
        case B_CRACKING: return 1;
        case B_NOSPINNY: return 2;
        case B_DISABLESPINNY: return 3;
        case B_ENABLESPINNY: return 4;
    }

    return -1;
}


void CEditorView::OnModDisablespin() 
{
    m_CurrentModType = B_DISABLESPINNY;
    m_EditMode = EditMode_Mod;
    RedrawWindow();
}

void CEditorView::OnModEnablespin() 
{
    m_CurrentModType = B_ENABLESPINNY;
    m_EditMode = EditMode_Mod;
    RedrawWindow();
}
