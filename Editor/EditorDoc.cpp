// EditorDoc.cpp : implementation of the CEditorDoc class
//

#include "stdafx.h"
#include "Editor.h"

#include "EditorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorDoc

IMPLEMENT_DYNCREATE(CEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CEditorDoc, CDocument)
	//{{AFX_MSG_MAP(CEditorDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorDoc construction/destruction

CEditorDoc::CEditorDoc()
{
	// TODO: add one-time construction code here

}

CEditorDoc::~CEditorDoc()
{
}

BOOL CEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

    //reset the tower
    for( int iRing = 0; iRing < MAX_TOWER_HEIGHT; iRing++ )
        for( int iBrick = 0; iBrick < BRICKS_PER_RING; iBrick++ )
            m_Tower[iRing][iBrick] = B_NORM;

    //add a start position
    m_Tower[1][0] = B_PLAT;
    m_Tower[2][0] = B_STRT;

    //reset the monsters
    while( !m_Monsters.IsEmpty() ) m_Monsters.RemoveHead();

    //reset texture name
    m_strTextures = "";

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CEditorDoc serialization

void CEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CEditorDoc diagnostics

#ifdef _DEBUG
void CEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEditorDoc commands


BOOL CEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
    OnNewDocument();

    CFile file;
    if( !file.Open( lpszPathName, CFile::modeRead ) ) return FALSE;

    TOWERFILEHEADER header;
    if( !file.Read( &header, sizeof(header) ) ) return FALSE;

    if( memcmp( header.chMagic, "TOWR", 4 ) != 0 ) return FALSE;

    for( int iMonster = 0; iMonster < header.nMonsters; iMonster++ )
    {
        FILEMONSTER monster;
        if( !file.Read( &monster, sizeof(monster) ) ) return FALSE;
        monster.iRing = (header.nTowerHeight - monster.iRing)-1;
        m_Monsters.AddTail( monster );
    }

    int iRing = header.nTowerHeight;
    while( iRing-- )
    {
        if( !file.Read( m_Tower[iRing], sizeof(TowerRing) ) ) return FALSE;
    }

    char szBuf[9]; szBuf[8] = '\0';
    memcpy( szBuf, header.szTextures, 8 );
    m_strTextures = szBuf;
	
	return TRUE;
}

BOOL CEditorDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
    //build header
    TOWERFILEHEADER header;
    ZeroMemory( &header, sizeof(header) );
    memcpy( header.chMagic, "TOWR", 4 );
    header.nMonsters = m_Monsters.GetCount();
    header.nTowerHeight = GetTowerHeight();
    memcpy( header.szTextures, LPCSTR(m_strTextures), min(8,m_strTextures.GetLength()) );

    if( header.nTowerHeight == 0 )
    {
        AfxMessageBox( "The tower has zero height! cannot save" );
        return FALSE;
    }

    //open file
	CFile file;
    if( !file.Open( lpszPathName, CFile::modeCreate|CFile::modeWrite ) ) return FALSE;

    //write header
    file.Write( &header, sizeof(header) );

    //write monsters
    POSITION p = m_Monsters.GetHeadPosition();
    while( p )
    {
        FILEMONSTER monster = m_Monsters.GetNext(p);
        monster.iRing = GetTowerHeight() - (monster.iRing+1);
        file.Write( &monster, sizeof(FILEMONSTER) );
    }

    //write tower
    int iRing = header.nTowerHeight;
    while( iRing-- )
    {
        file.Write( m_Tower[iRing], sizeof(TowerRing) );
    }

    file.Close();
	
	return TRUE;
}

int CEditorDoc::GetTowerHeight()
{
    int nHeight = 0;
    int iRing;
    for( iRing = 0; iRing < MAX_TOWER_HEIGHT; iRing++ )
    {
        for( int iBrick = 0; iBrick < BRICKS_PER_RING; iBrick++ )
        {
            if( (m_Tower[iRing][iBrick] & B_TYPE_MASK) == B_EXIT )
            {
                nHeight = iRing + 1;
            }
        }
    }

    return nHeight;
}
