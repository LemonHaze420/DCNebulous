

#define _KM_USE_VERTEX_MACRO_L5_
#include <math.h>
#include <shinobi.h>
#include <kamui2.h>
#include <kamuix.h>
#include <sn_fcntl.h>
#include <usrsnasm.h>
#include <sg_pdms.h>
#include <private.h>
#include "dcinit.h"
#include "tower.h"
#include "dispstr.h"

#define DRAW_TOWER_WALLS
#define DANGEROUS_MONSTERS
//#define USE_TEST_LEVEL

#define PI 3.1415926536f
#define RadiansToDegrees( r ) ( r * 57.2957795130f )
#define DegreesToRadians( d ) ( d * 0.01745329252f )

#define Z_BOOST     (0.00002f)
#define Z_BUMPBOOST (0.00001f)

#define min(a,b)((a<b)?a:b)
#define max(a,b)((a>b)?a:b)
#define sign(v)(v<0?-1:((v>0)?1:0))

static KMSURFACEDESC kmsdBrick, kmsdBrickBump, kmsdPlat, kmsdLift, kmsdRoof, kmsdWater[10], kmsdDoor, kmsdSprite, kmsdTest, kmsdStalk, kmsdBubble, kmsdSplash, kmsdPuff, kmsdSky, kmsdFragment, kmsdShoot;
static KMSTRIPHEAD   kmshBrick, kmshBrickBump, kmshPlat, kmshLift, kmshRoof, kmshWater, kmshDoor, kmshSprite, kmshTest, kmshShadow, kmshStalk, kmshBubble, kmshSplash, kmshPuff, kmshSky, kmshFragment, kmshShoot;

static KMSURFACEDESC kmsdRover, kmsdRoverBlack, kmsdSimple, kmsdSpinny;
static KMSTRIPHEAD kmshRover, kmshRoverBlack, kmshSimple, kmshSpinny;

static KMSURFACEDESC kmsdPlayer;
static KMSTRIPHEAD kmshPlayer;

static KMDWORD g_dwBumpLight;


#define DIRECTIONAL_LIGHT_AMOUNT   0.25f

static float fInnerRingX[BRICKS_PER_RING];
static float fInnerRingZ[BRICKS_PER_RING];
static float fOuterRingX[BRICKS_PER_RING];
static float fOuterRingZ[BRICKS_PER_RING];
static KMDWORD dwColours[BRICKS_PER_RING];
static float g_fTowerAngle;
static float g_fHeight;

static float fMiniInnerRingX[BRICKS_PER_RING][2];
static float fMiniInnerRingZ[BRICKS_PER_RING][2];
static float fMiniOuterRingX[BRICKS_PER_RING][2];
static float fMiniOuterRingZ[BRICKS_PER_RING][2];
static KMDWORD dwMiniColours[BRICKS_PER_RING][2];

unsigned long int g_nCurrentTime = 0;
unsigned long int g_nBestTime = 7200; //~2 min

struct BubbleData
{
    float x, y, z;
    int size;
} g_BubbleData[MAX_BUBBLES];

struct SplashData
{
    float x, z;
    int length;
} g_SplashData[MAX_SPLASHES];

struct PuffData
{
    float x, y, z;
    float size;
    int length;
    KMDWORD dwColour;
} g_PuffData[MAX_PUFFS];

struct FragmentData
{
    float x, y, z;
    float w, h;
    float dx, dy;
    int length;
    KMSTRIPHEAD* pkmsh;
} g_FragmentData[MAX_FRAGMENTS];


#define LIFT_STATE_DOWN         0
#define LIFT_STATE_MOVEUP       1
#define LIFT_STATE_UP           2
#define LIFT_STATE_MOVEDOWN     3
#define LIFT_STATE_DISABLED     4
#define LIFT_STATE_INVISIBLE    5

#define LIFT_STATE_MOVE_SPEED   1
#define LIFT_STATE_DROP_SPEED   8
#define LIFT_STATE_WAIT_TIME    300

struct LiftData
{
    TowerBrick* pBrick;
    float y;
    float base_y;
    float height;
    float dy;
    int iBrick;
    int iRing;

    int state;
    int nStateTime;

} g_LiftData[MAX_LIFTS];
static int g_nLiftCount = 0;

#define MONSTER_STATE_ALIVE         0
#define MONSTER_STATE_DIEJUMP       1
#define MONSTER_STATE_DROWN         2
#define MONSTER_STATE_INVISIBLE     3
#define MONSTER_STATE_ATTACK        4
#define MONSTER_STATE_STUN          5

#define MONSTER_STATE_STUN_TIME     80

struct MonsterData
{
    int state, stateCount;
    float dx, dy;
    float x, y, z;
    int iBrick, iRing;
    float default_dx, default_dy;
    unsigned char type;
} g_MonsterData[MAX_MONSTERS];
static int g_nMonsterCount = 0;


TowerRing* TheTower = NULL;
TowerRing* DefaultTower = NULL;

/*
TowerRing DefaultTower[] = {
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_EXIT },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_CRAK,B_PLAT,B_PLAT,B_PLAT,B_PLAT,B_STOP,B_PLAT,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_CRAK,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_CRAK,B_SUPP,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_CRAK,B_SUPP,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_LIFT,B_SUPP,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_PLAT,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_SUPP,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_DOOR,B_SUPP,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_MVLF,B_PLAT,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_PLAT },
{ B_NORM,B_NORM,B_NORM,B_MVLF,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_PLAT,B_PLAT,B_MVLF,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_STRT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_PLAT,B_PLAT,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_PLAT,B_PLAT,B_PLAT,B_PLAT,B_PLAT,B_PLAT,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },
{ B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM,B_NORM },

{ B_NULL },
};
*/

float g_fTextureWidth, g_fTextureHeight;
int g_nTowerHeight;
int g_nCurrentTowerHeight;


KMDWORD ScaleRGB( KMDWORD argb, float fValue )
{
    unsigned char r, g, b;
    r = (argb >> 16) & 0xFF;
    g = (argb >> 8 ) & 0xFF;
    b = (argb) & 0xFF;
    return( (argb & 0xFF000000) | ((unsigned char)((float)r * fValue)) << 16 | ((unsigned char)((float)g * fValue)) << 8 | ((unsigned char)((float)b * fValue)) );
}

KMDWORD AdjustColour( KMDWORD argb, KMDWORD argbdelta )
{
    unsigned char a, r, g, b;
    a = min( ((argb >> 24)&0xFF) + ((argbdelta >> 24)&0xFF), 0xFF );
    r = min( ((argb >> 16)&0xFF) + ((argbdelta >> 16)&0xFF), 0xFF );
    g = min( ((argb >> 8 )&0xFF) + ((argbdelta >> 8 )&0xFF), 0xFF );
    b = min( ((argb      )&0xFF) + ((argbdelta      )&0xFF), 0xFF );
    return (a<<24) | (r<<16) | (g<<8) | (b);

}

void GetPosition( float xpos, float ypos, float zpos, float* x, float* y, float* z, KMDWORD* pdwColour, KMDWORD dwOffset )
{
    float pos = GetTowerAngle(-xpos) + g_fTowerAngle;
    unsigned char col;

    //calculate position
    *y = g_fHeight - ypos;
    *x = (int)     320.0f - ( cos(pos) * zpos );
    *z = 1.0f / (((320.0f - ( sin(pos) * zpos )) / 10000.0f) + 1.0f);

    //calculate colour, if requested
    if( pdwColour )
    {
        register float fScale = (2*PI) / (float)TOWER_CIRCUMFERENCE;
        col = ((unsigned char)(/*sin( fmod(fAngle, PI))* */255) * (0.4f)) + ((unsigned char)(sin( (fmod((xpos*fScale), PI*2.0f))/2.0f)*255) * 0.4f);
        *pdwColour = AdjustColour( 0xFF000000 | col << 16 | col << 8 | col, dwOffset );

        //col = GetSmoothLightLevel( pos, xpos );
        if( ypos < 0.0f ) 
        {
            register float fScale = ((-ypos-16) / 64.0f);
            *pdwColour = ScaleRGB( *pdwColour, 1.0f - min(max(fScale,0.0f),1.0f) );
            //col *= 1.0f - min(max(fScale,0.0f),1.0f);
        }
    }
}

void TranslateVertices( float* x, float* y, float* z, KMDWORD* pdwColour, int nVertices )
{/*
    while( nVertices-- )
    {
        float xpos = x[nVertices], ypos = y[nVertices], zpos = z[nVertices];
        GetPosition( xpos, ypos, zpos, &x[nVertices], &y[nVertices], &z[nVertices], pdwColour ? &pdwColour[nVertices] : NULL, 0L );
    }
*/

    register float fScale = (2*PI) / (float)TOWER_CIRCUMFERENCE;
    unsigned char col;

    while( nVertices-- )
    {
        float pos = GetTowerAngle(-*x) + g_fTowerAngle;

        //calculate colour, if requested
        if( pdwColour )
        {
            col = ((unsigned char)(255) * (0.4f)) + ((unsigned char)(sin( (fmod(((*x)*fScale), PI*2.0f))/2.0f)*255) * 0.4f);
            //*pdwColour = AdjustColour( 0xFF000000 | col << 16 | col << 8 | col, dwOffset );
            *pdwColour = 0xFF000000 | col << 16 | col << 8 | col;

            //col = GetSmoothLightLevel( pos, xpos );
            if( (*y) < 0.0f ) 
            {
                register float fScale = ((-(*y)-16) / 64.0f);
                *pdwColour = ScaleRGB( *pdwColour, 1.0f - min(max(fScale,0.0f),1.0f) );
                //col *= 1.0f - min(max(fScale,0.0f),1.0f);
            }
        }

        //calculate position [fixme: use inline instruction!]
        *y = g_fHeight - *y;
        *x = (int)     320.0f - ( cos(pos) * (*z) );
        *z = 1.0f / (((320.0f - ( sin(pos) * (*z) )) / 10000.0f) + 1.0f);

        //advance to next
        x++;
        y++;
        z++;
        if( pdwColour ) pdwColour++;
    }
}




void DrawTime( int n )
{
    register int nn = n / 60;
    txt_DispString( "%d:%02d", nn/60, nn%60 );
}

void DrawHUD( PLAYER* pPlayer )
{
    txt_ClearScreen();
    txt_SetColor( 0.75f, 0.75f, 0.75f );

    if( pPlayer->GameState == GAME_STATE_RESCUE )
    {
        txt_SetCursor( 25, 6 );
        txt_DispString( "Your Time: " ); DrawTime( g_nCurrentTime );
        txt_SetCursor( 25, 7 );
        txt_DispString( "Best Time: " ); DrawTime( g_nBestTime );

        txt_SetCursor( 22, 20 );
        txt_DispString( "Press Start To Reset" );
    }
    else
    {
        if( pPlayer->GameState == GAME_STATE_GETREADY )
        {
            if( pPlayer->GameStateCount < (GAME_STATE_GETREADY_TIME>>2) )
            {
                txt_SetColor( 0.0f, 0.75f, 0.0f );
                txt_SetCursor( 30, 10 );
                txt_DispString( "GO!" );
            }
            else
            {
                txt_SetBlink(1);
                txt_SetBlinkSpeed( 15 );
                txt_SetColor( 0.0f, 0.0f, 0.75f );
                txt_SetCursor( 27, 10 );
                txt_DispString( "Get Ready!" );
                txt_SetBlink(0);
            }
        }
        else
        if( pPlayer->GameState != GAME_STATE_COLLAPSE && pPlayer->GameState != GAME_STATE_DONEEXIT )
        {
            txt_SetCursor( 30, 0 ); //1,0
            DrawTime( g_nCurrentTime );
        }
    }


    txt_DispScreen( &VertexBufferDesc );
}

void DrawSky( float fOffset, float y, float centerx )
{
    register float u = fOffset/(PI*2.0f) * 2.0f;
    register float z = 0.00001f;
    //register float z2 = 0.00002f;

    y = (-y / ((float)(GetTowerHeight()<<1))) - 0.5f;

	kmxxGetCurrentPtr( &VertexBufferDesc );
    kmxxStartStrip( &VertexBufferDesc, &kmshSky );
/*
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, 0, 480, z, u, y+0.5f, 0xFFFFFFFF, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, 0-64, 0,   z, u, y+0.0f, 0xFFFFFFFF, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, 320, 480, z2, u+0.5, y+0.5f, 0xFFFFFFFF, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, 320, 0,   z2, u+0.5, y+0.0f, 0xFFFFFFFF, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     640, 480, z, u+1.0f, y+0.5f, 0xFFFFFFFF, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, 640+64, 0,   z, u+1.0f, y+0.0f, 0xFFFFFFFF, 0x00000000 );
*/
    
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, centerx-320, 480, z, u, y+0.5f, 0xFFFFFFFF, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, centerx-320, 0,   z, u, y+0.0f, 0xFFFFFFFF, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     centerx+320, 480, z, u+0.75f, y+0.5f, 0xFFFFFFFF, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, centerx+320, 0,   z, u+0.75f, y+0.0f, 0xFFFFFFFF, 0x00000000 );

    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}

int FindLiftHeight( TowerRing* pTower, TowerRing* pThisRing, int iBrick )
{
    int nHeight = 0;

    while( pThisRing > pTower )
    {
        pThisRing--;
        nHeight++;
        if( (((*pThisRing)[iBrick]) & B_TYPE_MASK) == B_STOP ) return nHeight;
    }

    return 0;
}

BOOL LoadTower( const char* pszFilename )
{
    TOWERFILEHEADER* pHeader;
    unsigned char* pBuf;
    PKMDWORD pFile;

    pFile = LoadFile( pszFilename );
    if( pFile == NULL ) return FALSE;

    pBuf = (unsigned char*)pFile;
    pHeader = (TOWERFILEHEADER*)pBuf;  pBuf += sizeof(TOWERFILEHEADER);

    if( memcmp( pHeader->chMagic, "TOWR", 4 ) == 0 )
    {
        int i;

        //store tower height
        g_nTowerHeight = pHeader->nTowerHeight;

        //load all monsters
        g_nMonsterCount = pHeader->nMonsters;
        for( i = 0; i < g_nMonsterCount; i++ )
        {
            FILEMONSTER* pMonster = (FILEMONSTER*)pBuf;  pBuf += sizeof(FILEMONSTER);

            g_MonsterData[i].default_dx = g_MonsterData[i].default_dy = 0.0f;
            switch( pMonster->type )
            {
                case MONSTER_TYPE_UPDOWN:       g_MonsterData[i].default_dy = pMonster->delta; break;
                case MONSTER_TYPE_LEFTRIGHT:    g_MonsterData[i].default_dx = pMonster->delta; break;
                case MONSTER_TYPE_ROVER:        break;
                case MONSTER_TYPE_ROVERBADASS:  g_MonsterData[i].default_dx = pMonster->delta*1.5f; break;
                case MONSTER_TYPE_ROVERANGRY:   g_MonsterData[i].default_dx = pMonster->delta*1.5f; break;
            }
            g_MonsterData[i].type = pMonster->type;
            g_MonsterData[i].iBrick = pMonster->iBrick;
            g_MonsterData[i].iRing = pMonster->iRing;
        }

        //allocate tower
        DefaultTower = (TowerRing*)syMalloc( sizeof(TowerRing) * (g_nTowerHeight + 1) );
        memset( DefaultTower, B_NULL, sizeof(TowerRing) * (g_nTowerHeight + 1) );

        //load tower
        memcpy( DefaultTower, pBuf, sizeof(TowerRing) * g_nTowerHeight );

        //fixme: do something with the texture sub
        //...

        //clean up and return
        FreePtr( pFile );
        return TRUE;
    }

    FreePtr( pFile );
    return FALSE;
}



void InitTower()
{
    //load the textures
    LoadTexture( &kmsdPlayer, "sprites.pvr" );      InitStripContext( &kmshPlayer, &kmsdPlayer, KM_VERTEXTYPE_03, TRUE, TRUE );
    LoadTexture( &kmsdRover, "ball.pvr" );          InitStripContext( &kmshRover, &kmsdRover, KM_VERTEXTYPE_03, TRUE, TRUE );
    LoadTexture( &kmsdRoverBlack, "blakball.pvr" ); InitStripContext( &kmshRoverBlack, &kmsdRoverBlack, KM_VERTEXTYPE_03, TRUE, TRUE );
    LoadTexture( &kmsdSimple, "eye.pvr" );          InitStripContext( &kmshSimple, &kmsdSimple, KM_VERTEXTYPE_03, TRUE, TRUE );
    LoadTexture( &kmsdSpinny, "spinny.pvr" );       InitStripContext( &kmshSpinny, &kmsdSpinny, KM_VERTEXTYPE_03, TRUE, TRUE );
    
    LoadTexture( &kmsdShoot, "shotable.pvr" );       InitStripContext( &kmshShoot, &kmsdShoot, KM_VERTEXTYPE_03, FALSE, TRUE );
    
    LoadTexture( &kmsdSky, "sky2.pvr" );            InitStripContext( &kmshSky, &kmsdSky, KM_VERTEXTYPE_03, FALSE, FALSE );
    kmChangeStripFilterMode( &kmshSky, KM_IMAGE_PARAM1, KM_BILINEAR ); //just to make sure (currently not needed)
    //kmChangeStripSuperSampleMode( &kmshSky, KM_IMAGE_PARAM1, KM_TRUE ); //fixme: is this needed?
    kmChangeStripDepthCompareMode( &kmshSky, KM_ALWAYS );

    LoadTexture( &kmsdBrick, "bricks.pvr" );    InitStripContext( &kmshBrick, &kmsdBrick, KM_VERTEXTYPE_03, FALSE, FALSE );
    LoadTexture( &kmsdPlat,  "plat.pvr" );     InitStripContext( &kmshPlat, &kmsdPlat, KM_VERTEXTYPE_03, FALSE, FALSE );

    LoadTexture( &kmsdLift,  "lift.pvr" );      InitStripContext( &kmshLift, &kmsdLift, KM_VERTEXTYPE_03, FALSE, FALSE );
    LoadTexture( &kmsdRoof,  "roof.pvr" );      InitStripContext( &kmshRoof, &kmsdRoof, KM_VERTEXTYPE_03, FALSE, FALSE );
    LoadTexture( &kmsdStalk, "stalk.pvr" );     InitStripContext( &kmshStalk, &kmsdStalk, KM_VERTEXTYPE_03, FALSE, FALSE );
    LoadTexture( &kmsdSprite, "ball.pvr" );     InitStripContext( &kmshSprite, &kmsdSprite, KM_VERTEXTYPE_03, TRUE, TRUE );
    LoadTexture( &kmsdPuff, "puff.pvr" );       InitStripContext( &kmshPuff, &kmsdPuff, KM_VERTEXTYPE_03, TRUE, TRUE );
    kmChangeStripDepthCompareMode( &kmshPuff, KM_ALWAYS );
    //kmChangeStripBlendingMode( &kmshPuff, KM_IMAGE_PARAM1, KM_SRCALPHA, KM_ONE );
    LoadTexture( &kmsdSplash, "splash1.pvr" );  InitStripContext( &kmshSplash, &kmsdSplash, KM_VERTEXTYPE_03, TRUE, TRUE );
    kmChangeStripBlendingMode( &kmshSplash, KM_IMAGE_PARAM1, KM_SRCALPHA, KM_ONE );
    kmChangeStripZWriteDisable( &kmshSplash, KM_TRUE );
    kmChangeStripDepthCompareMode( &kmshSplash, KM_ALWAYS );
    LoadTexture( &kmsdBubble, "bubble.pvr" );   InitStripContext( &kmshBubble, &kmsdBubble, KM_VERTEXTYPE_03, TRUE, TRUE );
    kmChangeStripBlendingMode( &kmshBubble, KM_IMAGE_PARAM1, KM_SRCALPHA, KM_ONE );
    kmChangeStripSuperSampleMode( &kmshBubble, KM_IMAGE_PARAM1, KM_TRUE );
    LoadTexture( &kmsdTest, "check.pvr" );      InitStripContext( &kmshTest, &kmsdTest, KM_VERTEXTYPE_03, FALSE, TRUE );
    LoadTexture( &kmsdWater[0], "sea0.pvr" );   InitStripContext( &kmshWater, &kmsdWater[0], KM_VERTEXTYPE_03, TRUE, FALSE );
    LoadTexture( &kmsdWater[1], "sea1.pvr" );
    LoadTexture( &kmsdWater[2], "sea2.pvr" );
    LoadTexture( &kmsdWater[3], "sea3.pvr" );
    LoadTexture( &kmsdWater[4], "sea4.pvr" );
    LoadTexture( &kmsdWater[5], "sea5.pvr" );
    LoadTexture( &kmsdWater[6], "sea6.pvr" );
    LoadTexture( &kmsdWater[7], "sea7.pvr" );
    LoadTexture( &kmsdWater[8], "sea8.pvr" );
    LoadTexture( &kmsdWater[9], "sea9.pvr" );
    kmChangeStripZWriteDisable( &kmshWater, KM_TRUE );

    LoadTexture( &kmsdDoor, "door.pvr" );       InitStripContext( &kmshDoor, &kmsdDoor, KM_VERTEXTYPE_03, TRUE, TRUE );
    kmChangeStripDepthCompareMode( &kmshDoor, KM_GREATEREQUAL );
    kmChangeStripZWriteDisable( &kmshDoor, KM_TRUE );

    InitStripContext( &kmshShadow, NULL, KM_VERTEXTYPE_01, TRUE, FALSE );
    kmChangeStripDepthCompareMode( &kmshShadow, KM_GREATEREQUAL );
    kmChangeStripZWriteDisable( &kmshShadow, KM_TRUE );

    LoadTexture( &kmsdBrickBump, "bbricks.pvr" );    InitStripContext( &kmshBrickBump, &kmsdBrickBump, KM_VERTEXTYPE_03, TRUE, FALSE );
    kmChangeStripOffset( &kmshBrickBump, KM_TRUE );

    LoadTexture( &kmsdFragment,  "platfrag.pvr" );      InitStripContext( &kmshFragment, &kmsdFragment, KM_VERTEXTYPE_03, FALSE, FALSE );



    //allocate tower
#ifdef USE_TEST_LEVEL
    LoadTower( "test.twr" );
#else
    LoadTower( "level1.twr" );
#endif
    //g_nTowerHeight = 38;

    TheTower = (TowerRing*)syMalloc( sizeof(TowerRing) * (g_nTowerHeight+1) );

    //initialise tower
    ResetTower();
}

void DestroyTower()
{
    //free the tower
    syFree( TheTower );
    syFree( DefaultTower );

    //free all textures
    kmFreeTexture( &kmsdRover );
    kmFreeTexture( &kmsdRoverBlack );
    kmFreeTexture( &kmsdSimple );
    kmFreeTexture( &kmsdSpinny );
    
    kmFreeTexture( &kmsdSky );

    kmFreeTexture( &kmsdBrick );
    kmFreeTexture( &kmsdBrickBump );
    kmFreeTexture( &kmsdPlat );
    kmFreeTexture( &kmsdLift );
    kmFreeTexture( &kmsdRoof );
    kmFreeTexture( &kmsdStalk );
    kmFreeTexture( &kmsdSprite );
    kmFreeTexture( &kmsdPuff );
    kmFreeTexture( &kmsdSplash );
    kmFreeTexture( &kmsdBubble );
    kmFreeTexture( &kmsdTest );
    kmFreeTexture( &kmsdWater[0] );
    kmFreeTexture( &kmsdWater[1] );
    kmFreeTexture( &kmsdWater[2] );
    kmFreeTexture( &kmsdWater[3] );
    kmFreeTexture( &kmsdWater[4] );
    kmFreeTexture( &kmsdWater[5] );
    kmFreeTexture( &kmsdWater[6] );
    kmFreeTexture( &kmsdWater[7] );
    kmFreeTexture( &kmsdWater[8] );
    kmFreeTexture( &kmsdWater[9] );
    kmFreeTexture( &kmsdDoor );
    kmFreeTexture( &kmsdFragment );
}


void GetScreenPos( float* x, float* z, float fAngle, float fRadius )
{
    float s, c;
    fsca( (long)(fAngle * 10430.378350470453f), &s, &c );
    *x = 320.0f - c*fRadius;
    *z = 1.0f / (((320.0f - s*fRadius ) / 10000.0f) + 1.0f);
}

float GetZScaling( float z )
{
    //float def_pos = 1.0f / ((((((TOWER_RADIUS+PLATFORM_WIDTH)-16.0f)) ) / 10000.0f) + 1.0f);

    //*z = 1.0f / (((320.0f - ( sin(pos) * zpos )) / 10000.0f) + 1.0f);

    //FIXME
    return 1.0f;
/*

    float def_pos = (((1.0f/z)-1.0f)*10000.0f) - 320.0f;


    float pos = def_pos / ((TOWER_RADIUS+PLATFORM_WIDTH)-16.0f);
    return pos;*/
}

unsigned char GetLightLevel( float fAngle, int iBrick )
{
    register float fScale = (2*PI) / (float)BRICKS_PER_RING;
    return ((unsigned char)(sin( fmod(fAngle, PI))*255) * (1.0f - DIRECTIONAL_LIGHT_AMOUNT)) + ((unsigned char)(sin( (fmod((iBrick*fScale), PI*2.0f))/2.0f)*255) * DIRECTIONAL_LIGHT_AMOUNT);
}

unsigned char GetSmoothLightLevel( float fAngle, float x )
{
    register float fScale = (2*PI) / (float)TOWER_CIRCUMFERENCE;
    return ((unsigned char)(sin( fmod(fAngle, PI))*255) * (1.0f - DIRECTIONAL_LIGHT_AMOUNT)) + ((unsigned char)(sin( (fmod((x*fScale), PI*2.0f))/2.0f)*255) * DIRECTIONAL_LIGHT_AMOUNT);
}

void SetTowerPosition( float y, float angle )
{
    static float s_oldangle = -666.0f;
    int iBrick;
    float pos;
    KMDWORD dwColour;
    unsigned char col;
    float fScale = (2*PI) / (float)BRICKS_PER_RING;
    float fRadius = TOWER_RADIUS;

    g_fHeight = y + 360.0f;

    if( angle != s_oldangle )
    {
        g_fTowerAngle = angle + (PI*0.5f);
        for( iBrick = 0; iBrick < BRICKS_PER_RING; iBrick++ )
        {
            //calculate values
            float pos = (iBrick * fScale) + g_fTowerAngle;
            float x1, z1, x2, z2;
            unsigned char col = GetLightLevel( pos, iBrick );
            KMDWORD dwColour = col << 16 | col << 8 | col;

            GetScreenPos( &x1, &z1, pos, fRadius );
            GetScreenPos( &x2, &z2, pos, fRadius+PLATFORM_WIDTH );
    
            //store values
            fInnerRingX[iBrick] = (int)x1;
            fInnerRingZ[iBrick] = z1;
            fOuterRingX[iBrick] = (int)x2;
            fOuterRingZ[iBrick] = z2;
            dwColours[iBrick] = 0xFF000000 | dwColour;
        
            //fixme... this is evil
            pos = (((float)iBrick+0.30f) * fScale) + g_fTowerAngle;

            GetScreenPos( &x1, &z1, pos, (fRadius+((float)PLATFORM_WIDTH * 0.30f)) );
            GetScreenPos( &x2, &z2, pos, (fRadius+PLATFORM_WIDTH-((float)PLATFORM_WIDTH * 0.30f)) );
            col = GetSmoothLightLevel( pos, (float)(iBrick*BRICK_SIZE) + ((float)BRICK_SIZE*0.30f) ) * 0.75f;
            dwColour = col << 16 | col << 8 | col;
    
            //store values
            fMiniInnerRingX[iBrick][0] = (int)x1;
            fMiniInnerRingZ[iBrick][0] = z1;
            fMiniOuterRingX[iBrick][0] = (int)x2;
            fMiniOuterRingZ[iBrick][0] = z2;
            dwMiniColours[iBrick][0] = 0xFF000000 | dwColour;

            pos = (((float)iBrick+0.70f) * fScale) + g_fTowerAngle;

            GetScreenPos( &x1, &z1, pos, (fRadius+((float)PLATFORM_WIDTH * 0.30f)) );
            GetScreenPos( &x2, &z2, pos, (fRadius+PLATFORM_WIDTH-((float)PLATFORM_WIDTH * 0.30f)));
            col = GetSmoothLightLevel( pos, (float)(iBrick*BRICK_SIZE) + ((float)BRICK_SIZE*0.70f) ) * 0.75f;
            dwColour = col << 16 | col << 8 | col;
    
            //store values
            fMiniInnerRingX[iBrick][1] = (int)x1;
            fMiniInnerRingZ[iBrick][1] = z1;
            fMiniOuterRingX[iBrick][1] = (int)x2;
            fMiniOuterRingZ[iBrick][1] = z2;
            dwMiniColours[iBrick][1] = 0xFF000000 | dwColour;
        }
    }
    else
        s_oldangle = angle;
}


void DrawMultiPlat( float y, int iRing, int iBrick, KMSTRIPHEAD* pkmsh, unsigned short int brickMod )
{
    int iNextBrick = (iBrick + 1) % BRICKS_PER_RING;
    float u = 0.0f, v = 0.0f;
    int iTex = iBrick + iRing;

    u = (float)( iTex & 3) / 4.0f;
    v = (float)((iTex>>2)&3) / 8.0f;

    if( brickMod & B_CRACKING ) v += 0.5f;

	kmxxGetCurrentPtr( &VertexBufferDesc );
    kmxxStartStrip( &VertexBufferDesc, pkmsh );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y+RING_HEIGHT, fInnerRingZ[iBrick], u, v+0.125f, 0x00000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y,             fInnerRingZ[iBrick], u, v+0.0f, 0x00000000, 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iBrick], y+RING_HEIGHT, fOuterRingZ[iBrick], u+0.25f, v+0.125f, dwColours[iBrick], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iBrick], y,             fOuterRingZ[iBrick], u+0.25f, v+0.0f, dwColours[iBrick], 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iNextBrick], y+RING_HEIGHT, fOuterRingZ[iNextBrick], u, v+0.125f, dwColours[iNextBrick], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iNextBrick], y,             fOuterRingZ[iNextBrick], u, v+0.0f, dwColours[iNextBrick], 0x00000000 );


    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iNextBrick], y+RING_HEIGHT, fInnerRingZ[iNextBrick], u+0.25f, v+0.125f, 0x00000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[iNextBrick], y,             fInnerRingZ[iNextBrick], u+0.25f, v+0.0f, 0x00000000, 0x00000000 );


    kmxxStartStrip( &VertexBufferDesc, &kmshShadow );
    kmxxSetVertex_1( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y+(RING_HEIGHT*2.0f),         Z_BOOST+fInnerRingZ[iBrick],        0.0f, 0.0f, 0.0f, 0.0f );
    kmxxSetVertex_1( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y+RING_HEIGHT,             Z_BOOST+fInnerRingZ[iBrick],        0.5f, 0.0f, 0.0f, 0.0f );
    kmxxSetVertex_1( KM_VERTEXPARAM_NORMAL, fInnerRingX[iNextBrick], y+(RING_HEIGHT*2.0f),     Z_BOOST+fInnerRingZ[iNextBrick],    0.0f, 0.0f, 0.0f, 0.0f );
    kmxxSetVertex_1( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[iNextBrick], y+RING_HEIGHT,     Z_BOOST+fInnerRingZ[iNextBrick],    0.5f, 0.0f, 0.0f, 0.0f );

    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}


void DrawMiniMultiPlat( float y, int iRing, int iBrick, KMSTRIPHEAD* pkmsh )
{
    float u = 0.0f, v = 0.0f;
    int iTex = (iBrick * BRICKS_PER_RING) + iRing;

    u = (float)( iTex & 3) / 4.0f;
    v = (float)((iTex>>2)&3) / 8.0f;

	kmxxGetCurrentPtr( &VertexBufferDesc );
    kmxxStartStrip( &VertexBufferDesc, pkmsh );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniInnerRingX[iBrick][0], y+RING_HEIGHT, fMiniInnerRingZ[iBrick][0], u, v+0.125f, 0x000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniInnerRingX[iBrick][0], y,             fMiniInnerRingZ[iBrick][0], u, v, 0x000000, 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][0], y+RING_HEIGHT, fMiniOuterRingZ[iBrick][0], u+0.25f, v+0.125f, dwMiniColours[iBrick][0], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][0], y,             fMiniOuterRingZ[iBrick][0], u+0.25f, v, dwMiniColours[iBrick][0], 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][1], y+RING_HEIGHT, fMiniOuterRingZ[iBrick][1], u, v+0.125f, dwMiniColours[iBrick][1], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][1], y,             fMiniOuterRingZ[iBrick][1], u, v, dwMiniColours[iBrick][1], 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     fMiniInnerRingX[iBrick][1], y+RING_HEIGHT, fMiniInnerRingZ[iBrick][1], u+0.25f, v+0.125f, 0x000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fMiniInnerRingX[iBrick][1], y,             fMiniInnerRingZ[iBrick][1], u+0.25f, v, 0x000000, 0x00000000 );

    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}




void DrawPlat( float y, int iBrick, KMSTRIPHEAD* pkmsh, unsigned short int brickMod/*, KMSTRIPHEAD* pkmshBump*/ )
{
    int iNextBrick = (iBrick + 1) % BRICKS_PER_RING;
    float v = 0.0f;
    if( brickMod & B_CRACKING )
        v += 0.5f;

	kmxxGetCurrentPtr( &VertexBufferDesc );
    kmxxStartStrip( &VertexBufferDesc, pkmsh );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y+RING_HEIGHT, fInnerRingZ[iBrick], 0.5f, v+0.5f, 0x00000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y,             fInnerRingZ[iBrick], 0.5f, v+0.0f, 0x00000000, 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iBrick], y+RING_HEIGHT, fOuterRingZ[iBrick], 0.0f, v+0.5f, dwColours[iBrick], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iBrick], y,             fOuterRingZ[iBrick], 0.0f, v+0.0f, dwColours[iBrick], 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iNextBrick], y+RING_HEIGHT, fOuterRingZ[iNextBrick], 1.0f, v+0.5f, dwColours[iNextBrick], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iNextBrick], y,             fOuterRingZ[iNextBrick], 1.0f, v+0.0f, dwColours[iNextBrick], 0x00000000 );


#ifdef DRAW_TOWER_WALLS
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iNextBrick], y+RING_HEIGHT, fInnerRingZ[iNextBrick], 0.5f, v+0.5f, 0x00000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[iNextBrick], y,             fInnerRingZ[iNextBrick], 0.5f, v+0.0f, 0x00000000, 0x00000000 );


    kmxxStartStrip( &VertexBufferDesc, &kmshShadow );
    kmxxSetVertex_1( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y+(RING_HEIGHT*2.0f),         Z_BOOST+fInnerRingZ[iBrick],        0.0f, 0.0f, 0.0f, 0.0f );
    kmxxSetVertex_1( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y+RING_HEIGHT,             Z_BOOST+fInnerRingZ[iBrick],        0.5f, 0.0f, 0.0f, 0.0f );
    kmxxSetVertex_1( KM_VERTEXPARAM_NORMAL, fInnerRingX[iNextBrick], y+(RING_HEIGHT*2.0f),     Z_BOOST+fInnerRingZ[iNextBrick],    0.0f, 0.0f, 0.0f, 0.0f );
    kmxxSetVertex_1( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[iNextBrick], y+RING_HEIGHT,     Z_BOOST+fInnerRingZ[iNextBrick],    0.5f, 0.0f, 0.0f, 0.0f );
#else
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iNextBrick], y+RING_HEIGHT, fInnerRingZ[iNextBrick], 3.5f, v+0.5f, 0x00000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iNextBrick], y,             fInnerRingZ[iNextBrick], 3.5f, v+0.0f, 0x00000000, 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y+RING_HEIGHT, fInnerRingZ[iBrick], 4.0f, v+0.5f, 0x00000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[iBrick], y,             fInnerRingZ[iBrick], 4.0f, v+0.0f, 0x00000000, 0x00000000 );


#endif //DRAW_TOWER_WALLS
/*
    if( pkmshBump )
    {
        kmxxStartStrip( &VertexBufferDesc, pkmshBump );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y+RING_HEIGHT, fInnerRingZ[iBrick]+Z_BUMPBOOST, 0.5f, v+0.5f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y,             fInnerRingZ[iBrick]+Z_BUMPBOOST, 0.5f, v+0.0f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iBrick], y+RING_HEIGHT, fOuterRingZ[iBrick]+Z_BUMPBOOST, 0.0f, v+0.5f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iBrick], y,             fOuterRingZ[iBrick]+Z_BUMPBOOST, 0.0f, v+0.0f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iNextBrick], y+RING_HEIGHT, fOuterRingZ[iNextBrick]+Z_BUMPBOOST, 1.0f, v+0.5f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[iNextBrick], y,             fOuterRingZ[iNextBrick]+Z_BUMPBOOST, 1.0f, v+0.0f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iNextBrick], y+RING_HEIGHT, fInnerRingZ[iNextBrick]+Z_BUMPBOOST, 0.5f, v+0.5f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[iNextBrick], y,             fInnerRingZ[iNextBrick]+Z_BUMPBOOST, 0.5f, v+0.0f, 0xA0000000, g_dwBumpLight );
    }
*/
    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}

void DrawMiniPlat( float y, int iBrick, KMSTRIPHEAD* pkmsh/*, KMSTRIPHEAD* pkmshBump*/ )
{
	kmxxGetCurrentPtr( &VertexBufferDesc );
    kmxxStartStrip( &VertexBufferDesc, pkmsh );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniInnerRingX[iBrick][0], y+RING_HEIGHT, fMiniInnerRingZ[iBrick][0], 0.0f, 0.5f, 0x000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniInnerRingX[iBrick][0], y,             fMiniInnerRingZ[iBrick][0], 0.0f, 0.0f, 0x000000, 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][0], y+RING_HEIGHT, fMiniOuterRingZ[iBrick][0], 0.25f, 0.5f, dwMiniColours[iBrick][0], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][0], y,             fMiniOuterRingZ[iBrick][0], 0.25f, 0.0f, dwMiniColours[iBrick][0], 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][1], y+RING_HEIGHT, fMiniOuterRingZ[iBrick][1], 0.5f, 0.5f, dwMiniColours[iBrick][1], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][1], y,             fMiniOuterRingZ[iBrick][1], 0.5f, 0.0f, dwMiniColours[iBrick][1], 0x00000000 );


#ifdef DRAW_TOWER_WALLS
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     fMiniInnerRingX[iBrick][1], y+RING_HEIGHT, fMiniInnerRingZ[iBrick][1], 0.75f, 0.5f, 0x000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fMiniInnerRingX[iBrick][1], y,             fMiniInnerRingZ[iBrick][1], 0.75f, 0.0f, 0x000000, 0x00000000 );
#else
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniInnerRingX[iBrick][1], y+RING_HEIGHT, fMiniInnerRingZ[iBrick][1], 0.75f, 0.5f, 0x00000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniInnerRingX[iBrick][1], y,             fMiniInnerRingZ[iBrick][1], 0.75f, 0.0f, 0x00000000, 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     fMiniInnerRingX[iBrick][0], y+RING_HEIGHT, fMiniInnerRingZ[iBrick][0], 1.0f, 0.5f, 0x00000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fMiniInnerRingX[iBrick][0], y,             fMiniInnerRingZ[iBrick][0], 1.0, 0.0f, 0x00000000, 0x00000000 );

#endif //DRAW_TOWER_WALLS
/*
    if( pkmshBump )
    {
        kmxxStartStrip( &VertexBufferDesc, pkmshBump );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniInnerRingX[iBrick][0], y+RING_HEIGHT, fMiniInnerRingZ[iBrick][0]+Z_BUMPBOOST, 0.0f, 0.5f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniInnerRingX[iBrick][0], y,             fMiniInnerRingZ[iBrick][0]+Z_BUMPBOOST, 0.0f, 0.0f, 0xA0000000, g_dwBumpLight );

        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][0], y+RING_HEIGHT, fMiniOuterRingZ[iBrick][0]+Z_BUMPBOOST, 1.0f, 0.5f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][0], y,             fMiniOuterRingZ[iBrick][0]+Z_BUMPBOOST, 1.0f, 0.0f, 0xA0000000, g_dwBumpLight );

        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][1], y+RING_HEIGHT, fMiniOuterRingZ[iBrick][1]+Z_BUMPBOOST, 0.0f, 0.5f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fMiniOuterRingX[iBrick][1], y,             fMiniOuterRingZ[iBrick][1]+Z_BUMPBOOST, 0.0f, 0.0f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     fMiniInnerRingX[iBrick][1], y+RING_HEIGHT, fMiniInnerRingZ[iBrick][1]+Z_BUMPBOOST, 1.0f, 0.5f, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fMiniInnerRingX[iBrick][1], y,             fMiniInnerRingZ[iBrick][1]+Z_BUMPBOOST, 1.0f, 0.0f, 0xA0000000, g_dwBumpLight );
    }
*/

    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}


//vertices that make up the door arch volume
#define DOOR_VERTICES   8
static float fDoorVerticesX[DOOR_VERTICES] = { 0.00f, 0.00f, 0.15f, 0.30f, 0.60f, 0.85f, 1.00f, 1.00f };
static float fDoorVerticesY[DOOR_VERTICES] = { 0.00f, 0.70f, 0.90f, 1.00f, 1.00f, 0.90f, 0.70f, 0.00f };

static float fDoorVerticesWorkX[DOOR_VERTICES*2];
static float fDoorVerticesWorkY[DOOR_VERTICES*2];
static float fDoorVerticesWorkZ[DOOR_VERTICES*2];

static KMDWORD dwDoorLighting[DOOR_VERTICES];

void DrawDoor( float y, int iBrick, BOOL bExit )
{
    int iNextBrick = (iBrick + 1) % BRICKS_PER_RING;
    int iPos, iPos2;
    //int i;
    //float xoff = ((iBrick) * BRICK_SIZE);
    //float v = 0.0f;


    y += RING_HEIGHT;

	kmxxGetCurrentPtr( &VertexBufferDesc );

    //draw door decal
    kmxxStartStrip( &VertexBufferDesc, &kmshDoor );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y,                     Z_BOOST+fInnerRingZ[iBrick],        0.0f, 1.0f, dwColours[iBrick], 0x000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iBrick], y-DOOR_HEIGHT,         Z_BOOST+fInnerRingZ[iBrick],        0.0f, 0.0f, dwColours[iBrick], 0x000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[iNextBrick], y,                 Z_BOOST+fInnerRingZ[iNextBrick],    1.0f, 1.0f, dwColours[iNextBrick], 0x000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[iNextBrick], y-DOOR_HEIGHT, Z_BOOST+fInnerRingZ[iNextBrick],    1.0f, 0.0f, dwColours[iNextBrick], 0x000000 );


    
    //build door volume
//    i = 0;
//    iPos = DOOR_VERTICES;
//    while( iPos-- )
//    {
//        fDoorVerticesWorkX[i] = xoff + (BRICK_SIZE*fDoorVerticesX[iPos]);
//        fDoorVerticesWorkY[i] = g_fHeight - y+(DOOR_HEIGHT*fDoorVerticesY[iPos]);
//        fDoorVerticesWorkZ[i] = TOWER_RADIUS+8;
//        fDoorVerticesWorkX[i+DOOR_VERTICES] = xoff + (BRICK_SIZE*fDoorVerticesX[iPos]);
//        fDoorVerticesWorkY[i+DOOR_VERTICES] = g_fHeight - y+(DOOR_HEIGHT*fDoorVerticesY[iPos]);
//        fDoorVerticesWorkZ[i+DOOR_VERTICES] = 1;
//        i++;
//    }
//
//    //translate the vertices
//    TranslateVertices( fDoorVerticesWorkX, fDoorVerticesWorkY, fDoorVerticesWorkZ, dwDoorLighting, DOOR_VERTICES*2 );
//
//    //draw the front of the door volume
//    kmxxStartStrip( &VertexBufferDesc, &kmshPlat );
//    i = DOOR_VERTICES>>1;
//    iPos = 0;
//    iPos2 = DOOR_VERTICES-1;
//    while( --i )
//    {
//        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, 0.0f, dwDoorLighting[iPos], 0x000000 );
//        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  0.0f, 0.0f, dwDoorLighting[iPos2], 0x000000 );
//        iPos++; iPos2--;
//    }
//    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, 0.0f, dwDoorLighting[iPos], 0x000000 );
//    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  0.0f, 0.0f, dwDoorLighting[iPos2], 0x000000 );
//
//    //draw the back of the door volume
//    kmxxStartStrip( &VertexBufferDesc, &kmshPlat );
//    i = DOOR_VERTICES>>1;
//    iPos = DOOR_VERTICES;
//    iPos2 = DOOR_VERTICES+DOOR_VERTICES-1;
//    while( --i )
//    {
//        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  0.0f, 0.0f, dwDoorLighting[iPos2], 0x000000 );
//        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, 0.0f, dwDoorLighting[iPos], 0x000000 );
//        iPos++; iPos2--;
//    }
//    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  0.0f, 0.0f, dwDoorLighting[iPos2], 0x000000 );
//    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, 0.0f, dwDoorLighting[iPos], 0x000000 );
//
//    //draw the door volume
//    kmxxStartStrip( &VertexBufferDesc, &kmshPlat );
//    i = DOOR_VERTICES;
//    iPos = 0;
//    iPos2 = DOOR_VERTICES;
//    while( --i )
//    {
//        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  0.0f, 0.0f, dwDoorLighting[iPos2], 0x000000 );
//        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, 0.0f, dwDoorLighting[iPos], 0x000000 );
//        iPos++; iPos2++;
//    }
//    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  0.0f, 0.0f, dwDoorLighting[iPos2], 0x000000 );
//    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, 0.0f, dwDoorLighting[iPos], 0x000000 );
//
//
//
//    //build door volume
//    i = 0;
//    iPos = DOOR_VERTICES;
//    while( iPos-- )
//    {
//        fDoorVerticesWorkX[i] = xoff + (BRICK_SIZE*fDoorVerticesX[iPos]);
//        fDoorVerticesWorkY[i] = g_fHeight - y+(DOOR_HEIGHT*fDoorVerticesY[iPos]);
//        fDoorVerticesWorkZ[i] = TOWER_RADIUS;
//        //fDoorVerticesWorkX[i+DOOR_VERTICES] = xoff + (BRICK_SIZE*fDoorVerticesX[iPos]);
//        //fDoorVerticesWorkY[i+DOOR_VERTICES] = g_fHeight - y+(DOOR_HEIGHT*fDoorVerticesY[iPos]);
//        //fDoorVerticesWorkZ[i+DOOR_VERTICES] = TOWER_RADIUS-32;
//        i++;
//    }
//
//    //translate the vertices
//    TranslateVertices( fDoorVerticesWorkX, fDoorVerticesWorkY, fDoorVerticesWorkZ, dwDoorLighting, DOOR_VERTICES );
//
///
//    if( bExit )
//    {
//        //draw the back of the door volume
//        kmxxStartStrip( &VertexBufferDesc, &kmshBrick );
//        i = DOOR_VERTICES>>1;
//        iPos = DOOR_VERTICES;
//        iPos2 = DOOR_VERTICES+DOOR_VERTICES-1;
//        while( --i )
//        {
//            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, 0.0f, 0x000000, 0x000000 );
//            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  0.0f, 0.0f, 0x000000, 0x000000 );
//            iPos++; iPos2--;
//        }
//        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, 0.0f, 0x000000, 0x000000 );
//        kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  0.0f, 0.0f, 0x000000, 0x000000 );
//     }
//
//    //draw the door volume
//    kmxxStartStrip( &VertexBufferDesc, &kmshBrick );
//    i = DOOR_VERTICES;
//    iPos = 0;
//    iPos2 = DOOR_VERTICES;
//    while( --i )
//    {
//        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, v, dwDoorLighting[iPos], 0x000000 );
//        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  2.0f, v, 0x000000, 0x000000 );
//        iPos++; iPos2++;
//        v += 0.1f;
//    }
//    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fDoorVerticesWorkX[iPos], fDoorVerticesWorkY[iPos], fDoorVerticesWorkZ[iPos],  0.0f, v, dwDoorLighting[iPos], 0x000000 );
//    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fDoorVerticesWorkX[iPos2], fDoorVerticesWorkY[iPos2], fDoorVerticesWorkZ[iPos2],  2.0f, v, 0x000000, 0x000000 );
//
//
//
//
//

    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}


int DrawRing( float y, TowerRing t, int iRing, int iLift )
{
    float fScale = (2*PI) / (float)BRICKS_PER_RING;
    float fRadius = TOWER_RADIUS;
    KMBOOL bNewStrip = TRUE;
    int nMax = BRICKS_PER_RING, iBrick = BRICKS_PER_RING-1;
    TowerBrick* pNextBrick = t;
    TowerBrick* pThisBrick = &t[BRICKS_PER_RING-1];
    TowerBrick* pPrevBrick = pThisBrick - 1;

    while( nMax-- )
    {
        switch( (*pThisBrick) & B_TYPE_MASK )
        {
            case B_MVLF:
            case B_MVRT:
            case B_CRAK:
            case B_PLAT:
                if( !((*pThisBrick) & B_TOUCH) ) DrawMultiPlat( y, iRing, iBrick, &kmshPlat, (*pThisBrick)&B_MOD_MASK );
                break;

            case B_EXIT:
                DrawDoor( y, iBrick, TRUE );
                break;

            case B_DOOR:
                DrawDoor( y, iBrick, FALSE );
                break;

            case B_SUPP:
                DrawMiniMultiPlat( y, iRing, iBrick, &kmshPlat );
                break;

            case B_SHOT:
                DrawMiniPlat( y, iBrick, &kmshShoot );
                break;
        }

        if( ((*pThisBrick) & B_TOUCH) ) DrawPlat( y, iBrick, &kmshTest, 0/*, NULL*/ );
        *pThisBrick &= ~B_TOUCH;

        if( ((*pThisBrick) & B_LIFTSTALK ) )
        {
            DrawMiniPlat( y, iBrick, &kmshStalk/*, NULL*/ );
        }

        //advance to the next brick
        if( ++iBrick == BRICKS_PER_RING ) iBrick = 0;
        pPrevBrick = pThisBrick;
        pThisBrick = pNextBrick;
        pNextBrick++;
    }

    return iLift;
}

void AddBubble( float x, float y, float z )
{
    int i, iDeepest = 0;
    float cur_y = 0;
    for( i = 0; i < MAX_BUBBLES; i++ )
    {
        if( g_BubbleData[i].size == 0 )
        {
            iDeepest = i;
            break;
        }
        else
            if( g_BubbleData[i].y < cur_y )
            {
                cur_y = g_BubbleData[i].y;
                iDeepest = i;
            }
    }

    g_BubbleData[iDeepest].size = (rand()%8)+4;
    g_BubbleData[iDeepest].x = x + (rand()&0x0F)-8;
    g_BubbleData[iDeepest].y = y;
    g_BubbleData[iDeepest].z = z + (rand()&0x0F)-8;
}

void AddSplash( float x, float z )
{
    int i;
    for( i = 0; i < MAX_SPLASHES; i++ )
    {
        if( g_SplashData[i].length == 0 )
        {
            g_SplashData[i].length = 30;
            g_SplashData[i].x = x;
            g_SplashData[i].z = z;
            return;
        }
    }
}

void AddFragment( float x, float y, float z, float w, float h, float dx, float dy, KMSTRIPHEAD* pkmsh )
{
    static int s_iFree = 0;

    g_FragmentData[s_iFree].length = 64 + (rand()&0x7F);
    g_FragmentData[s_iFree].x = x;
    g_FragmentData[s_iFree].y = y;
    g_FragmentData[s_iFree].z = z;
    g_FragmentData[s_iFree].w = w;
    g_FragmentData[s_iFree].h = h;
    g_FragmentData[s_iFree].dx = dx;
    g_FragmentData[s_iFree].dy = dy;
    g_FragmentData[s_iFree].pkmsh = pkmsh;

    s_iFree = (s_iFree + 1) % MAX_FRAGMENTS;
}

void AddPuff( float x, float y, float z, float size, KMDWORD dwColour )
{
    static int s_iFree = 0;

    g_PuffData[s_iFree].length = 15;
    g_PuffData[s_iFree].x = x;
    g_PuffData[s_iFree].y = y;
    g_PuffData[s_iFree].z = z;
    g_PuffData[s_iFree].size = size;
    g_PuffData[s_iFree].dwColour = dwColour;

    s_iFree = (s_iFree + 1) % MAX_PUFFS;
}

void UpdateBubbles()
{
    int i;
    for( i = 0; i < MAX_BUBBLES; i++ )
    {
        if( g_BubbleData[i].size != 0 )
        {
            g_BubbleData[i].y += (float)(g_BubbleData[i].size-3) * 0.5f;
            g_BubbleData[i].x += ((rand()&3)-1);
            g_BubbleData[i].z += ((rand()&3)-1);

            if( g_BubbleData[i].y > 0 ) g_BubbleData[i].size = 0;
        }
    }
}

void UpdateSplashes()
{
    int i;
    for( i = 0; i < MAX_SPLASHES; i++ )
        if( g_SplashData[i].length > 0 ) g_SplashData[i].length--;
}

void UpdateFragments()
{
    int i;
    for( i = 0; i < MAX_FRAGMENTS; i++ )
    {
        if( g_FragmentData[i].length > 0 )
        {
            g_FragmentData[i].length--;
            g_FragmentData[i].x += g_FragmentData[i].dx;
            g_FragmentData[i].y += g_FragmentData[i].dy;
            g_FragmentData[i].z += 1.0f;
            if( g_FragmentData[i].dy > -8.0f ) g_FragmentData[i].dy -= 1.0f;
        }
    }
}

void UpdatePuffs()
{
    int i;
    for( i = 0; i < MAX_PUFFS; i++ )
    {
        if( g_PuffData[i].length > 0 ) 
        {
            g_PuffData[i].length--;
            g_PuffData[i].y -= 2.0f;
            if( g_PuffData[i].y <= 0.0f ) g_PuffData[i].length = 0;
        }
    }
}

int GetTowerHeight()
{
    return g_nTowerHeight * RING_HEIGHT;
}

float GetTowerAngle( float x )
{
    if( x < 0.0f ) x += TOWER_CIRCUMFERENCE; else if( x > TOWER_CIRCUMFERENCE ) x -= TOWER_CIRCUMFERENCE;
    return ((1.0f - (x / (float)TOWER_CIRCUMFERENCE)) * PI * 2.0f);
}

float GetTowerDistanceX( float x1, float x2 )
{
    
    float fDiff = fabs(x1-x2);
    if( fDiff > (TOWER_CIRCUMFERENCE*0.5f) )
        return fabs(TOWER_CIRCUMFERENCE-fDiff);
    else
        return fDiff;
/*
    float fDiff = (x1-x2);
    if( fDiff < 0 )
    {
        if( -fDiff > (TOWER_CIRCUMFERENCE>>1) )
            return TOWER_CIRCUMFERENCE+fDiff;
        else
            return fDiff;
    }
    else
        if( fDiff > (TOWER_CIRCUMFERENCE>>1) )
            return TOWER_CIRCUMFERENCE-fDiff;
        else
            return fDiff;*/
}

signed char GetTowerDirectionX( float xfrom, float xto )
{
    int nDiff = ( (int)(xfrom-xto) );
    if( nDiff < 0 )
    {
        if( -nDiff > (TOWER_CIRCUMFERENCE>>1) )
            return -1;
        else
            return 1;
    }
    else
        if( nDiff > (TOWER_CIRCUMFERENCE>>1) )
            return 1;
        else
            return -1;
}

BOOL CenterOnNearestBrick( PLAYER* pPlayer )
{
    int desiredX;
    int nX = (int)pPlayer->x;
    int i;

    desiredX = (BRICK_SIZE * GetBrickIndex( pPlayer->x )) + (BRICK_SIZE>>1);
    if( abs(nX-desiredX) < 2 )
    {
        pPlayer->x = desiredX;
        pPlayer->bCentering = FALSE;
        return TRUE;
    }
    else
    {
        pPlayer->x = nX + (GetTowerDirectionX( (float)nX, (float)desiredX ) << 1 );
        pPlayer->bCentering = TRUE;
        return FALSE;
    }
}


int FindLift( TowerBrick* pBrick )
{
    int iLift;
    for( iLift = 0; iLift < g_nLiftCount; iLift++ )
        if( g_LiftData[iLift].pBrick == pBrick ) return iLift;
    return -1;
}

int FindLiftTop( TowerBrick* pBrick )
{
    int iLift;
    for( iLift = 0; iLift < g_nLiftCount; iLift++ )
    {
        int iRing = GetRingIndex( g_LiftData[iLift].y + g_LiftData[iLift].base_y );
        if( iRing >= 0 && &TheTower[iRing][g_LiftData[iLift].iBrick] == pBrick )
            return iLift;
    }
    return -1;
}

int GetBrickIndex( float x )
{
    if( x < 0 ) x += TOWER_CIRCUMFERENCE; else if( x > TOWER_CIRCUMFERENCE ) x -= TOWER_CIRCUMFERENCE;
    return x / BRICK_SIZE;
//    return ((int)((x + TOWER_CIRCUMFERENCE)) % TOWER_CIRCUMFERENCE) / BRICK_SIZE;
}

int GetRingIndex( float y )
{
    return g_nTowerHeight - (int)( y / RING_HEIGHT );
}

TowerBrick* GetBrickAt( int iRing, int iBrick )
{
    static TowerBrick s_NullBrick;
    s_NullBrick = B_NULL;
    if( iRing < 0 || iRing >= g_nTowerHeight )
        return &s_NullBrick;
    else
        return &TheTower[iRing][iBrick];
}

TowerBrick* GetBrick( float x, float y )
{
    return GetBrickAt( GetRingIndex( y ), GetBrickIndex( x ) );
}

TowerBrick** GetBricks( float x1, float y1, float x2, float y2 )
{
    TowerBrick* s_pBricks[4];
    register int iBrick1 = GetBrickIndex( x1 );
    register int iRing1 = GetRingIndex( y1 );
    register int iBrick2 = GetBrickIndex( x2 );
    register int iRing2 = GetRingIndex( y2 );
    s_pBricks[0] = GetBrickAt( iRing1, iBrick1 );
    s_pBricks[1] = GetBrickAt( iRing1, iBrick2 );
    s_pBricks[2] = GetBrickAt( iRing2, iBrick1 );
    s_pBricks[3] = GetBrickAt( iRing2, iBrick2 );
    return s_pBricks;

}

BOOL IsSolid( TowerBrick b )
{
    if( b & B_LIFTSTALK ) return TRUE;

    b &= B_TYPE_MASK;
    return( b == B_PLAT || b == B_LIFT || b == B_MVLF || b == B_MVRT || b == B_SUPP || b == B_CRAK || b == B_SHOT );
}

BOOL IsAlwaysSolid( TowerBrick b )
{
    if( b & B_LIFTSTALK ) return TRUE;

    b &= B_TYPE_MASK;
    return( b == B_PLAT || b == B_LIFT || b == B_MVLF || b == B_MVRT || b == B_SUPP );
}

BOOL CanMove( PLAYER* pPlayer )
{
    return pPlayer->GameState == GAME_STATE_DEFAULT;
}

BOOL CanJump( PLAYER* pPlayer )
{
    //can only jump in the default game state
    if( pPlayer->GameState == GAME_STATE_DEFAULT )
    {
        //see if there's a roof overhead
        if( (IsSolid(*GetBrick( pPlayer->x-15, pPlayer->y+48 )) || IsSolid(*GetBrick( pPlayer->x+15, pPlayer->y+48 ))))
            return FALSE;
        else //can only jump when on the ground
            return pPlayer->bOnGround;
    }
    else
        return FALSE;
}

void DestroyBlock( TowerBrick* pBrick, float x, float y )
{
    int i = 8;
    float offx, offy;

    //remove block
    *pBrick = (*pBrick & ~B_TYPE_MASK) | B_NORM;

    //add some shrapnell
    while( i-- )
    {
        offx = (float)((rand() & 0xFF) - 128) / 128.0f;
        offy = (float)((rand() & 0xFF) - 128) / 128.0f;
        AddFragment( x + offx*(PLATFORM_WIDTH>>1), y + offy*(RING_HEIGHT>>1), TOWER_RADIUS+PLATFORM_WIDTH, 5, 7, offx*2.0f, 4.0f+(offy*4.0f), &kmshFragment );
    }

    //add a puff of smoke
    AddPuff( x, y, TOWER_RADIUS+PLATFORM_WIDTH, 16, 0xAF9469 );
}

void UpdateBullet( PLAYER* pPlayer )
{
    if( pPlayer->bullet_life )
    {
        TowerBrick* pBrick;

        //move bullet
        pPlayer->bullet_x += pPlayer->bullet_dx;
        pPlayer->bullet_y += 0.5f;
        if( pPlayer->bullet_x > TOWER_CIRCUMFERENCE ) pPlayer->bullet_x -= TOWER_CIRCUMFERENCE; else if( pPlayer->bullet_x < 0.0f ) pPlayer->bullet_x += TOWER_CIRCUMFERENCE;
        pPlayer->bullet_life--;

        //check bullet collision
        pBrick = GetBrick( pPlayer->bullet_x, pPlayer->bullet_y-BULLET_HIT_RADIUS+16 );
        if( (*pBrick & B_TYPE_MASK) == B_CRAK || (*pBrick & B_TYPE_MASK) == B_SHOT )
        {
            DestroyBlock( pBrick, (GetBrickIndex( pPlayer->bullet_x )*BRICK_SIZE) + (BRICK_SIZE>>1), GetTowerHeight() - (GetRingIndex( pPlayer->bullet_y-BULLET_HIT_RADIUS ) * RING_HEIGHT) - 16 );
            pPlayer->bullet_life = 0;
        }
        else
            if( IsSolid(*pBrick) )
            {
                //solid block - die
                pPlayer->bullet_life = 0;
            }
            else
            {
                //check other half
                pBrick = GetBrick( pPlayer->bullet_x, pPlayer->bullet_y+BULLET_HIT_RADIUS+16 );
                if( (*pBrick & B_TYPE_MASK) == B_CRAK || (*pBrick & B_TYPE_MASK) == B_SHOT  )
                {
                    DestroyBlock( pBrick, (GetBrickIndex( pPlayer->bullet_x )*BRICK_SIZE) + (BRICK_SIZE>>1), GetTowerHeight() - (GetRingIndex( pPlayer->bullet_y+BULLET_HIT_RADIUS ) * RING_HEIGHT) - 16 );
                    pPlayer->bullet_life = 0;
                } 
                else
                    if( IsSolid(*pBrick) )
                    {
                        //solid block - die
                        pPlayer->bullet_life = 0;
                    }
                    else
                    {
                        //check collide with monsters
                        struct MonsterData* pMonster = g_MonsterData;
                        int nCount = g_nMonsterCount;

                        while( nCount-- )
                        {
                            if( pMonster->state == MONSTER_STATE_ALIVE || pMonster->state == MONSTER_STATE_ATTACK || pMonster->state == MONSTER_STATE_STUN )
                            {
                                if( GetTowerDistanceX(pPlayer->bullet_x,pMonster->x)<16 && fabs((pMonster->y+16)-pPlayer->bullet_y)<32)
                                {
                                    switch( pMonster->type )
                                    {
                                        case MONSTER_TYPE_UPDOWN:
                                        case MONSTER_TYPE_LEFTRIGHT:
                                            break;

                                        case MONSTER_TYPE_ROVERBADASS:
                                            pMonster->state = MONSTER_STATE_STUN;
                                            pMonster->stateCount = MONSTER_STATE_STUN_TIME;
                                            break;

                                        case MONSTER_TYPE_ROVER:
                                        case MONSTER_TYPE_ROVERANGRY:
                                            pMonster->state = MONSTER_STATE_DIEJUMP;
                                            pMonster->stateCount = 0;
                                            pMonster->dy = 4.0f;
                                            pMonster->dx = 0.0f;
                                            break;
                                    }

                                    pPlayer->bullet_life = 0;
                                    break;
                                }
                            }

                            pMonster++;
                        }
                    }
            }
    }
}

BOOL UpdatePosition( PLAYER* pPlayer )
{
    //update angle
    if( pPlayer->desired_angle != pPlayer->angle )
    {
        register float delta = pPlayer->angle - pPlayer->desired_angle;
        if( fabs(delta) < PI/15.0f )
        {
            pPlayer->angle = pPlayer->desired_angle;
        }
        else
        {
            if( fabs(delta) >= PI )
                pPlayer->angle += (PI/15.0f) * sign(delta);
            else
                pPlayer->angle -= (PI/15.0f) * sign(delta);
        }
        if( pPlayer->angle > (PI*2.0f) ) pPlayer->angle -= (PI*2.0f); else if( pPlayer->angle < 0.0f ) pPlayer->angle += (PI*2.0f);
    }

    //apply drag and set the maximum altitude
    if( !pPlayer->bOnGround )
    {
        if( pPlayer->dx < -0.1f ) 
            pPlayer->dx += 0.025f; 
        else 
            if( pPlayer->dx > 0.1f ) 
                pPlayer->dx -= 0.025f; 
            else
                pPlayer->dx = 0.0f; 
    }

    //update player bullet
    UpdateBullet( pPlayer );

    //center on the nearest block, if needed
    if( pPlayer->bCentering ) CenterOnNearestBrick( pPlayer );

    //wrap player position
    if( pPlayer->x < 0 ) pPlayer->x += TOWER_CIRCUMFERENCE; else if( pPlayer->x > TOWER_CIRCUMFERENCE ) pPlayer->x -= TOWER_CIRCUMFERENCE;

    //update player
    switch( pPlayer->GameState )
    {
        case GAME_STATE_GETREADY:
            if( pPlayer->GameStateCount-- == 0 )
            {
                pPlayer->GameState = GAME_STATE_DEFAULT;
            }
            return FALSE;

        case GAME_STATE_ENTER:
            pPlayer->z = TOWER_RADIUS-16.0f+((PLATFORM_WIDTH) * ((float)pPlayer->GameStateCount/(float)GAME_STATE_ENTERLEAVE_TIME));
            if( pPlayer->GameStateCount-- == 0 )
            {
                pPlayer->GameState = GAME_STATE_ROTATE;
                pPlayer->GameStateCount = GAME_STATE_ROTATE_TIME;
            }
            if( CheckSpinnyThing( pPlayer ) )
            {
                CenterOnNearestBrick( pPlayer );
                pPlayer->GameState = GAME_STATE_FALL;
                pPlayer->iCurrentLift = -1;
                pPlayer->GameStateCount = GAME_STATE_FALL_TIME;
                pPlayer->dy = 6.0f;
                pPlayer->dx = 0.0f;
                return FALSE;
            }
            break;

        case GAME_STATE_ROTATE:
            pPlayer->z = (TOWER_RADIUS+64.0f)*fabs((float)(((GAME_STATE_ROTATE_TIME>>1)-pPlayer->GameStateCount) / (float)(GAME_STATE_ROTATE_TIME))); //fixme: checkme
            pPlayer->x += ((float)TOWER_CIRCUMFERENCE-(BRICK_SIZE>>1)) / ((float)GAME_STATE_ROTATE_TIME * 2.0f);
            pPlayer->angle += (PI) / ((float)GAME_STATE_ROTATE_TIME);
            pPlayer->desired_angle = pPlayer->angle;
            if( pPlayer->GameStateCount-- == 0 )
            {
                //snap the player directly onto the center of the brick, just in case
                pPlayer->x = (BRICK_SIZE * GetBrickIndex( pPlayer->x )) + (BRICK_SIZE>>1);
                pPlayer->GameState = GAME_STATE_LEAVE;
                pPlayer->GameStateCount = GAME_STATE_ENTERLEAVE_TIME;
            }
            break;

        case GAME_STATE_LEAVE:
            pPlayer->z = TOWER_RADIUS-16.0f+((PLATFORM_WIDTH) * (1.0f - ((float)pPlayer->GameStateCount/(float)GAME_STATE_ENTERLEAVE_TIME)));
            if( pPlayer->GameStateCount-- == 0 )
            {
                pPlayer->GameState = GAME_STATE_DEFAULT;
                pPlayer->desired_angle = pPlayer->fOldAngle;
                if( CheckSpinnyThing( pPlayer ) )
                {
                    CenterOnNearestBrick( pPlayer );
                    pPlayer->GameState = GAME_STATE_FALL;
                    pPlayer->iCurrentLift = -1;
                    pPlayer->GameStateCount = GAME_STATE_FALL_TIME;
                    pPlayer->dy = 6.0f;
                    pPlayer->dx = 0.0f;
                    return FALSE;
                }
            }
            break;

        case GAME_STATE_LIFT:
            if( pPlayer->iCurrentLift != -1 )
                pPlayer->y = g_LiftData[pPlayer->iCurrentLift].base_y + g_LiftData[pPlayer->iCurrentLift].y + g_LiftData[pPlayer->iCurrentLift].dy;
            pPlayer->dy = 0.0f;
            if( CheckMonsters( pPlayer ) )
            {
                CenterOnNearestBrick( pPlayer );
                pPlayer->GameState = GAME_STATE_FALL;
                g_LiftData[pPlayer->iCurrentLift].state = LIFT_STATE_UP;
                g_LiftData[pPlayer->iCurrentLift].nStateTime = LIFT_STATE_WAIT_TIME;
                g_LiftData[pPlayer->iCurrentLift].dy = 0.0f;
                pPlayer->iCurrentLift = -1;
                pPlayer->GameStateCount = GAME_STATE_FALL_TIME;
                pPlayer->dy = 6.0f;
                pPlayer->dx = 0.0f;
                return FALSE;
            }
            break;

        case GAME_STATE_FALL:
            pPlayer->z = (TOWER_RADIUS+PLATFORM_WIDTH)-16.0f+((1.0f - fabs(1.0f - (((float)pPlayer->GameStateCount/(float)GAME_STATE_FALL_TIME) * 2.0f)))*32.0f);
            //pPlayer->angle -= PI / 10.0f;
            //pPlayer->desired_angle = pPlayer->angle;
            if( pPlayer->dy > -4.0f ) pPlayer->dy -= 0.5f;
            pPlayer->y += pPlayer->dy;
            if( pPlayer->y < 0.0f )
            {
                pPlayer->GameState = GAME_STATE_DROWN;
                pPlayer->GameStateCount = 0;
                AddSplash( pPlayer->x, (TOWER_RADIUS+PLATFORM_WIDTH)-16.0f );
                return FALSE; 
            }
            else if( pPlayer->y > GetTowerHeight() )
            {
                pPlayer->dy = -pPlayer->dy; 
                pPlayer->y += pPlayer->dy; 
                pPlayer->y = GetTowerHeight(); 
            }
            if( pPlayer->GameStateCount-- == 0 )
            {
                //make sure we don't fall through a solid platform
                if( IsSolid(*GetBrick(pPlayer->x,pPlayer->y+16)) || IsSolid(*GetBrick(pPlayer->x,pPlayer->y+32)))
                    pPlayer->GameStateCount = 1;
                else
                {
                    pPlayer->GameState = GAME_STATE_DEFAULT;
                }
            }
            return FALSE;

        case GAME_STATE_EXIT:
            pPlayer->z = TOWER_RADIUS-16.0f+((PLATFORM_WIDTH) * ((float)pPlayer->GameStateCount/(float)GAME_STATE_ENTERLEAVE_TIME));
            if( pPlayer->GameStateCount-- == 0 )
            {
                int i;
                pPlayer->GameState = GAME_STATE_COLLAPSE;
                pPlayer->GameStateCount = GAME_STATE_COLLAPSE_RUMBLE_TIME;

                if( pPlayer->nSpinnyThingCount == 0 ) pPlayer->bSpinnyThingToggle = !pPlayer->bSpinnyThingToggle;

                //store current time if it's better than the current best time
                if( g_nCurrentTime < g_nBestTime ) g_nBestTime = g_nCurrentTime;

                for( i = 0; i < g_nLiftCount; i++ )
                    g_LiftData[i].state = LIFT_STATE_DISABLED;


            }
            if( CheckSpinnyThing( pPlayer ) )
            {
                CenterOnNearestBrick( pPlayer );
                pPlayer->GameState = GAME_STATE_FALL;
                pPlayer->iCurrentLift = -1;
                pPlayer->GameStateCount = GAME_STATE_FALL_TIME;
                pPlayer->dy = 6.0f;
                pPlayer->dx = 0.0f;
                return FALSE;
            }
            break;

        case GAME_STATE_DONEEXIT:
            pPlayer->z = TOWER_RADIUS-16.0f+((PLATFORM_WIDTH) * (1.0f - ((float)pPlayer->GameStateCount/(float)GAME_STATE_ENTERLEAVE_TIME)));
            if( pPlayer->GameStateCount-- == 0 )
            {
                pPlayer->GameState = GAME_STATE_RESCUE;
                pPlayer->GameStateCount = 8;
            }
            //blow up the spinny thing
            if( pPlayer->nSpinnyThingCount == 0 ) { pPlayer->fSpinnyThingY += 8.0f; }
            break;

        case GAME_STATE_COLLAPSE:
        {
            int i;
            //blow up the spinny thing
            if( pPlayer->nSpinnyThingCount == 0 ) { pPlayer->fSpinnyThingY += 8.0f; }
            if( pPlayer->GameStateCount == 0 )
            {
                if( TheTower[2][0] != B_NULL )
                {
                    //remove the bottom ring of the tower
                    TowerRing* pTower = TheTower;
                    while( *pTower[0] ) pTower++;
                    pTower[-1][0] = B_NULL;

                    //shrink the current tower height
                    g_nCurrentTowerHeight -= 1;
                    g_fTextureHeight = ((g_nCurrentTowerHeight * RING_HEIGHT) / 256.0f) * g_fTextureWidth;
                    pPlayer->y -= RING_HEIGHT;

                    if( g_nCurrentTowerHeight > 8 )
                    {
                        if( (g_nCurrentTowerHeight&3) == 0 ) AddPuff( rand() % TOWER_CIRCUMFERENCE, pPlayer->y - (rand() % 80) , (TOWER_RADIUS+PLATFORM_WIDTH)+32.0f, 64.0f, 0x89724F );
                    }
                    else
                    {
                        AddSplash( rand() % TOWER_CIRCUMFERENCE, (TOWER_RADIUS+PLATFORM_WIDTH)+256.0f );
                    }

                    //move all lifts down
                    for( i = 0; i < g_nLiftCount; i++ )
                    {
                        g_LiftData[i].y -= RING_HEIGHT;
                        if( g_LiftData[i].base_y + g_LiftData[i].y < 0.0f ) g_LiftData[i].state = LIFT_STATE_INVISIBLE;
                    }
                }
                else
                {
                    pPlayer->GameState = GAME_STATE_DONEEXIT;
                    pPlayer->GameStateCount = GAME_STATE_ENTERLEAVE_TIME;
                }
            }
            else
            {
                if( (pPlayer->GameStateCount&3) == 0 ) AddPuff( rand() % TOWER_CIRCUMFERENCE, pPlayer->y - (rand() % 80) , (TOWER_RADIUS+PLATFORM_WIDTH)+32.0f, 64.0f, 0x89724F );
                pPlayer->GameStateCount--;
                if( pPlayer->GameStateCount == (GAME_STATE_COLLAPSE_RUMBLE_TIME>>1) )
                {
                    int i;
                    for( i = 0; i < g_nMonsterCount; i++ )
                    {
                        g_MonsterData[i].state = MONSTER_STATE_DIEJUMP;
                        g_MonsterData[i].stateCount = 0;
                        g_MonsterData[i].dy = 4.0f;
                        g_MonsterData[i].dx = 0.0f;
                    }
                }
            }
            break;
        }

        case GAME_STATE_RESCUE:
            pPlayer->desired_angle = pPlayer->fOldAngle;
            if( pPlayer->GameStateCount )
            {
                pPlayer->GameStateCount--;
                AddSplash( rand() % TOWER_CIRCUMFERENCE, (TOWER_RADIUS+PLATFORM_WIDTH)+256.0f );
            }
            //else
            //{
            //    ResetTower();
            //    ResetPlayer( pPlayer );
            //}
            break;

        case GAME_STATE_DROWN:
            pPlayer->y -= 2.0f; 
            pPlayer->x += pPlayer->dx; 
            pPlayer->dy = 0.0f; 
            AddBubble( pPlayer->x, pPlayer->y, (TOWER_RADIUS+PLATFORM_WIDTH)-16.0f );
            pPlayer->GameStateCount++;
            return FALSE;

        case GAME_STATE_DEFAULT:
        {
            float newx = pPlayer->x, newy = pPlayer->y + pPlayer->dy;
            int iNextBrick, iPrevBrick, iRing;
            TowerBrick nextbrick, prevbrick;
            int iLift;
            int directionWidth;

            //set current z position
            pPlayer->z = (TOWER_RADIUS+PLATFORM_WIDTH)-16.0f;

            //set the direction based on where they want to move
            if( pPlayer->dx < 0.0f )
                pPlayer->desired_angle = DegreesToRadians(270.0f);
            else
            if( pPlayer->dx > 0.0f )
                pPlayer->desired_angle = DegreesToRadians(90.0f);

            //can only move if we're pointing in the right direction
            if( pPlayer->desired_angle == pPlayer->angle )
                newx += pPlayer->dx;

            //calculate the direction sign
            if( pPlayer->angle <= PI ) directionWidth = 10; else directionWidth = -10;

            //wrap the current position
            if( newx < 0.0f ) newx += TOWER_CIRCUMFERENCE; else if( newx > TOWER_CIRCUMFERENCE ) newx -= TOWER_CIRCUMFERENCE;

            //check collide with monsters
            if( CheckMonsters( pPlayer ) )
            {
                CenterOnNearestBrick( pPlayer );
                pPlayer->GameState = GAME_STATE_FALL;
                pPlayer->iCurrentLift = -1;
                pPlayer->GameStateCount = GAME_STATE_FALL_TIME;
                pPlayer->dy = 6.0f;
                pPlayer->dx = 0.0f;
                return FALSE;
            }

            //test for drop off bottom
            if( newy < 0.0f  )
            {
                pPlayer->GameState = GAME_STATE_DROWN;
                pPlayer->GameStateCount = 0;
                AddSplash( pPlayer->x, (TOWER_RADIUS+PLATFORM_WIDTH)-16.0f );
                return FALSE; 
            }

            //check firing
            if( pPlayer->bOnGround && pPlayer->bFirePress && pPlayer->bullet_life == 0 )
            {
                pPlayer->bullet_life = BULLET_LIFE_TIME;
                pPlayer->bullet_y = pPlayer->y + BULLET_HIT_RADIUS;
                pPlayer->bullet_x = pPlayer->x + directionWidth;
                pPlayer->bullet_dx = sign(directionWidth) * BULLET_SPEED;
            }

            //get indices
            iRing = GetRingIndex( newy+15);
            iNextBrick = GetBrickIndex(newx+directionWidth);
            iPrevBrick = GetBrickIndex(newx-directionWidth);
            if( iRing < 0 )
            {
                pPlayer->dy = -pPlayer->dy; 
                pPlayer->y += pPlayer->dy; 
                if( pPlayer->y > GetTowerHeight() ) pPlayer->y = GetTowerHeight(); 
                return FALSE; 
            }

            //check for breakaways
            if( (*GetBrick( newx+directionWidth, newy+1 ) & B_TYPE_MASK) == B_CRAK )
            {
                if( IsAlwaysSolid( *GetBrick( newx-directionWidth, newy+1 ) ) )
                {
                    //it's breakable - start it cracking
                    *GetBrick( newx+directionWidth, newy+1 ) |= B_CRACKING;
                }
                else
                {
                    DestroyBlock( GetBrick( newx+directionWidth, newy+1 ), (GetBrickIndex( newx-directionWidth )*BRICK_SIZE) + (BRICK_SIZE>>1), GetTowerHeight() - (GetRingIndex( newy+1 ) * RING_HEIGHT) - 16 );
                }
            }
            if( (*GetBrick( newx-directionWidth, newy+1 ) & B_TYPE_MASK) == B_CRAK )
            {
                DestroyBlock( GetBrick( newx-directionWidth, newy+1 ), (GetBrickIndex( newx-directionWidth )*BRICK_SIZE) + (BRICK_SIZE>>1), GetTowerHeight() - (GetRingIndex( newy+1 ) * RING_HEIGHT) - 16 );
            }


            //test the row we're in
            if( iRing > 0 )
            {
                //make sure we're not going up against a solid wall
                if( iRing > 1 )
                {
                    nextbrick = TheTower[iRing-2][iNextBrick];
                    prevbrick = TheTower[iRing-2][iPrevBrick];
                    if( IsSolid(nextbrick) || IsSolid(prevbrick) )
                    {
                        float fPlatY = ((g_nTowerHeight-(iRing))*RING_HEIGHT);

                        nextbrick = TheTower[iRing-1][iNextBrick];
                        prevbrick = TheTower[iRing-1][iPrevBrick];
                        if( IsSolid(nextbrick) || IsSolid(prevbrick) )
                        {
                            if( pPlayer->y != fPlatY )
                            {
                                pPlayer->x -= sign(directionWidth);
                                pPlayer->dx = 0.0f; //pPlayer->dx = -pPlayer->dx; 
                            }
                        }

                        if( pPlayer->dy > 0.0f && pPlayer->y != fPlatY-16 ) pPlayer->dy = -pPlayer->dy;
                        pPlayer->y += pPlayer->dy;
                        return( pPlayer->y == fPlatY );
                    }
                }

                nextbrick = TheTower[iRing-1][iNextBrick];
                prevbrick = TheTower[iRing-1][iPrevBrick];

                if( IsSolid(nextbrick) /*|| IsSolid(prevbrick)*/ )
                {
                    if( pPlayer->bOnGround )
                    {
                        //nope, move up a bit towards the new platform
                        if( IsSolid(*GetBrick( newx, newy+1 )) )
                        {
                            pPlayer->dy = 1.0f;
                            pPlayer->y += 8.0f;
                            pPlayer->dx = sign(directionWidth) * 2.0f;
                            pPlayer->x += pPlayer->dx;
                            return TRUE;
                        }
                        else
                        {
                            float fPlatY = ((g_nTowerHeight-(iRing))*RING_HEIGHT);
                            pPlayer->x -= sign(directionWidth);
                            pPlayer->dx = 0.0f;
                            pPlayer->y = fPlatY;
                            pPlayer->dy = 0.0f;
                            return TRUE;
                        }

                        return FALSE;
                    }
                    else
                    {
                        float fPlatY = ((g_nTowerHeight-(iRing))*RING_HEIGHT);
                        pPlayer->x -= sign(directionWidth);
                        pPlayer->dx = 0.0f;// pPlayer->dx = -pPlayer->dx; 
                        if( pPlayer->y > fPlatY ) pPlayer->y = fPlatY; else pPlayer->y = newy;

                        return( pPlayer->y == fPlatY );
                    }
                }
            }

            //test underfoot
            nextbrick = TheTower[iRing][iNextBrick];
            prevbrick = TheTower[iRing][iPrevBrick];
            if( IsSolid(nextbrick) || IsSolid(prevbrick) )
            {
                float fPlatY = ((g_nTowerHeight-(iRing))*RING_HEIGHT);

                //see if we're stuck in a block
                if( IsSolid(*GetBrick( newx, newy+16 ))||(!pPlayer->bOnGround&&(IsSolid(*GetBrick( newx-directionWidth, newy+16 ))||IsSolid(*GetBrick( newx+directionWidth, newy+16 )))))
                {
                    if( !IsSolid(*GetBrick( pPlayer->x, pPlayer->y+16 )))
                    {
                        newy = pPlayer->y = fPlatY; 
                        pPlayer->dy = 0.0f;                        
                    }
                }

                if( !IsSolid(*GetBrick( newx, newy )) && !IsSolid(*GetBrick( newx-directionWidth, newy )) )
                {
                    if( newy < fPlatY )
                    {
                        pPlayer->x -= sign(directionWidth);
                        pPlayer->dx = 0.0f; //pPlayer->dx = -pPlayer->dx; 
                        pPlayer->dy = 0.0f;
                        //pPlayer->x += pPlayer->dx;
                        pPlayer->y = fPlatY;
                        return TRUE;
                    }
                }
        
                //check for moving floors
                if( pPlayer->dx == 0.0f )
                {
                    if( (nextbrick & B_TYPE_MASK) == B_MVLF || (prevbrick & B_TYPE_MASK) == B_MVLF ) newx -= 1.5f;
                    if( (nextbrick & B_TYPE_MASK) == B_MVRT || (prevbrick & B_TYPE_MASK) == B_MVRT ) newx += 1.5f;
                }

                pPlayer->x = newx;
                pPlayer->y = newy;
                return( newy == fPlatY );
            }
            else
            {
                //fall
                if( pPlayer->dy > -4.0f ) pPlayer->dy -= 0.5f;
                pPlayer->x = newx;
                pPlayer->y = newy;
                return FALSE;
            }

            break;
        }
    }

    return TRUE;
}


void UpdateTower( PLAYER* pPlayer )
{
    //update lifts
    int i;
    for( i = 0; i < g_nLiftCount; i++ )
    {
        switch( g_LiftData[i].state )
        {
            //case LIFT_STATE_INVISIBLE:
            //    break;
            //
            //case LIFT_STATE_DISABLED:
            //    break;
            //
            //case LIFT_STATE_DOWN:
            //    break;
            //
            case LIFT_STATE_MOVEUP:
                if( g_LiftData[i].y < g_LiftData[i].height )
                    g_LiftData[i].y += LIFT_STATE_MOVE_SPEED;

                if( g_LiftData[i].y >= g_LiftData[i].height )
                {
                    int iRing;
                    g_LiftData[i].y = g_LiftData[i].height;
                    g_LiftData[i].state = LIFT_STATE_UP;
                    g_LiftData[i].nStateTime = LIFT_STATE_WAIT_TIME;
                    g_LiftData[i].dy = 0.0f;
                    if( i == pPlayer->iCurrentLift )
                    {
                        pPlayer->GameState = GAME_STATE_DEFAULT;
                        pPlayer->iCurrentLift = -1;
                    }
                    iRing = GetRingIndex( g_LiftData[i].base_y + g_LiftData[i].y );
                    if( iRing >= 0 ) TheTower[iRing][g_LiftData[i].iBrick] |= B_LIFTSTALK;
                }
                else
                {
                    int iRing = GetRingIndex( g_LiftData[i].base_y + g_LiftData[i].y );
                    if( iRing >= 0 ) TheTower[iRing][g_LiftData[i].iBrick] |= B_LIFTSTALK;
                }
                break;

            case LIFT_STATE_UP:
                if( --g_LiftData[i].nStateTime == 0 )
                {
                    g_LiftData[i].state = LIFT_STATE_MOVEDOWN;
                    g_LiftData[i].dy = -LIFT_STATE_DROP_SPEED;
                }
                break;

            case LIFT_STATE_MOVEDOWN:
                if( g_LiftData[i].y > 0.0f )
                    g_LiftData[i].y += g_LiftData[i].dy;
                if( g_LiftData[i].y <= 0.0f )
                {
                    g_LiftData[i].y = 0.0f;
                    g_LiftData[i].state = LIFT_STATE_DOWN;
                    g_LiftData[i].nStateTime = LIFT_STATE_WAIT_TIME;
                    g_LiftData[i].dy = 0.0f;
                    if( i == pPlayer->iCurrentLift )
                    {
                        pPlayer->GameState = GAME_STATE_DEFAULT;
                        pPlayer->iCurrentLift = -1;
                    }
                }
                else
                {
                    int iRing = GetRingIndex( g_LiftData[i].base_y + g_LiftData[i].y );
                    if( iRing > 0 ) TheTower[iRing-1][g_LiftData[i].iBrick] &= ~B_LIFTSTALK;
                }
                break;
        }
    }
    
    //update monsters
    for( i = 0; i < g_nMonsterCount; i++ )
    {
        switch( g_MonsterData[i].state )
        {
            case MONSTER_STATE_ALIVE:
                switch( g_MonsterData[i].type )
                {
                    case MONSTER_TYPE_LEFTRIGHT:
                    {
                        if( !CheckCollideOtherMonsters(i) )
                        {
                            TowerBrick** pBricks = GetBricks( g_MonsterData[i].x-16+g_MonsterData[i].dx, g_MonsterData[i].y+31, g_MonsterData[i].x+16+g_MonsterData[i].dx, g_MonsterData[i].y+15 );
                            if( IsSolid(*pBricks[0]) || IsSolid(*pBricks[1]) || IsSolid(*pBricks[2]) || IsSolid(*pBricks[3]) )
                            {
                                g_MonsterData[i].dx = -g_MonsterData[i].dx;
                                g_MonsterData[i].x += g_MonsterData[i].dx;
                            }
                        }

                        g_MonsterData[i].x += g_MonsterData[i].dx;
                        if( g_MonsterData[i].x > TOWER_CIRCUMFERENCE ) g_MonsterData[i].x -= TOWER_CIRCUMFERENCE; else if( g_MonsterData[i].x < 0 ) g_MonsterData[i].x += TOWER_CIRCUMFERENCE;
                        break;
                    }

                    case MONSTER_TYPE_UPDOWN:
                    {
                        if( !CheckCollideOtherMonsters(i) )
                        {
                            TowerBrick** pBricks = GetBricks( g_MonsterData[i].x-15, g_MonsterData[i].y+48+g_MonsterData[i].dy, g_MonsterData[i].x+15, g_MonsterData[i].y+g_MonsterData[i].dy+16 );
                            if( IsSolid(*pBricks[0]) || IsSolid(*pBricks[1]) || IsSolid(*pBricks[2]) || IsSolid(*pBricks[3]) )
                            {
                                g_MonsterData[i].dy = -g_MonsterData[i].dy;
                                g_MonsterData[i].y += g_MonsterData[i].dy;
                            }
                        }

                        g_MonsterData[i].y += g_MonsterData[i].dy;
                        break;
                    }

                    case MONSTER_TYPE_ROVER:
                    {
                        float newy = g_MonsterData[i].y + g_MonsterData[i].dy;

                        //bounce off the platform underneath
                        if( IsSolid(*GetBrick(g_MonsterData[i].x-15,newy+15)) || IsSolid(*GetBrick(g_MonsterData[i].x+15,newy+15)) )
                        {
                            newy = ((g_nTowerHeight-(GetRingIndex(newy+15)))*RING_HEIGHT);
                            g_MonsterData[i].dy = 3.0f;

                            //see if the player is near - go into 'attack' mode
                            if( pPlayer->GameState == GAME_STATE_DEFAULT && pPlayer->y >= newy-16 && pPlayer->y <= newy + 16 && GetTowerDistanceX(pPlayer->x,g_MonsterData[i].x) < (BRICK_SIZE*4)  )
                            {
                                g_MonsterData[i].dx = GetTowerDirectionX(g_MonsterData[i].x,pPlayer->x)*1.5f;
                                g_MonsterData[i].state = MONSTER_STATE_ATTACK;
                           }
                        }
                        else
                        {
                            if( g_MonsterData[i].dy > -3.0f ) g_MonsterData[i].dy -= 0.25f;
                        }


                        //move
                        g_MonsterData[i].y = newy;
                        if( g_MonsterData[i].y < 0.0f )
                        {
                            AddSplash( g_MonsterData[i].x, g_MonsterData[i].z );
                            g_MonsterData[i].state = MONSTER_STATE_DROWN;
                        }
                        break;
                    }

                    case MONSTER_TYPE_ROVERBADASS:
                    {
                        float newx = g_MonsterData[i].x + g_MonsterData[i].dx, newy = g_MonsterData[i].y + g_MonsterData[i].dy;
                        int directionWidth = g_MonsterData[i].dx < 0 ? -15 : 15;

                        //bounce off walls and other monsters
                        if( !CheckCollideOtherMonsters(i) )   
                        {
                            if( IsSolid(*GetBrick(newx+directionWidth,newy+16)) ||
                                IsSolid(*GetBrick(newx+directionWidth,newy+32)) ||
                                !( ( *GetBrick(newx,newy+15) & B_TYPE_MASK ) == B_PLAT )  )
                            {
                                g_MonsterData[i].dx = -g_MonsterData[i].dx;
                                newx = g_MonsterData[i].x + (g_MonsterData[i].dx*2.0f);
                            }
                        }
                        else
                        {
                            newx = g_MonsterData[i].x + g_MonsterData[i].dx;
                            g_MonsterData[i].dy = -g_MonsterData[i].dy;
                            //newy = g_MonsterData[i].y + g_MonsterData[i].dy;
                        }

                        //see if we're stuck in a platform
                        if( IsSolid(*GetBrick(newx,newy+15)) )
                        {
                            newy = ((g_nTowerHeight-(GetRingIndex(newy)))*RING_HEIGHT);
                            g_MonsterData[i].dy = 0.0f;
                        }
                        else
                        {
                            if( g_MonsterData[i].dy > -3.0f ) g_MonsterData[i].dy -= 0.25f;
                        }

                        //move
                        if( newx > TOWER_CIRCUMFERENCE ) newx -= TOWER_CIRCUMFERENCE; else if( newx < 0 ) newx += TOWER_CIRCUMFERENCE;
                        g_MonsterData[i].x = newx;
                        g_MonsterData[i].y = newy;
                        if( g_MonsterData[i].y < 0.0f )
                        {
                            AddSplash( g_MonsterData[i].x, g_MonsterData[i].z );
                            g_MonsterData[i].state = MONSTER_STATE_DROWN;
                        }
                        break;
                    }

                    case MONSTER_TYPE_ROVERANGRY:
                    {
                        float newx = g_MonsterData[i].x + g_MonsterData[i].dx, newy = g_MonsterData[i].y + g_MonsterData[i].dy;
                        int directionWidth = g_MonsterData[i].dx < 0 ? -15 : 15;

                        //see if we're stuck in a platform
                        if( IsSolid(*GetBrick(g_MonsterData[i].x-15,newy+15)) || IsSolid(*GetBrick(g_MonsterData[i].x+15,newy+15)) )
                        {
                            newy = ((g_nTowerHeight-(GetRingIndex(newy+15)))*RING_HEIGHT);
                            g_MonsterData[i].dy = 3.0f;
                        }
                        else
                        {
                            if( g_MonsterData[i].dy > -3.0f ) g_MonsterData[i].dy -= 0.25f;
                        }

                        //bounce off walls and other monsters
                        if( !CheckCollideOtherMonsters(i) )
                        {
                            if( IsSolid(*GetBrick(newx+directionWidth,newy+16)) ||
                                IsSolid(*GetBrick(newx+directionWidth,newy+32)) )
                            {
                                g_MonsterData[i].dx = -g_MonsterData[i].dx;
                                newx = g_MonsterData[i].x + (g_MonsterData[i].dx*2.0f);
                            }
                        }
                        else
                        {
                            newx = g_MonsterData[i].x + g_MonsterData[i].dx;
                            g_MonsterData[i].dy = -g_MonsterData[i].dy;
                            //newy = g_MonsterData[i].y + g_MonsterData[i].dy;
                        }

                        //move
                        if( newx > TOWER_CIRCUMFERENCE ) newx -= TOWER_CIRCUMFERENCE; else if( newx < 0 ) newx += TOWER_CIRCUMFERENCE;
                        g_MonsterData[i].x = newx;
                        g_MonsterData[i].y = newy;
                        if( g_MonsterData[i].y < 0.0f )
                        {
                            AddSplash( g_MonsterData[i].x, g_MonsterData[i].z );
                            g_MonsterData[i].state = MONSTER_STATE_DROWN;
                        }
                        break;
                    }
                }
                break;

            case MONSTER_STATE_ATTACK:
                switch( g_MonsterData[i].type )
                {
                    case MONSTER_TYPE_ROVER:
                    {
                        float newx = g_MonsterData[i].x + g_MonsterData[i].dx, newy = g_MonsterData[i].y + g_MonsterData[i].dy;

                        //bounce off walls and other monsters
                        if( !CheckCollideOtherMonsters(i) )
                        {
                            if( IsSolid(*GetBrick(newx-15,newy+16)) || IsSolid(*GetBrick(newx+15,newy+16)) ||
                                IsSolid(*GetBrick(newx-15,newy+32)) || IsSolid(*GetBrick(newx+15,newy+32)) )
                            {
                                g_MonsterData[i].dx = -g_MonsterData[i].dx;
                                newx = g_MonsterData[i].x + (g_MonsterData[i].dx*2.0f);
                            }
                        }
                        else
                        {
                            newx = g_MonsterData[i].x + g_MonsterData[i].dx;
                            g_MonsterData[i].dy = -g_MonsterData[i].dy;
                            //newy = g_MonsterData[i].y + g_MonsterData[i].dy;
                        }

                        //see if we're stuck in a platform
                        if( IsSolid(*GetBrick(g_MonsterData[i].x-15,newy+15)) || IsSolid(*GetBrick(g_MonsterData[i].x+15,newy+15)) )
                        {
                            newy = ((g_nTowerHeight-(GetRingIndex(newy+15)))*RING_HEIGHT);
                            g_MonsterData[i].dy = 3.0f;

                            /* note: normal mode turned off because the monsters might stop outside a door when the player goes through
                            //see if the player is too far away - go into 'normal' mode
                            //pPlayer->y >= newy-16 && pPlayer->y <= newy + 16 &&
                            if( !(GetTowerDistanceX(pPlayer->x,g_MonsterData[i].x) < (BRICK_SIZE*5)  ))
                            {
                                g_MonsterData[i].dx = 0.0f;
                                g_MonsterData[i].state = MONSTER_STATE_ALIVE;
                            }
                            else
                            {
                            */
                                //adjust direction
                                g_MonsterData[i].dx = GetTowerDirectionX(g_MonsterData[i].x,pPlayer->x)*1.5f;
                            /*}*/
                        }
                        else
                        {
                            if( g_MonsterData[i].dy > -3.0f ) g_MonsterData[i].dy -= 0.25f;
                        }

                        //move
                        if( newx > TOWER_CIRCUMFERENCE ) newx -= TOWER_CIRCUMFERENCE; else if( newx < 0 ) newx += TOWER_CIRCUMFERENCE;
                        g_MonsterData[i].x = newx;
                        g_MonsterData[i].y = newy;
                        if( g_MonsterData[i].y < 0.0f )
                        {
                            AddSplash( g_MonsterData[i].x, g_MonsterData[i].z );
                            g_MonsterData[i].state = MONSTER_STATE_DROWN;
                        }
                        break;
                    }
                }
                break;

            case MONSTER_STATE_STUN:
                if( --g_MonsterData[i].stateCount == 0 )
                    g_MonsterData[i].state = MONSTER_STATE_ALIVE;
                break;

            case MONSTER_STATE_DIEJUMP:
                if( g_MonsterData[i].y < 0.0f )
                {
                    AddSplash( g_MonsterData[i].x, g_MonsterData[i].z );
                    g_MonsterData[i].state = MONSTER_STATE_DROWN;
                }
                else
                {
                    if( g_MonsterData[i].dy > -8.0f )
                    {
                        g_MonsterData[i].dy -= 0.5f;
                    }
                    g_MonsterData[i].z += 1.5f;
                    g_MonsterData[i].y += g_MonsterData[i].dy;
                }
                g_MonsterData[i].stateCount++;
                break;

            case MONSTER_STATE_DROWN:
                if( g_MonsterData[i].y < -100.0f )
                    g_MonsterData[i].state = MONSTER_STATE_INVISIBLE;
                else
                {
                    AddBubble( g_MonsterData[i].x, g_MonsterData[i].y, g_MonsterData[i].z );
                    g_MonsterData[i].y -= 2.0f;
                }
                break;

            case MONSTER_STATE_INVISIBLE:
                break;
        }
    }

    //update spinny thing
    if( pPlayer->GameState != GAME_STATE_GETREADY )
    {
        if( pPlayer->nSpinnyThingCount == 0 )
        {
            if( pPlayer->bSpinnyThingToggle )
            {
                if( pPlayer->fSpinnyThingX > 390.0f )
                    pPlayer->fSpinnyThingX += 6.0f;
                else
                    pPlayer->fSpinnyThingX += ((pPlayer->dx>0.0f)?1.0f:2.0f); //go slower if they're walking towards us
                if( pPlayer->fSpinnyThingX > 656.0f )
                {
                    pPlayer->nSpinnyThingCount = SPINNY_THING_TIME;
                }
            }
            else
            {
                if( pPlayer->fSpinnyThingX < 250.0f )
                    pPlayer->fSpinnyThingX -= 6.0f;
                else
                    pPlayer->fSpinnyThingX -= ((pPlayer->dx<0.0f)?1.0f:2.0f); //go slower if they're walking towards us
                if( pPlayer->fSpinnyThingX < -16.0f )
                {
                    pPlayer->nSpinnyThingCount = SPINNY_THING_TIME;
                }
            }
        }
        else
        {
            TowerBrick* b1 = GetBrick(pPlayer->x-15,pPlayer->y+16);
            TowerBrick* b2 = GetBrick(pPlayer->x+15,pPlayer->y+16);

            //see if the spinny thing should arrive yet
            if( pPlayer->bSpinnyThingEnabled )
            {           
                if( *b1 & B_DISABLESPINNY )
                {
                    *b1 &= ~B_DISABLESPINNY;
                    pPlayer->bSpinnyThingEnabled = FALSE;
                }
                if( *b2 & B_DISABLESPINNY )
                {
                    *b2 &= ~B_DISABLESPINNY;
                    pPlayer->bSpinnyThingEnabled = FALSE;
                }

                if( ( !((*b1 & B_NOSPINNY) || (*b2 & B_NOSPINNY )) ) )
                {
                    if( --pPlayer->nSpinnyThingCount == 0 )
                    {
                        switch( pPlayer->GameState )
                        {
                            case GAME_STATE_LIFT:
                            case GAME_STATE_ROTATE:
                            case GAME_STATE_DEFAULT:
                            case GAME_STATE_ENTER:
                            case GAME_STATE_LEAVE:
                            case GAME_STATE_FALL:
                                pPlayer->bSpinnyThingToggle = !pPlayer->bSpinnyThingToggle;
                                pPlayer->fSpinnyThingY = ((g_nTowerHeight-(GetRingIndex(pPlayer->y)))*RING_HEIGHT); 
                                break;

                            case GAME_STATE_DROWN:
                            case GAME_STATE_EXIT:
                            case GAME_STATE_DONEEXIT:
                            case GAME_STATE_COLLAPSE:
                            case GAME_STATE_RESCUE:
                                pPlayer->nSpinnyThingCount = SPINNY_THING_TIME;
                                break;
                        }
                    }
                }
            }
            else
            {
                if( *b1 & B_ENABLESPINNY )
                {
                    *b1 &= ~B_ENABLESPINNY;
                    pPlayer->nSpinnyThingCount = SPINNY_THING_DEFAULT_TIME;
                    pPlayer->bSpinnyThingEnabled = TRUE;
                }
                if( *b2 & B_ENABLESPINNY )
                {
                    *b2 &= ~B_ENABLESPINNY;
                    pPlayer->nSpinnyThingCount = SPINNY_THING_DEFAULT_TIME;
                    pPlayer->bSpinnyThingEnabled = TRUE;
                }
            }
        }
    }
    
    //update bubbles etc.
    UpdateBubbles();
    UpdateSplashes();
    UpdatePuffs();
    UpdateFragments();

    //update timer if the game hasn't finished
    if( pPlayer->GameState != GAME_STATE_GETREADY && pPlayer->GameState != GAME_STATE_DONEEXIT && pPlayer->GameState != GAME_STATE_COLLAPSE && pPlayer->GameState != GAME_STATE_RESCUE )
        g_nCurrentTime++;
}

void CheckLift( PLAYER* pPlayer, BOOL bDown )
{
    TowerBrick* pBrick = GetBrick( pPlayer->x, pPlayer->y );
    if( bDown )
    {
        int iLift = FindLiftTop( pBrick );
        if( iLift >= 0 )
        {
            if( g_LiftData[iLift].state == LIFT_STATE_UP )
            {
                g_LiftData[iLift].state = LIFT_STATE_MOVEDOWN;
                g_LiftData[iLift].dy = -LIFT_STATE_MOVE_SPEED;
                pPlayer->iCurrentLift = iLift;
                pPlayer->dx = 0.0f;
                CenterOnNearestBrick( pPlayer );
                pPlayer->GameState = GAME_STATE_LIFT;
            }
        }
    }
    else
    {
        int iLift = FindLift( pBrick );
        if( iLift >= 0 )
        {
            if( g_LiftData[iLift].state == LIFT_STATE_DOWN )
            {
                g_LiftData[iLift].state = LIFT_STATE_MOVEUP;
                g_LiftData[iLift].dy = LIFT_STATE_MOVE_SPEED;
                pPlayer->iCurrentLift = iLift;
                pPlayer->dx = 0.0f;
                CenterOnNearestBrick( pPlayer );
                pPlayer->GameState = GAME_STATE_LIFT;
            }
        }
    }
}

void CheckDoor( PLAYER* pPlayer )
{
    TowerBrick* pBrick = GetBrick( pPlayer->x, pPlayer->y+16 );
    if( (*pBrick & B_TYPE_MASK ) == B_DOOR )
    {
        pPlayer->fOldAngle = pPlayer->desired_angle;
        pPlayer->desired_angle = 0.0f;
        pPlayer->dx = 0.0f;
        pPlayer->dy = 0.0f;
        CenterOnNearestBrick( pPlayer );
        pPlayer->GameState = GAME_STATE_ENTER;
        pPlayer->GameStateCount = GAME_STATE_ENTERLEAVE_TIME;
    }
    else
    if( (*pBrick & B_TYPE_MASK ) == B_EXIT )
    {
        pPlayer->fOldAngle = pPlayer->desired_angle;
        pPlayer->desired_angle = 0.0f;
        pPlayer->dx = 0.0f;
        pPlayer->dy = 0.0f;
        CenterOnNearestBrick( pPlayer );
        pPlayer->GameState = GAME_STATE_EXIT;
        pPlayer->GameStateCount = GAME_STATE_ENTERLEAVE_TIME;
    }
}

BOOL CheckCollideOtherMonsters( int i )
{
    int iMonster;

    float x = g_MonsterData[i].x;
    float y = g_MonsterData[i].y;

    for( iMonster = 0; iMonster < g_nMonsterCount; iMonster++ )
    {
        if( iMonster != i && (g_MonsterData[iMonster].state == MONSTER_STATE_ALIVE || g_MonsterData[iMonster].state == MONSTER_STATE_ATTACK || g_MonsterData[iMonster].state == MONSTER_STATE_STUN ) )
        {
            register float dx, dy;
            dx = GetTowerDistanceX(g_MonsterData[iMonster].x, x);
            dy = g_MonsterData[iMonster].y - y;

            if( (dx*dx + dy*dy) < 512.0f )
            {
                g_MonsterData[i].dx = -g_MonsterData[i].dx;
                g_MonsterData[i].dy = -g_MonsterData[i].dy;
                g_MonsterData[i].x += g_MonsterData[i].dx;
                g_MonsterData[i].y += g_MonsterData[i].dy;

                g_MonsterData[iMonster].dx = -g_MonsterData[iMonster].dx;
                g_MonsterData[iMonster].dy = -g_MonsterData[iMonster].dy;
                g_MonsterData[iMonster].x += g_MonsterData[iMonster].dx;
                g_MonsterData[iMonster].y += g_MonsterData[iMonster].dy;

                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL CheckSpinnyThing( PLAYER* pPlayer )
{
    //check spinny thing [fuck me this is a nasty if statement...]
#ifdef DANGEROUS_MONSTERS
    return( pPlayer->nSpinnyThingCount == 0 && pPlayer->fSpinnyThingX > 320-16 && pPlayer->fSpinnyThingX < 320+16 && fabs(pPlayer->fSpinnyThingY-pPlayer->y) < 32 && pPlayer->z >= TOWER_RADIUS-8 && pPlayer->z <= (TOWER_RADIUS+PLATFORM_WIDTH) );
#else
    return FALSE;
#endif
}

BOOL CheckMonsters( PLAYER* pPlayer )
{
#ifdef DANGEROUS_MONSTERS
    struct MonsterData* pMonster = g_MonsterData;
    int nCount = g_nMonsterCount;

    while( nCount-- )
    {
        if( pMonster->state == MONSTER_STATE_ALIVE || pMonster->state == MONSTER_STATE_ATTACK || pMonster->state == MONSTER_STATE_STUN )
        {
            register float dx, dy;
            dx = GetTowerDistanceX(pMonster->x, pPlayer->x);
            dy = pMonster->y - pPlayer->y;

            if( dx*dx + dy*dy < 512.0f )//(16.0f*16.0f) )
                return TRUE;

            //if( fabs(pMonster->y-pPlayer->y)<24 && GetTowerDistanceX(pMonster->x,pPlayer->x)<24 ) return TRUE;
        }

        pMonster++;
    }

    if( CheckSpinnyThing( pPlayer ) ) return TRUE;

#endif
    return FALSE;
}


static float g_fWave = 0.0f;

#define REFLECTION_HEIGHT   15.0f
void DrawSubTower( float y )
{
    int iBrick;
    float pos;
    KMDWORD dwColour;
    unsigned char col;
    float fScale = (2*PI) / (float)BRICKS_PER_RING;
    float fRadius = TOWER_RADIUS;
    int nMax = REFLECTION_HEIGHT;
    float du = -(1.0f / (float)BRICKS_PER_RING) * g_fTextureWidth;
    float dv = -((g_fTextureHeight / (float)g_nCurrentTowerHeight)) / (REFLECTION_HEIGHT*0.3f);
    float u = 0.0f;
    float v = -dv*2.0f;
    float h = RING_HEIGHT/ 4.0f;

	kmxxGetCurrentPtr( &VertexBufferDesc );
    while( nMax-- )
    {
        float fRingX[BRICKS_PER_RING];
        float fRingZ[BRICKS_PER_RING];
        KMDWORD dwCol[BRICKS_PER_RING];
        for( iBrick = 0; iBrick < BRICKS_PER_RING; iBrick++ )
        {
            //calculate values
            float fVal = (nMax/REFLECTION_HEIGHT);
            float fThisRadius = fRadius + sin( g_fWave + (iBrick*fScale) + fVal*8.0f ) * 4.0f * (1.0f - fVal*fVal);
            float pos = (iBrick * fScale) + g_fTowerAngle;
            float x, z;
            unsigned char col = GetLightLevel( pos, iBrick ) * fVal;//(((unsigned char)(sin( fmod(pos, PI))*255) * (1.0f - DIRECTIONAL_LIGHT_AMOUNT)) + ((unsigned char)(sin( (fmod((iBrick*fScale), PI*2.0f))/2.0f)*255) * DIRECTIONAL_LIGHT_AMOUNT)) * (fVal);
            KMDWORD dwColour = col << 16 | col << 8 | col;
            GetScreenPos( &x, &z, pos, fThisRadius );
            fRingX[iBrick] = (int)x;
            fRingZ[iBrick] = z;
            dwCol[iBrick] = dwColour;
        }

        //draw the tower
        kmxxStartStrip( &VertexBufferDesc, &kmshBrick );

        for( iBrick = 0; iBrick < BRICKS_PER_RING; iBrick++ )
        {
            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fRingX[iBrick], y+h, fRingZ[iBrick], u, v+dv, dwCol[iBrick], 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fRingX[iBrick], y,   fRingZ[iBrick], u, v, dwCol[iBrick], 0x00000000 );
            u += du;
        }
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fRingX[0], y+h, fRingZ[0], u, v+dv, dwCol[0], 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fRingX[0], y,   fRingZ[0], u, v, dwCol[0], 0x00000000 );

        v += dv;
        u = 0.0f;
        y += h;
    }

    g_fWave += 0.2f;
    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}

float g_foo = 1.0f; //fixme: water current hack
int g_iWater = 0;
void DrawWater( float y )
{
    float z1 = 1.0f / (((20000) / 10000.0f) + 1.0f), z2 = 1.0f / (((-5500) / 10000.0f) + 1.0f);
    float u = 1.0f, v = 1.0f;
    float yoff = 64.0f + (((y-360.0f) / ((float)(GetTowerHeight()<<1))) * 16.0f );

    FloatRotate( &u, &v, g_fTowerAngle );
    
    g_iWater = (g_iWater + 1) % 20;
    kmChangeStripTextureSurface( &kmshWater, KM_IMAGE_PARAM1, &kmsdWater[g_iWater / 2] );

	kmxxGetCurrentPtr( &VertexBufferDesc );
    kmxxStartStrip( &VertexBufferDesc, &kmshWater );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,    -1500.0f,        y+120.0f, z2, g_foo + -u*1.2f,  v*2.0f, 0x60C0C0C0, 0x000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     0.0f,           y-yoff,   z1, g_foo + -v*1.2f, -u*2.0f, 0xFF000000, 0x000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     640.0f+1500.0f, y+120.0f, z2, g_foo +  v*1.2f,  u*2.0f, 0x60C0C0C0, 0x000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, 640.0f,         y-yoff,   z1, g_foo +  u*1.2f, -v*2.0f, 0xFF000000, 0x000000 );
    kmxxReleaseCurrentPtr(&VertexBufferDesc);

    g_foo += 0.005f;
    if( g_foo > 1.0f ) g_foo -= 1.0f;
}



int walkanim[] = { 0,0, 1,0, 2,0, 3,0, 4,0 };
int standanim[] = { 0,0 };
int fallanim[] = { 0,2, 1,2, 2,2, 3,2 };
int upwalkanim[] = { 3,1, 4,2, 3,1, 5,2 };
int downwalkanim[] = { 4,1, 6,2, 4,1, 7,2 };
int rotateanim[] = { 3,1,   2,1, 1,1, 0,1,   0,0,  7,1, 6,1, 5,1,   4,1,  5,1, 6,1, 7,1,   0,0,  0,1, 1,1, 2,1 };
int fallbackanim[] = { 1,4 };
int fallforwardsanim[] = { 0,4 };

void DrawChar( PLAYER* pPlayer, float fValue )
{
    float x, y, z;
    KMDWORD dwColour;
    KMDWORD dwOffset = 0x000000;
    float offx, offy;
    float r;
    float u, v;
    float du = 0.125f;
    static int s_iFrameTick = 0;
    static int s_iFrameTickMax = 0;
    int i;
    if( ++s_iFrameTickMax > 2 )
    {
        s_iFrameTick++;
        s_iFrameTickMax = 0;
    }

    //get sprite position on screen
    if( (pPlayer->GameState == GAME_STATE_FALL || pPlayer->GameState == GAME_STATE_DROWN ) && (pPlayer->GameStateCount&4) ) dwOffset = 0x400000;//AdjustColour( dwColour, 0x200000 );
    GetPosition( pPlayer->x, pPlayer->y, pPlayer->z, &x, &y, &z, &dwColour, dwOffset );
    dwColour = ScaleRGB( dwColour, fValue );
    r = GetZScaling(z) * 16.0f;



    //set default rotating cutout
    i = (int)(RadiansToDegrees(pPlayer->angle) / 22.5f) << 1;
    u = (float)rotateanim[i] / 8.0f;
    v = (float)rotateanim[i+1] / 8.0f;

    //determine the UV cutout for the current frame etc.
    switch( pPlayer->GameState )
    {
        case GAME_STATE_FALL:
            i = (s_iFrameTick<<1) % (sizeof(fallanim)/sizeof(int));
            u = (float)fallanim[i] / 8.0f;
            v = (float)fallanim[i+1] / 8.0f;
            break;

        case GAME_STATE_EXIT:
        case GAME_STATE_ENTER:
            if( pPlayer->angle == pPlayer->desired_angle )
            {
                i = (s_iFrameTick<<1) % (sizeof(upwalkanim)/sizeof(int));
                u = (float)upwalkanim[i] / 8.0f;
                v = (float)upwalkanim[i+1] / 8.0f;
            }
            break;

        case GAME_STATE_LEAVE:
        case GAME_STATE_DONEEXIT:
            if( pPlayer->angle == pPlayer->desired_angle )
            {
                i = (s_iFrameTick<<1) % (sizeof(downwalkanim)/sizeof(int));
                u = (float)downwalkanim[i] / 8.0f;
                v = (float)downwalkanim[i+1] / 8.0f;
            }
            break;

        case GAME_STATE_DROWN:
            u = (float)fallanim[2] / 8.0f;
            v = (float)fallanim[3] / 8.0f;
            break;

        case GAME_STATE_DEFAULT:
            if( pPlayer->angle == pPlayer->desired_angle && pPlayer->dx != 0.0f )
            {
                if( pPlayer->dy > -4.0f ) //if not at terminal velocity!
                {
                    i = (s_iFrameTick<<1) % (sizeof(walkanim)/sizeof(int));
                    u = (float)walkanim[i] / 8.0f;
                    v = (float)walkanim[i+1] / 8.0f;
                }
                else
                {
                    if( (pPlayer->angle < DegreesToRadians(180.0f) && pPlayer->dx > 0.0f) || (pPlayer->angle > DegreesToRadians(180.0f) && pPlayer->dx < 0.0f) )
                    {
                        u = (float)fallforwardsanim[0] / 8.0f;
                        v = (float)fallforwardsanim[1] / 8.0f;
                    }
                    else
                    {
                        u = (float)fallbackanim[0] / 8.0f;
                        v = (float)fallbackanim[1] / 8.0f;
                    }
                }
            }
            break;

        case GAME_STATE_GETREADY:
        case GAME_STATE_COLLAPSE:
        case GAME_STATE_RESCUE:
        case GAME_STATE_LIFT:
        case GAME_STATE_ROTATE:
        default:
            break;
    }

    if( pPlayer->angle > DegreesToRadians(180.0f) ) du = -du;
    if( u < 0.0f ) { u = -u; du = -du; }
    if( du < 0.0f ) u -= du;

    u += 0.001953125f;
    v += 0.001953125f;
    du -= 0.001953125f;

    //draw the sprite
    kmxxGetCurrentPtr( &VertexBufferDesc );
    kmxxStartStrip( &VertexBufferDesc, &kmshPlayer );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-r, y,              z, u,    v+0.125f-0.001953125f, dwColour, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-r, y-(r*2.0f),     z, u,    v,                     dwColour, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x+r, y,              z, u+du, v+0.125f-0.001953125f, dwColour, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+r, y-(r*2.0f), z, u+du, v,                     dwColour, 0x00000000 );


    /*
    kmxxGetCurrentPtr( &VertexBufferDesc );
    kmxxStartStrip( &VertexBufferDesc, &kmshSprite );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-r, y,        z, 0.0f, 1.0f, dwColour, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-r, y-(r*2.0f),     z, 0.0f, 0.0f, dwColour, 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x+r, y,        z, 1.0f, 1.0f, dwColour, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+r, y-(r*2.0f), z, 1.0f, 0.0f, dwColour, 0x00000000 );
    
    offx = 0, offy = 16;
    FloatRotate( &offx, &offy, pPlayer->angle );

    kmxxStartStrip( &VertexBufferDesc, &kmshSprite );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x+offx-4, y+offy+4-r,        Z_BOOST+z, 0.0f, 1.0f, 0xFF000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x+offx-4, y+offy-4-r,     Z_BOOST+z, 0.0f, 0.0f, 0xFF000000, 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x+offx+4, y+offy+4-r,        Z_BOOST+z, 1.0f, 1.0f, 0xFF000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+offx+4, y+offy-4-r, Z_BOOST+z, 1.0f, 0.0f, 0xFF000000, 0x00000000 );
*/
  
    /*
    
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-4+(16.0f*fDirection), y-12,        Z_BOOST+z, 0.0f, 1.0f, 0xFF000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-4+(16.0f*fDirection), y-20,     Z_BOOST+z, 0.0f, 0.0f, 0xFF000000, 0x00000000 );

    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x+4+(16.0f*fDirection), y-12,        Z_BOOST+z, 1.0f, 1.0f, 0xFF000000, 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+4+(16.0f*fDirection), y-20, Z_BOOST+z, 1.0f, 0.0f, 0xFF000000, 0x00000000 );
*/
    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}

void DrawBullet( PLAYER* pPlayer )
{
    if( pPlayer->bullet_life )
    {
        float x, y, z;
        KMDWORD dwColour, dwOffset = 0x006060;
        float r;

        if(g_nCurrentTime&4) dwOffset = 0x606060;

        GetPosition( pPlayer->bullet_x, pPlayer->bullet_y, (TOWER_RADIUS+PLATFORM_WIDTH)-16.0f, &x, &y, &z, &dwColour, dwOffset );
        r = GetZScaling(z) * BULLET_RADIUS;

        kmxxGetCurrentPtr( &VertexBufferDesc );
        kmxxStartStrip( &VertexBufferDesc, &kmshSprite );

        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-r, y+r,        z, 0.0f, 1.0f, dwColour, 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-r, y-r,     z, 0.0f, 0.0f, dwColour, 0x00000000 );

        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x+r, y+r,        z, 1.0f, 1.0f, dwColour, 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+r, y-r, z, 1.0f, 0.0f, dwColour, 0x00000000 );

        kmxxReleaseCurrentPtr(&VertexBufferDesc);
    }
}

void DrawMonster( int i )
{
    static int s_iFlash = 0;
    float x, y, z;
    KMDWORD dwColour;
    KMDWORD dwOffset = 0x000000;
    float r;

    if( g_MonsterData[i].state != MONSTER_STATE_INVISIBLE )
    {
        KMSTRIPHEAD* pkmsh = &kmshSprite;
        switch( g_MonsterData[i].type )
        {
            case MONSTER_TYPE_UPDOWN:
            case MONSTER_TYPE_LEFTRIGHT:
                pkmsh = &kmshSimple;
                break;
            case MONSTER_TYPE_ROVERBADASS:
                pkmsh = &kmshRoverBlack;
                break;
            case MONSTER_TYPE_ROVER:
            case MONSTER_TYPE_ROVERANGRY:
                pkmsh = &kmshRover;
                break;
        }

        if( g_MonsterData[i].state == MONSTER_STATE_DIEJUMP && (g_MonsterData[i].stateCount&4) ) dwOffset = 0x400000;//dwColour = AdjustColour( dwColour, 0x200000 );
        GetPosition( g_MonsterData[i].x, g_MonsterData[i].y, g_MonsterData[i].z, &x, &y, &z, &dwColour, dwOffset );
        r = GetZScaling(z) * 16.0f;

        kmxxGetCurrentPtr( &VertexBufferDesc );
        
        kmxxStartStrip( &VertexBufferDesc, pkmsh );

        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-r, y,        z, 0.0f, 1.0f, dwColour, 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x-r, y-(r*2.0f),     z, 0.0f, 0.0f, dwColour, 0x00000000 );

        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, x+r, y,        z, 1.0f, 1.0f, dwColour, 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+r, y-(r*2.0f), z, 1.0f, 0.0f, dwColour, 0x00000000 );
    
        kmxxReleaseCurrentPtr(&VertexBufferDesc);
    }
}

void DrawBubbles()
{
    int i;

	kmxxGetCurrentPtr( &VertexBufferDesc );
    
    for( i = 0; i < MAX_BUBBLES; i++ )
    {
        if( g_BubbleData[i].size != 0 )
        {
            float x, y, z, s;
            KMDWORD dwCol;
            GetPosition( g_BubbleData[i].x, g_BubbleData[i].y, g_BubbleData[i].z, &x, &y, &z, &dwCol, 0 );
            s = 2.0f + (g_BubbleData[i].size / 2.0f);

            kmxxStartStrip( &VertexBufferDesc, &kmshBubble );

            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-s, y+s, z, 0.0f, 1.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-s, y-s, z, 0.0f, 0.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x+s, y+s, z, 1.0f, 1.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+s, y-s, z, 1.0f, 0.0f, dwCol, 0x00000000 );

        }
    }
    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}

void DrawSplashes()
{
    int i;
	kmxxGetCurrentPtr( &VertexBufferDesc );
    for( i = 0; i < MAX_SPLASHES; i++ )
    {
        if( g_SplashData[i].length )
        {
            float x, y, z, off = ((float)g_SplashData[i].length/30.0f),  w = 32.0f + (16.0f*( 1.0f - off)), h = 32.0f + (16.0f*( 1.0f - off));
            KMDWORD dwCol =((g_SplashData[i].length * 8)<<24) | 0xC0E0F0;


            GetPosition( g_SplashData[i].x, -8.0f + 16.0f*off, g_SplashData[i].z, &x, &y, &z, NULL, 0 );


            y *= z;

            kmxxStartStrip( &VertexBufferDesc, &kmshSplash );

            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-(w*(1+(1.0f-off))), y+(h*off), z, 0.0f, 1.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-(w*(1+(1.0f-off))), y-(h*off), z, 0.0f, 0.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x+(w*(1+(1.0f-off))), y+(h*off), z, 1.0f, 1.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+(w*(1+(1.0f-off))), y-(h*off), z, 1.0f, 0.0f, dwCol, 0x00000000 );


            //FloatRotate( &w, &h, g_SplashData[i].length / 32.0f );
            //kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-w, y+(h*0.65f), z, 0.0f, 1.0f, dwCol, 0x00000000 );
            //kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-h, y-(w*0.65f), z, 0.0f, 0.0f, dwCol, 0x00000000 );
            //kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x+h, y+(w*0.65f), z, 1.0f, 1.0f, dwCol, 0x00000000 );
            //kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+w, y-(h*0.65f), z, 1.0f, 0.0f, dwCol, 0x00000000 );
        }
    }
    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}
void DrawFragments()
{
    int i;
	kmxxGetCurrentPtr( &VertexBufferDesc );
    for( i = 0; i < MAX_FRAGMENTS; i++ )
    {
        if( g_FragmentData[i].length )
        {
            float x, y, z, w = g_FragmentData[i].w, h = g_FragmentData[i].h;
            KMDWORD dwCol;
            GetPosition( g_FragmentData[i].x, g_FragmentData[i].y, g_FragmentData[i].z, &x, &y, &z, &dwCol, 0 );

            FloatRotate( &w, &h, g_FragmentData[i].length * ((i&1)?0.2f:-0.2f) );

            kmxxStartStrip( &VertexBufferDesc, g_FragmentData[i].pkmsh );

            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-w, y+h, z, 0.0f, 1.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-h, y-w, z, 0.0f, 0.0f, dwCol, 0x00000000 );
            //kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x+h, y+w, z, 1.0f, 1.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+w, y-h, z, 1.0f, 0.0f, dwCol, 0x00000000 );
        }
    }
    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}

void DrawPuffs()
{
    int i;
	kmxxGetCurrentPtr( &VertexBufferDesc );
    for( i = 0; i < MAX_PUFFS; i++ )
    {
        if( g_PuffData[i].length )
        {
            float x, y, z, off = 1.0f - ((float)g_PuffData[i].length/60.0f),  w = g_PuffData[i].size + (g_PuffData[i].size*off), h = w;
            KMDWORD dwCol =((g_PuffData[i].length * 16)<<24) | g_PuffData[i].dwColour;
            GetPosition( g_PuffData[i].x, g_PuffData[i].y, g_PuffData[i].z, &x, &y, &z, NULL, 0 );

            z = 1.0f / (((-5500) / 10000.0f) + 1.0f);

            FloatRotate( &w, &h, g_PuffData[i].length / 16.0f );

            kmxxStartStrip( &VertexBufferDesc, &kmshPuff );

            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-w, y+h, z, 0.0f, 1.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-h, y-w, z, 0.0f, 0.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x+h, y+w, z, 1.0f, 1.0f, dwCol, 0x00000000 );
            kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+w, y-h, z, 1.0f, 0.0f, dwCol, 0x00000000 );
        }
    }
    kmxxReleaseCurrentPtr(&VertexBufferDesc);
}

KMDWORD CalcBumpLightValue( PLAYER* pPlayer )
{
    return ((BYTE)((GetTowerAngle(pPlayer->x)/(PI*2))*255.0f)) | 0x8000FF00; 
}

void DrawTower( PLAYER* pPlayer )
{
    TowerRing* pTower = TheTower;
    int i = 0, iLift = 0;
    float u = 0;
    float du = (1.0f / (float)BRICKS_PER_RING) * g_fTextureWidth;
    float h = g_nCurrentTowerHeight*RING_HEIGHT;
    float v;
    float cur_y;
    static int mmm = 0;

    cur_y = (int)g_fHeight;

    DrawSky( g_fTowerAngle, cur_y - 360, 320 );

    switch( pPlayer->GameState )
    {
        default:
        case GAME_STATE_DEFAULT:
            DrawChar( pPlayer, 1.0f );
            break;

        case GAME_STATE_COLLAPSE:
            DrawChar( pPlayer, 1.0f );
            break;

        case GAME_STATE_EXIT:
        case GAME_STATE_ENTER:
        {
            register float f = ((float)pPlayer->GameStateCount/(float)GAME_STATE_ENTERLEAVE_TIME);
            DrawChar( pPlayer, f );
            break;
        }

        case GAME_STATE_DONEEXIT:
        case GAME_STATE_LEAVE:
        {
            register float f = 1.0f - ((float)pPlayer->GameStateCount/(float)GAME_STATE_ENTERLEAVE_TIME);
            DrawChar( pPlayer, f );
            break;
        }

        case GAME_STATE_ROTATE:
            DrawChar( pPlayer, 0.0f );
            break;

        case GAME_STATE_FALL:
            DrawChar( pPlayer, 1.0f );
            break;
    }

    //draw bullet
    DrawBullet( pPlayer );

    kmxxGetCurrentPtr( &VertexBufferDesc );

#ifdef DRAW_TOWER_WALLS
    //draw the tower
  
    kmxxStartStrip( &VertexBufferDesc, &kmshBrick );
    for( i = 0; i < BRICKS_PER_RING; i++ )
    {
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[i], cur_y, fInnerRingZ[i], u, g_fTextureHeight, dwColours[i], 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[i], cur_y-h, fInnerRingZ[i], u, 0.0f, dwColours[i], 0x00000000 );
        u += du;
    }
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[0], cur_y, fInnerRingZ[0], u, g_fTextureHeight, dwColours[0], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[0], cur_y-h, fInnerRingZ[0], u, 0.0f, dwColours[0], 0x00000000 );

    //calculate tower bump map value
    g_dwBumpLight = CalcBumpLightValue( pPlayer );

    //draw tower bump maps
    kmxxStartStrip( &VertexBufferDesc, &kmshBrickBump );
    u = 0.0f;
    for( i = 0; i < BRICKS_PER_RING; i++ )
    {
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[i], cur_y, fInnerRingZ[i]+Z_BUMPBOOST, u, g_fTextureHeight, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[i], cur_y-h, fInnerRingZ[i]+Z_BUMPBOOST, u, 0.0f, 0xA0000000, g_dwBumpLight );
        u += du;
    }
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[0], cur_y, fInnerRingZ[0]+Z_BUMPBOOST, u, g_fTextureHeight, 0xA0000000, g_dwBumpLight );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[0], cur_y-h, fInnerRingZ[0]+Z_BUMPBOOST, u, 0.0f, 0xA0000000, g_dwBumpLight );

#endif //DRAW_TOWER_WALLS
    
    //draw the top ring
    u = 0;
    v = -((g_fTextureHeight / (float)g_nCurrentTowerHeight)*2);
    kmxxStartStrip( &VertexBufferDesc, &kmshBrick );
    for( i = 0; i < BRICKS_PER_RING; i++ )
    {
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[i], cur_y-h, fInnerRingZ[i], u, v, dwColours[i], 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[i], cur_y-h-(RING_HEIGHT*2), fInnerRingZ[i], u, 0.0f, 0x00000000, 0x00000000 );
        u += du;
    }
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[0], cur_y-h, fInnerRingZ[0], u, v, dwColours[0], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[0], cur_y-h-(RING_HEIGHT*2), fInnerRingZ[0], u, 0.0f, 0x00000000, 0x00000000 );

    //draw the top ring bump
    u = 0;
    kmxxStartStrip( &VertexBufferDesc, &kmshBrickBump );
    for( i = 0; i < BRICKS_PER_RING; i++ )
    {
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[i], cur_y-h, fInnerRingZ[i]+Z_BUMPBOOST, u, v, 0xA0000000, g_dwBumpLight );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[i], cur_y-h-(RING_HEIGHT*2), fInnerRingZ[i]+Z_BUMPBOOST, u, 0.0f, 0xA0000000, g_dwBumpLight );
        u += du;
    }
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fInnerRingX[0], cur_y-h, fInnerRingZ[0]+Z_BUMPBOOST, u, v, 0xA0000000, g_dwBumpLight );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, fInnerRingX[0], cur_y-h-(RING_HEIGHT*2), fInnerRingZ[0]+Z_BUMPBOOST, u, 0.0f, 0xA0000000, g_dwBumpLight );


    //draw the top of the tower
    kmxxStartStrip( &VertexBufferDesc, &kmshRoof );
    u = 0; du = (1.0f / (float)BRICKS_PER_RING) * 3.0f;
    for( i = 0; i < BRICKS_PER_RING; i++ )
    {
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[i], cur_y-h-(RING_HEIGHT*2), fOuterRingZ[i], u, 0.0f, dwColours[i], 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, 320.0f, cur_y-h-TOWER_ROOF_HEIGHT, 0.968992248062f, u, 1.0f, dwColours[i], 0x00000000 );
        u += du;
    }
    kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL, fOuterRingX[0], cur_y-h-(RING_HEIGHT*2), fOuterRingZ[0], u, 0.0f, dwColours[0], 0x00000000 );
    kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, 320.0f, cur_y-h-TOWER_ROOF_HEIGHT, 0.968992248062f, u, 1.0f, dwColours[0], 0x00000000 );

    
    kmxxReleaseCurrentPtr(&VertexBufferDesc);
    //draw all lifts
    for( i = 0; i < g_nLiftCount; i++ )
    {
        if( g_LiftData[i].state != LIFT_STATE_INVISIBLE ) DrawPlat( g_fHeight-(g_LiftData[i].base_y + g_LiftData[i].y), g_LiftData[i].iBrick, &kmshLift, 0/*, NULL*/ );
    }
  




    //draw the water
    DrawWater( cur_y );

    //draw the rippling sub-aquatic part of the tower
    DrawSubTower( cur_y );

    //draw the bubbles
    DrawBubbles();

    //draw all tower blocks
    i = g_nTowerHeight - 1;
    while( *pTower[0] ) 
    {
        iLift = DrawRing( cur_y-h, *pTower++, i++, iLift );
        cur_y += RING_HEIGHT;
    }

    //draw all monsters
    for( i = 0; i < g_nMonsterCount; i++ )
    {
        DrawMonster( i );
    }

    //draw spinny thing
    if( pPlayer->nSpinnyThingCount == 0 )
    {
        float x, y, z, w = 16.0f, h = 16.0f;
        x = pPlayer->fSpinnyThingX;
        y = g_fHeight - pPlayer->fSpinnyThingY - 16;
        z = 1.0f / (((320.0f - ( (TOWER_RADIUS+PLATFORM_WIDTH) )) / 10000.0f) + 1.0f);

        FloatRotate( &w, &h, DegreesToRadians(g_nCurrentTime%360) * (pPlayer->bSpinnyThingToggle?10.0f:-10.0f) );


        kmxxGetCurrentPtr( &VertexBufferDesc );
        kmxxStartStrip( &VertexBufferDesc, &kmshSpinny );

        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-w, y+h, z, 0.0f, 1.0f, 0xFFFFFFFF, 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x-h, y-w, z, 0.0f, 0.0f, 0xFFFFFFFF, 0x00000000 );

        kmxxSetVertex_3( KM_VERTEXPARAM_NORMAL,     x+h, y+w, z, 1.0f, 1.0f, 0xFFFFFFFF, 0x00000000 );
        kmxxSetVertex_3( KM_VERTEXPARAM_ENDOFSTRIP, x+w, y-h, z, 1.0f, 0.0f, 0xFFFFFFFF, 0x00000000 );

        kmxxReleaseCurrentPtr(&VertexBufferDesc);
    }


    //draw HUD
    DrawHUD( pPlayer );

    //draw special effects
    DrawSplashes();
    DrawPuffs();
    DrawFragments();
}


void ResetPlayer( PLAYER* pPlayer )
{
    int iRing = 0, iBrick;
    TowerRing* pTower = TheTower;

    pPlayer->x = pPlayer->y = 0;

    //set defaults
    pPlayer->z = (TOWER_RADIUS+PLATFORM_WIDTH)-16.0f;
    pPlayer->angle = pPlayer->desired_angle = pPlayer->fOldAngle = PI/2.0f;
    pPlayer->dx = 0.0f;
    pPlayer->dy = 0.0f;
    pPlayer->bOnGround = TRUE;
    pPlayer->iCurrentLift = -1;
    pPlayer->nSpinnyThingCount = SPINNY_THING_DEFAULT_TIME;
    pPlayer->bSpinnyThingToggle = FALSE;
    pPlayer->bSpinnyThingEnabled = TRUE;
    pPlayer->fSpinnyThingX = 640+16;
    pPlayer->GameState = GAME_STATE_GETREADY;
    pPlayer->GameStateCount = GAME_STATE_GETREADY_TIME;
    pPlayer->bullet_life = 0;
    pPlayer->bFirePress = FALSE;

    while( *pTower[0] != B_NULL )
    {
        for( iBrick = 0; iBrick < BRICKS_PER_RING; iBrick++ )
        {
            if( (TheTower[iRing][iBrick] & B_TYPE_MASK) == B_STRT )
            {
                TheTower[iRing][iBrick] = (TheTower[iRing][iBrick]&B_MOD_MASK) | B_NORM;
                pPlayer->x = (iBrick*BRICK_SIZE) + (BRICK_SIZE>>1);
                pPlayer->y = GetTowerHeight() - ((iRing+1) * RING_HEIGHT);
                return;
            }
        }

        iRing++;
        pTower++;
    }

}

void ResetTower()
{
    TowerRing* pTower = TheTower;
    float y;
    int i = 0;

    //copy over tower data
    memcpy( TheTower, DefaultTower, sizeof(TowerRing) * (g_nTowerHeight + 1) );
    g_nCurrentTowerHeight = g_nTowerHeight;

    //reset timer
    g_nCurrentTime = 0;

    //process the tower data
    g_nLiftCount = 0;
    y = GetTowerHeight();
    while( *pTower[0] )
    {
         TowerBrick* t;
         int nMax = BRICKS_PER_RING;
         int iBrick = 0;
         t = *pTower;
         while( nMax-- )
         {
             switch( *t & B_TYPE_MASK )
             {
                case B_PLAT:
                    break;

                case B_LIFT:
                    g_LiftData[g_nLiftCount].y = 0;
                    g_LiftData[g_nLiftCount].base_y = y;
                    g_LiftData[g_nLiftCount].height = (RING_HEIGHT*FindLiftHeight( TheTower, pTower, iBrick ));
                    g_LiftData[g_nLiftCount].pBrick = t;
                    g_LiftData[g_nLiftCount].dy = 0.0f;
                    g_LiftData[g_nLiftCount].iBrick = iBrick;
                    g_LiftData[g_nLiftCount].iRing = i;
                    g_LiftData[g_nLiftCount].state = LIFT_STATE_DOWN;
                    g_nLiftCount++;
                    break;
             }

             t++;
             iBrick++;
         }
         pTower++;
         i++;
         y -= RING_HEIGHT;
    }

    //reset monsters
    for( i = 0; i < g_nMonsterCount; i++ )
    {
        g_MonsterData[i].stateCount = 0;
        g_MonsterData[i].dx = g_MonsterData[i].default_dx;
        g_MonsterData[i].dy = g_MonsterData[i].default_dy;
        g_MonsterData[i].state = MONSTER_STATE_ALIVE;
        g_MonsterData[i].x = (g_MonsterData[i].iBrick * BRICK_SIZE) + (BRICK_SIZE>>1);
        g_MonsterData[i].y = GetTowerHeight() - ((g_MonsterData[i].iRing+1) * RING_HEIGHT);
        g_MonsterData[i].z = TOWER_RADIUS+(PLATFORM_WIDTH>>1);
    }

    //set texture scale values
    g_fTextureWidth = 3.0f;
    g_fTextureHeight = ((g_nTowerHeight * RING_HEIGHT) / 128.0f);

    //initialise particle type things
    for( i = 0; i < MAX_BUBBLES; i++ ) g_BubbleData[i].size = 0;
    for( i = 0; i < MAX_SPLASHES; i++ ) g_SplashData[i].length = 0;
    for( i = 0; i < MAX_PUFFS; i++ ) g_PuffData[i].length = 0;
    for( i = 0; i < MAX_FRAGMENTS; i++ ) g_FragmentData[i].length = 0;
}