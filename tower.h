#ifndef _TOWER_H_
#define _TOWER_H_

#define GAME_STATE_DEFAULT      0
#define GAME_STATE_ENTER        1
#define GAME_STATE_ROTATE       2
#define GAME_STATE_LEAVE        3
#define GAME_STATE_LIFT         4
#define GAME_STATE_FALL         5
#define GAME_STATE_EXIT         6
#define GAME_STATE_DONEEXIT     7
#define GAME_STATE_COLLAPSE     8
#define GAME_STATE_RESCUE       9
#define GAME_STATE_DROWN        10
#define GAME_STATE_GETREADY     11

#define GAME_STATE_ENTERLEAVE_TIME          35
#define GAME_STATE_ROTATE_TIME              60
#define GAME_STATE_FALL_TIME                50
#define GAME_STATE_COLLAPSE_RUMBLE_TIME     60
#define GAME_STATE_GETREADY_TIME            120



typedef struct tagPLAYER
{
    //the lift that the player's currently on
    int iCurrentLift; //-1
    
    //the player's current game state information
    int GameState; //= GAME_STATE_DEFAULT;
    int GameStateNext_Center; // = GAME_STATE_DEFAULT;
    int GameStateCount; // = 0;
    float fOldAngle;// = 0.0f;
    BOOL bCentering;

    //player's current position & speed etc.
    float x, y, z;
    float angle;
    float desired_angle;
    float dx, dy;
    BOOL bOnGround;

    //player's spinny thing information
    int nSpinnyThingCount; // = SPINNY_THING_TIME;
    float fSpinnyThingX, fSpinnyThingY;
    BOOL bSpinnyThingToggle;
    BOOL bSpinnyThingEnabled;

    //player shooting properties
    float bullet_x, bullet_y;
    float bullet_dx;
    int bullet_life;
    BOOL bFirePress;

} PLAYER;



#define B_NULL          0
#define B_NORM          1
#define B_PLAT          2
#define B_LIFT          3
#define B_STOP          4
#define B_DOOR          5
#define B_EXIT          6
#define B_CRAK          7
#define B_MVLF          8
#define B_MVRT          9
#define B_SUPP          10
#define B_STRT          11
#define B_SHOT          12

#define B_LIFTSTALK     256
#define B_CRACKING      512    
#define B_NOSPINNY      1024
#define B_DISABLESPINNY 2048
#define B_ENABLESPINNY  4096
//                      8192
//                      16384
#define B_TOUCH         32768 //debug only

#define B_TYPE_MASK     0x00FF
#define B_MOD_MASK      0xFF00


#define BRICKS_PER_RING     16
#define BRICK_SIZE          32
#define RING_HEIGHT         16
#define PLATFORM_WIDTH      32
#define DOOR_HEIGHT         (RING_HEIGHT*3)
#define TOWER_CIRCUMFERENCE 512 //(BRICKS_PER_RING*BRICK_SIZE)
#define TOWER_RADIUS        81.4873308631f //(TOWER_CIRCUMFERENCE / (PI*2.0f))

#define TOWER_ROOF_HEIGHT   128

#define MAX_LIFTS           16
#define MAX_MONSTERS        16
#define MAX_BUBBLES         64
#define MAX_SPLASHES        64
#define MAX_PUFFS           8
#define MAX_FRAGMENTS       32


#define SPINNY_THING_TIME           250
#define SPINNY_THING_DEFAULT_TIME   50
#define BULLET_SPEED                2
#define BULLET_LIFE_TIME            32 //60
#define BULLET_RADIUS               4
#define BULLET_HIT_RADIUS           8

typedef unsigned short int TowerBrick;
typedef TowerBrick TowerRing[BRICKS_PER_RING];

extern void SetTowerPosition( float y, float angle );
extern void InitTower();
extern void DrawTower( PLAYER* pPlayer );
extern int GetTowerHeight();

extern void UpdateTower( PLAYER* pPlayer );
extern BOOL UpdatePosition( PLAYER* pPlayer );
extern void ResetPlayer( PLAYER* pPlayer );

extern void CheckDoor( PLAYER* pPlayer );
extern void CheckLift( PLAYER* pPlayer, BOOL bDown );
extern BOOL CanMove( PLAYER* pPlayer );
extern BOOL CanJump( PLAYER* pPlayer );

extern void _Nuke();

extern float GetTowerAngle( float x );

extern void ResetTower();

#ifdef _WINDOWS
    #pragma pack( push, 1 )
#endif

typedef struct tagTOWERFILEHEADER
{
    unsigned char chMagic[4];
    unsigned short int nTowerHeight;
    unsigned short int nMonsters;
    unsigned char szTextures[8]; //texture subdirectory
} TOWERFILEHEADER, *PTOWERFILEHEADER;

#define MONSTER_TYPE_UPDOWN         0
#define MONSTER_TYPE_LEFTRIGHT      1
#define MONSTER_TYPE_ROVER          2
#define MONSTER_TYPE_ROVERBADASS    3
#define MONSTER_TYPE_ROVERANGRY     4


typedef struct tagFILEMONSTER
{
    unsigned short int iBrick;
    unsigned short int iRing;
    unsigned char type;
    unsigned char delta;
    unsigned short int reserved;
} FILEMONSTER, *PFILEMONSTER;

#ifdef _WINDOWS
    #pragma pack( pop )
#endif


#endif //_TOWER_H_
