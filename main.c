#define _KM_USE_VERTEX_MACRO_L5_
#include <math.h>
#include <shinobi.h>
#include <kamui2.h>
#include <kamuix.h>
#include <sn_fcntl.h>
#include <usrsnasm.h>
#include <sg_pdms.h>
#include <private.h>
#include <usrsnasm.h>//for debug_printf
#include "dcinit.h"
#include "tower.h"
#include "dispstr.h"

#define SMOOTH_CAMERA

#define PI      3.1415926536f
#define RadiansToDegrees( r ) ( r * 57.2957795130f )
#define DegreesToRadians( d ) ( d * 0.01745329252f )
#define CAMERA_MOVE_DELAY   (20.0f)
#define CAMERA_CLAMP_THRESH (16.0f)
#define MAX_DIRECTION   8

#define min(a,b)((a<b)?a:b)
#define max(a,b)((a>b)?a:b)
#define sign(v)(v<0?-1:((v>0)?1:0))



void main()
{
    int i = 0, m = 0;
    float fAngle = 0.0f;
    float camera_y = 0.0f;
    float camera_dy = 0.0f;
    float camera_x = 0.0f;
    float camera_real_y = 0.0f;
    float camera_angle = 0.0f;
    PLAYER player;

    float fAltitude = 0.0f;

    //start up
    DCInitialise();
    txt_InitDisplayString();
    InitTower();


    ResetPlayer( &player );
    camera_y = player.y;
    camera_real_y = player.y;


    //main loop
    while(1)
	{
        PDS_PERIPHERAL* per = (PDS_PERIPHERAL*)pdGetPeripheral( PDD_PORT_A0 );

        //hack check for soft reset
        if( (per->on & (PDD_DGT_ST|PDD_DGT_TA|PDD_DGT_TB|PDD_DGT_TX|PDD_DGT_TY)) == (PDD_DGT_ST|PDD_DGT_TA|PDD_DGT_TB|PDD_DGT_TX|PDD_DGT_TY) )
        {
            syBtExit();
        }

        //check for start button to exit
        if( per->press & PDD_DGT_ST )
        {
            ResetTower();
            ResetPlayer( &player );
        }

        //update controls
        if( CanMove( &player ) )
        {
            if( player.bOnGround )
            {
                //check move left/right
                if( per->on & PDD_DGT_KL || per->x1 < -64 )
                    player.dx = -1.5f; 
                else
                    if( per->on & PDD_DGT_KR || per->x1 > 64  ) 
                        player.dx = 1.5f; 
                    else
                        player.dx = 0.0f;

                //check lift/door up
                if( per->press & PDD_DGT_KU || per->y1 < -64  )
                {
                    CheckDoor( &player );
                    CheckLift( &player, FALSE );
                }

                //check lift down
                if( per->press & PDD_DGT_KD || per->y1 > 64  )
                {
                    CheckLift( &player, TRUE );
                }

                //check jump
                if( per->on & PDD_DGT_TA && CanJump( &player ) )
                {
                    if( player.angle == player.desired_angle )
                    {
                        if( player.angle == DegreesToRadians(270.0f) )
                            player.dx = -2.0f;
                        else
                            if( player.angle == DegreesToRadians(90.0f) )
                                player.dx = 2.0f;
                        player.dy = 5.0f;
                    }
                }

                //check fire
                player.bFirePress = ( per->press & PDD_DGT_TB );

                //check smart bomb debug fixme hack
//                if( per->press & PDD_DGT_TY )
//                {
//                    _Nuke();
//                }
            }
        }

        //check for drop off bottom
        if( player.y < -200.0f )
        {
            ResetTower();
            player.dx = player.dy = 0.0f;
            ResetPlayer( &player );
            camera_y = player.y;
            camera_real_y = player.y;
        }

        //update player position
#if 0
        if( per->on & PDD_DGT_TX )
        {
            if( per->on & PDD_DGT_KU ) player.y += 3;
            if( per->on & PDD_DGT_KD ) player.y -= 3;
            if( per->on & PDD_DGT_KR ) player.x += 3;
            if( per->on & PDD_DGT_KL ) player.x -= 3;
            if( player.x > TOWER_CIRCUMFERENCE ) player.x -= TOWER_CIRCUMFERENCE; else
                if( player.x < 0.0f ) player.x += TOWER_CIRCUMFERENCE;

            player.bOnGround = FALSE;
        }
        else
#endif
            player.bOnGround = UpdatePosition( &player );

        //update the camera
#ifdef SMOOTH_CAMERA
        camera_dy = player.y - camera_y;
        if( fabs(camera_dy) < CAMERA_CLAMP_THRESH ) camera_y = player.y; else camera_y += (int)((camera_dy) / CAMERA_MOVE_DELAY);
#else
        camera_y = player.y;
#endif
//        camera_x = ((float)(per->r - per->l) / 256.0f) * (float)((BRICKS_PER_RING>>2)*(BRICK_SIZE));
        camera_angle = GetTowerAngle( player.x + camera_x );
        camera_real_y = camera_y - 120;// + (float)per->y1;
        if( player.GameState == GAME_STATE_COLLAPSE ) camera_real_y += (rand()&0xF) - 8;
        if( camera_real_y > (GetTowerHeight()) ) camera_real_y = (GetTowerHeight());  else if( camera_real_y < 0 ) camera_real_y = 0.0f;

        //update tower
        SetTowerPosition( camera_real_y, camera_angle );
        UpdateTower( &player );


        //begin drawing
        kmBeginScene(&kmsc);
          kmBeginPass(&VertexBufferDesc);
            DrawTower( &player );
          kmEndPass( &VertexBufferDesc );
          kmRender( KM_RENDER_FLIP );
        kmEndScene( &kmsc );


        //update peripherals
        pdExecPeripheralServer();
	}

    //clean up and exit
    DCShutdown();
    syBtExit();
}

