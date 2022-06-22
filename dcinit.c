#define _KM_USE_VERTEX_MACRO_L5_
#include <shinobi.h>
#include <kamui2.h>
#include <kamuix.h>
#include <sn_fcntl.h>
#include <usrsnasm.h>
#include <sg_syCbl.h>



KMSYSTEMCONFIGSTRUCT kmsc;
KMVERTEXBUFFDESC VertexBufferDesc;

void InitStripContext( PKMSTRIPHEAD pStripHead, PKMSURFACEDESC pSurfaceDesc, KMVERTEXTYPE nVertexType, KMBOOL bTransparent, KMBOOL bClamp )
{
    KMSTRIPCONTEXT StripContext;
    
    memset( &StripContext, 0, sizeof(KMSTRIPCONTEXT));
    StripContext.nSize = sizeof(KMSTRIPCONTEXT);
    StripContext.StripControl.nListType = bTransparent ? KM_TRANS_POLYGON : KM_OPAQUE_POLYGON;
    StripContext.StripControl.nUserClipMode = KM_USERCLIP_DISABLE;
    StripContext.StripControl.nShadowMode = KM_NORMAL_POLYGON;
    StripContext.StripControl.bOffset = KM_FALSE;
    StripContext.StripControl.bGouraud = KM_TRUE;
    StripContext.ObjectControl.nDepthCompare = KM_GREATER;
    StripContext.ObjectControl.nCullingMode = KM_CULLCCW;
    StripContext.ObjectControl.bZWriteDisable = KM_FALSE;
    StripContext.ObjectControl.bDCalcControl = KM_FALSE;
    StripContext.ImageControl[KM_IMAGE_PARAM1].nSRCBlendingMode = bTransparent ? KM_SRCALPHA : KM_ONE;
    StripContext.ImageControl[KM_IMAGE_PARAM1].nDSTBlendingMode = bTransparent ? KM_INVSRCALPHA : KM_ZERO;
    StripContext.ImageControl[KM_IMAGE_PARAM1].bSRCSelect = KM_FALSE;
    StripContext.ImageControl[KM_IMAGE_PARAM1].bDSTSelect = KM_FALSE;
    StripContext.ImageControl[KM_IMAGE_PARAM1].nFogMode = KM_NOFOG;
    StripContext.ImageControl[KM_IMAGE_PARAM1].bColorClamp = KM_FALSE;
    StripContext.ImageControl[KM_IMAGE_PARAM1].bUseAlpha = bTransparent;
    StripContext.ImageControl[KM_IMAGE_PARAM1].bIgnoreTextureAlpha = KM_FALSE;
    StripContext.ImageControl[KM_IMAGE_PARAM1].nFlipUV = KM_NOFLIP;
    StripContext.ImageControl[KM_IMAGE_PARAM1].nClampUV = bClamp ? KM_CLAMP_UV : KM_NOCLAMP;
    StripContext.ImageControl[KM_IMAGE_PARAM1].nFilterMode = KM_BILINEAR;//KM_POINT_SAMPLE;
    StripContext.ImageControl[KM_IMAGE_PARAM1].bSuperSampleMode = KM_FALSE;
    StripContext.ImageControl[KM_IMAGE_PARAM1].dwMipmapAdjust = KM_MIPMAP_D_ADJUST_1_00;
    StripContext.ImageControl[KM_IMAGE_PARAM1].nTextureShadingMode= bTransparent ? KM_MODULATE_ALPHA : KM_MODULATE;
    StripContext.ImageControl[KM_IMAGE_PARAM1].pTextureSurfaceDesc = pSurfaceDesc;

    kmGenerateStripHead( pStripHead, &StripContext, nVertexType );
}



void FloatRotate( float* x, float* y, const float angle )
{
    register float _x = *x; //fixme: use inline instruction
    *x =  _x * cos(angle) + *y * sin(angle);
    *y = -_x * sin(angle) + *y * cos(angle);
}



void DrawRotatedRectangleThing( KMSTRIPHEAD* pkmsh, float ox, float oy, float x, float y, float w, float h, float angle )
{
    KMVERTEX_03 kmvSprite[4];

    ox = -ox;
    oy = -oy;
    angle = -angle;

    kmStartStrip( &VertexBufferDesc, pkmsh );

    kmvSprite[0].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[0].fX = ox;
    kmvSprite[0].fY = oy+h;
    kmvSprite[0].u.fZ = 11.0f;
    kmvSprite[0].fU = 0.0f;
    kmvSprite[0].fV = 1.0f;
    kmvSprite[0].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[0].uOffsetRGB.dwPacked = 0;

    kmvSprite[1].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[1].fX = ox;
    kmvSprite[1].fY = oy;
    kmvSprite[1].u.fZ = 11.0f;
    kmvSprite[1].fU = 0.0f;
    kmvSprite[1].fV = 0.0f;
    kmvSprite[1].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[1].uOffsetRGB.dwPacked = 0;

    kmvSprite[2].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[2].fX = ox+w;
    kmvSprite[2].fY = oy+h;
    kmvSprite[2].u.fZ = 11.0f;
    kmvSprite[2].fU = 1.0f;
    kmvSprite[2].fV = 1.0f;
    kmvSprite[2].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[2].uOffsetRGB.dwPacked = 0;

    kmvSprite[3].ParamControlWord = KM_VERTEXPARAM_ENDOFSTRIP;
    kmvSprite[3].fX = ox+w;
    kmvSprite[3].fY = oy;
    kmvSprite[3].u.fZ = 11.0f;
    kmvSprite[3].fU = 1.0f;
    kmvSprite[3].fV = 0.0f;
    kmvSprite[3].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[3].uOffsetRGB.dwPacked = 0;

    FloatRotate( &kmvSprite[0].fX, &kmvSprite[0].fY, angle );
    FloatRotate( &kmvSprite[1].fX, &kmvSprite[1].fY, angle );
    FloatRotate( &kmvSprite[2].fX, &kmvSprite[2].fY, angle );
    FloatRotate( &kmvSprite[3].fX, &kmvSprite[3].fY, angle );

    kmvSprite[0].fX += x; kmvSprite[0].fY += y;
    kmvSprite[1].fX += x; kmvSprite[1].fY += y;
    kmvSprite[2].fX += x; kmvSprite[2].fY += y;
    kmvSprite[3].fX += x; kmvSprite[3].fY += y;

    kmSetVertex( &VertexBufferDesc, &kmvSprite[0], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[1], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[2], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[3], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
}



void DrawSprite( KMSTRIPHEAD* pkmsh, float x, float y, float w, float h )
{
    KMVERTEX_03 kmvSprite[4];

    kmStartStrip( &VertexBufferDesc, pkmsh );

    kmvSprite[0].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[0].fX = x;
    kmvSprite[0].fY = y+h;
    kmvSprite[0].u.fZ = 11.0f;
    kmvSprite[0].fU = 0.0f;
    kmvSprite[0].fV = 1.0f;
    kmvSprite[0].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[0].uOffsetRGB.dwPacked = 0;

    kmvSprite[1].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[1].fX = x;
    kmvSprite[1].fY = y;
    kmvSprite[1].u.fZ = 11.0f;
    kmvSprite[1].fU = 0.0f;
    kmvSprite[1].fV = 0.0f;
    kmvSprite[1].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[1].uOffsetRGB.dwPacked = 0;

    kmvSprite[2].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[2].fX = x+w;
    kmvSprite[2].fY = y+h;
    kmvSprite[2].u.fZ = 11.0f;
    kmvSprite[2].fU = 1.0f;
    kmvSprite[2].fV = 1.0f;
    kmvSprite[2].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[2].uOffsetRGB.dwPacked = 0;

    kmvSprite[3].ParamControlWord = KM_VERTEXPARAM_ENDOFSTRIP;
    kmvSprite[3].fX = x+w;
    kmvSprite[3].fY = y;
    kmvSprite[3].u.fZ = 11.0f;
    kmvSprite[3].fU = 1.0f;
    kmvSprite[3].fV = 0.0f;
    kmvSprite[3].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[3].uOffsetRGB.dwPacked = 0;

    kmSetVertex( &VertexBufferDesc, &kmvSprite[0], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[1], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[2], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[3], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
}
void DrawSpriteColoured( KMSTRIPHEAD* pkmsh, float x, float y, float w, float h, KMDWORD dwColour )
{
    KMVERTEX_03 kmvSprite[4];

    kmStartStrip( &VertexBufferDesc, pkmsh );

    kmvSprite[0].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[0].fX = x;
    kmvSprite[0].fY = y+h;
    kmvSprite[0].u.fZ = 11.0f;
    kmvSprite[0].fU = 0.0f;
    kmvSprite[0].fV = 1.0f;
    kmvSprite[0].uBaseRGB.dwPacked = dwColour;
    kmvSprite[0].uOffsetRGB.dwPacked = 0;

    kmvSprite[1].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[1].fX = x;
    kmvSprite[1].fY = y;
    kmvSprite[1].u.fZ = 11.0f;
    kmvSprite[1].fU = 0.0f;
    kmvSprite[1].fV = 0.0f;
    kmvSprite[1].uBaseRGB.dwPacked = dwColour;
    kmvSprite[1].uOffsetRGB.dwPacked = 0;

    kmvSprite[2].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[2].fX = x+w;
    kmvSprite[2].fY = y+h;
    kmvSprite[2].u.fZ = 11.0f;
    kmvSprite[2].fU = 1.0f;
    kmvSprite[2].fV = 1.0f;
    kmvSprite[2].uBaseRGB.dwPacked = dwColour;
    kmvSprite[2].uOffsetRGB.dwPacked = 0;

    kmvSprite[3].ParamControlWord = KM_VERTEXPARAM_ENDOFSTRIP;
    kmvSprite[3].fX = x+w;
    kmvSprite[3].fY = y;
    kmvSprite[3].u.fZ = 11.0f;
    kmvSprite[3].fU = 1.0f;
    kmvSprite[3].fV = 0.0f;
    kmvSprite[3].uBaseRGB.dwPacked = dwColour;
    kmvSprite[3].uOffsetRGB.dwPacked = 0;

    kmSetVertex( &VertexBufferDesc, &kmvSprite[0], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[1], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[2], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[3], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
}
void DrawSubSprite( KMSTRIPHEAD* pkmsh, float x, float y, float scale, int i, int nTexW, int nTexH, int nSprW, int nSprH )
{
    KMVERTEX_03 kmvSprite[4];
    float uw = (float)nSprW / (float)nTexW,  uh = (float)nSprH / (float)nTexH;
    float baseU, baseV;
    i *= nSprW;
    baseU = (float)(i & (nTexW-1)) / (float)nTexW;
    baseV = (float)((i / nTexW) * nSprW) / (float)nTexH;

    //create strips
    kmStartStrip( &VertexBufferDesc, pkmsh );

    kmvSprite[0].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[0].fX = x;
    kmvSprite[0].fY = y+(nSprH*scale);
    kmvSprite[0].u.fZ = 11.0f;
    kmvSprite[0].fU = baseU;
    kmvSprite[0].fV = baseV+uh;
    kmvSprite[0].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[0].uOffsetRGB.dwPacked = 0;

    kmvSprite[1].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[1].fX = x;
    kmvSprite[1].fY = y;
    kmvSprite[1].u.fZ = 11.0f;
    kmvSprite[1].fU = baseU;
    kmvSprite[1].fV = baseV;
    kmvSprite[1].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[1].uOffsetRGB.dwPacked = 0;

    kmvSprite[2].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    kmvSprite[2].fX = x+(nSprW*scale);
    kmvSprite[2].fY = y+(nSprH*scale);
    kmvSprite[2].u.fZ = 11.0f;
    kmvSprite[2].fU = baseU+uw;
    kmvSprite[2].fV = baseV+uh;
    kmvSprite[2].uBaseRGB.dwPacked = 0xFFFFFFFF;
    kmvSprite[2].uOffsetRGB.dwPacked = 0;

    kmvSprite[3].ParamControlWord = KM_VERTEXPARAM_ENDOFSTRIP;
    kmvSprite[3].fX = x+(nSprW*scale);
    kmvSprite[3].fY = y;
    kmvSprite[3].u.fZ = 11.0f;
    kmvSprite[3].fU = baseU+uw;
    kmvSprite[3].fV = baseV;
    kmvSprite[3].uBaseRGB.dwPacked = 0xFFC0C0C0;
    kmvSprite[3].uOffsetRGB.dwPacked = 0;

    kmSetVertex( &VertexBufferDesc, &kmvSprite[0], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[1], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[2], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
    kmSetVertex( &VertexBufferDesc, &kmvSprite[3], KM_VERTEXTYPE_03, sizeof(KMVERTEX_03) );
}





#define VERTEXBUFFERSIZE    0x40000     /* Size of vertex buffer, in bytes (to be doubled). */
#define TEXTUREMEMORYSIZE   0x400000    /* VRAM set aside for textures, in bytes (4 MB). */
#define MAXTEXTURES         4096        /* Maximum number of textures to be tracked. */
#define MAXSMALLVQ          0           /* Maximum number of small VQ textures. */

/* Override Shinobi's syMalloc() / syFree() macros. */
#ifdef syMalloc
#undef syMalloc
#define syMalloc    malloc
#endif /* syMalloc */

#ifdef syFree
#undef syFree
#define syFree      free
#endif /* syFree */

void *(*MallocPtr)(unsigned long) = syMalloc;   /* Default allocation is syMalloc(). */
void (*FreePtr)(void *) = syFree;               /* Default de-allocation is syFree(). */
void PALExtCallbackFunc(PVOID pCallbackArguments); 

KMPACKEDARGB    Border;         /* Border color packed ARGB value. */



/* Kamui2 texture work area, must be aligned to 32-byte boundary. */
#ifdef  __MWERKS__
#pragma align (32);
KMDWORD
#else
#ifdef __GNUC__
KMDWORD __attribute__ ((aligned (32)))
#else
#pragma aligndata32(TextureWorkArea)
KMDWORD
#endif /* __GNUC__ */
#endif /* _MWERKS__ */
TextureWorkArea[MAXTEXTURES * 24 / 4 + MAXSMALLVQ * 76 / 4];

#ifdef  __MWERKS__
#pragma align (4);	/* Return Metrowerks alignment to 4-bytes. */
#endif



/* Set up 32-byte aligned malloc() / free() memory management. */
#define Align32Malloc   (*MallocPtr)
#define Align32Free     (*FreePtr)

/* Align a pointer to the nearest 32-byte aligned memory address. */
#define Align32Byte(ADR)        ((((long) ADR) + 0x1F) & 0xFFFFFFE0)

/* Reference physical memory through a cached memory address (SH4 P1 memory region). */
#define SH4_P1CachedMem(ADR)    ((((long) ADR) & 0x0FFFFFFF) | 0x80000000)

/* Reference physical memory through a non-cached memory address (SH4 P2 memory region). */
#define SH4_P2NonCachedMem(ADR) ((((long) ADR) & 0x0FFFFFFF) | 0xA0000000)


void *_Align32Malloc (unsigned long Size)
{
    void *Ptr, *AlignedPtr;

    /* Adjust the requested size to a multiple of 32 bytes. */
    Size = (Size + 0x1F) & 0xFFFFFFE0;

    /* Allocate requested size plus 32 bytes padding. */
    Ptr = syMalloc (Size + 32);

    /* Align to 32-bytes (add 32 bytes if already aligned - the padding is used below). */
    AlignedPtr = (void *) ((((long) Ptr) + 0x20) & 0xFFFFFFE0);

    /* Place cookie one byte earlier for _Align32Free(). */
    *((char *) AlignedPtr - 1) = (char) ((long) AlignedPtr - (long) Ptr);

    return (AlignedPtr);
}


void _Align32Free (void *Ptr)
{
    char Diff;

    /* Read cookie and adjust pointer back to original unaligned address before freeing. */
    Diff = *((char *) Ptr - 1);
    Ptr = (void *) ((long) Ptr - Diff);

    syFree (Ptr);
}



void Init32Malloc (void)
{
    char *Ptr1, *Ptr2;

    /* Use syMalloc() / syFree() by default. */
    MallocPtr = syMalloc;
    FreePtr = syFree;

    Ptr1 = syMalloc (1);
    Ptr2 = syMalloc (1);

    /* Test if either allocation was not 32-byte aligned. */
    if (((long) Ptr1 & 0x1F) || ((long) Ptr2 & 0x1F))
    {
        MallocPtr = _Align32Malloc;
        FreePtr = _Align32Free;
    }

    syFree (Ptr1);
    syFree (Ptr2);
}




VOID STATIC	PALExtCallbackFunc( PVOID pCallbackArguments )
{
    PKMPALEXTINFO pInfo = (PKMPALEXTINFO)pCallbackArguments;
	pInfo->nPALExtMode = KM_PALEXT_HEIGHT_RATIO_1_133;
}

KMSTRIPHEAD StripHeadBackground;
void InitBackground()
{
//  InitStripContext( &StripHeadBackground, NULL, KM_VERTEXTYPE_01, FALSE, FALSE );
}

void DrawBackground()
{
/*  KMVERTEX_01 VertexBackground[3];

    VertexBackground[0].ParamControlWord     = KM_VERTEXPARAM_NORMAL;
    VertexBackground[0].fX                   =   0.0f;
    VertexBackground[0].fY                   = 479.0f;
    VertexBackground[0].u.fZ                 =   0.2f;
    VertexBackground[0].fBaseAlpha           =   1.0f;
    VertexBackground[0].fBaseRed             =   0.0f;
    VertexBackground[0].fBaseGreen           =   0.0f;
    VertexBackground[0].fBaseBlue            =   0.25f;

    VertexBackground[1].ParamControlWord     = KM_VERTEXPARAM_NORMAL;
    VertexBackground[1].fX                   =   0.0f;
    VertexBackground[1].fY                   =   0.0f;
    VertexBackground[1].u.fZ                 =   0.2f;
    VertexBackground[1].fBaseAlpha           =   1.0f;
    VertexBackground[1].fBaseRed             =   0.0f;
    VertexBackground[1].fBaseGreen           =   0.0f;
    VertexBackground[1].fBaseBlue            =   0.0f;

    VertexBackground[2].ParamControlWord     = KM_VERTEXPARAM_ENDOFSTRIP;
    VertexBackground[2].fX                   = 639.0f;
    VertexBackground[2].fY                   = 479.0f;
    VertexBackground[2].u.fZ                 =   0.2f;
    VertexBackground[2].fBaseAlpha           =   1.0f;
    VertexBackground[2].fBaseRed             =   0.0f;
    VertexBackground[2].fBaseGreen           =   0.0f;
    VertexBackground[2].fBaseBlue            =   0.25f;

    kmSetBackGround( &StripHeadBackground, KM_VERTEXTYPE_01, &VertexBackground[0], &VertexBackground[1], &VertexBackground[2]);
*/
}
void CheapAndNastyFade( float r1, float g1, float b1, float r2, float g2, float b2, int nFrames )
{/*
    KMVERTEX_01 VertexBackground[3];
    int i;

    float dr = (r2-r1) / (float)nFrames;
    float dg = (g2-g1) / (float)nFrames;
    float db = (b2-b1) / (float)nFrames;

    VertexBackground[0].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    VertexBackground[0].fX = 0.0f;
    VertexBackground[0].fY = 479.0f;
    VertexBackground[0].u.fZ = 0.2f;
    VertexBackground[0].fBaseAlpha = 1.0f;

    VertexBackground[1].ParamControlWord = KM_VERTEXPARAM_NORMAL;
    VertexBackground[1].fX = 0.0f;
    VertexBackground[1].fY = 0.0f;
    VertexBackground[1].u.fZ = 0.2f;
    VertexBackground[1].fBaseAlpha = 1.0f;

    VertexBackground[2].ParamControlWord = KM_VERTEXPARAM_ENDOFSTRIP;
    VertexBackground[2].fX = 639.0f;
    VertexBackground[2].fY = 479.0f;
    VertexBackground[2].u.fZ = 0.2f;

    for( i = 0; i < nFrames; i++ )
    {
        //set new colour
        VertexBackground[0].fBaseRed  = VertexBackground[1].fBaseRed = VertexBackground[2].fBaseRed = r1;
        VertexBackground[0].fBaseGreen = VertexBackground[1].fBaseGreen = VertexBackground[2].fBaseGreen = g1;
        VertexBackground[0].fBaseBlue = VertexBackground[1].fBaseBlue = VertexBackground[2].fBaseBlue = b1;
        Border.dwPacked = ((int)(r1 * 256.0)<<16) | ((int)(g1 * 256.0)<<8) | ((int)(b1 * 256.0));

        //set background & border colour
        kmSetBackGround( &StripHeadBackground, KM_VERTEXTYPE_01, &VertexBackground[0], &VertexBackground[1], &VertexBackground[2]);
        kmSetBorderColor( Border );

        //update display
        kmBeginScene(&kmsc);
        kmBeginPass(&VertexBufferDesc);
        kmEndPass( &VertexBufferDesc );
        kmRender( KM_RENDER_FLIP );
        kmEndScene( &kmsc );

        //update colour
        r1 += dr;  g1 += dg;  b1 += db;
    }*/
}




void DCInitialise()
{
    KMSURFACEDESC PrimarySurfaceDesc, BackSurfaceDesc;
    PKMSURFACEDESC FBSurfaces[2];
    PKMDWORD VertexBufferPtr;

#ifdef __GNUC__
	shinobi_workaround();
#endif

    //set display mode
    switch( syCblCheck() )
	{
        case SYE_CBL_NTSC:  //U.S./North America NTSC (60Hz) and Brazil PAL-M (60Hz).
            sbInitSystem( KM_DSPMODE_NTSCNI640x480, KM_DSPBPP_RGB565, 1 );
            break;

        case SYE_CBL_PAL:   //Europe PAL (50Hz) and Argentina PAL-N (50Hz).
            //sbInitSystem( KM_DSPMODE_PALNI640x480EXT, KM_DSPBPP_RGB565, 1 );
			//kmSetPALEXTCallback( PALExtCallbackFunc, NULL );
			//kmSetDisplayMode( KM_DSPMODE_PALNI640x480EXT, KM_DSPBPP_RGB565, TRUE, FALSE );
            sbInitSystem( KM_DSPMODE_PALNI640x480, KM_DSPBPP_RGB565, 1 );
            break;

        case SYE_CBL_VGA:   //standard VGA.
            sbInitSystem (KM_DSPMODE_VGA, KM_DSPBPP_RGB565, 1);
            break;

        default:            //unknown video type, return to Dreamcast BootROM.
            syBtExit();     
	}

    //initialise aligned memory allocation
    Init32Malloc();

    //initialise the system configuration structure
    kmsc.dwSize = sizeof(KMSYSTEMCONFIGSTRUCT);
    kmsc.flags  = KM_CONFIGFLAG_ENABLE_CLEAR_FRAMEBUFFER|KM_CONFIGFLAG_NOWAITVSYNC|KM_CONFIGFLAG_ENABLE_2V_LATENCY;
    FBSurfaces[0] = &PrimarySurfaceDesc;
    FBSurfaces[1] = &BackSurfaceDesc;
    kmsc.ppSurfaceDescArray = FBSurfaces;
    kmsc.fb.nNumOfFrameBuffer = 2;
    kmsc.fb.nStripBufferHeight = 32;
    kmsc.nTextureMemorySize = TEXTUREMEMORYSIZE;
    kmsc.nNumOfTextureStruct = MAXTEXTURES;
    kmsc.nNumOfSmallVQStruct = MAXSMALLVQ;
    kmsc.pTextureWork = TextureWorkArea;
    VertexBufferPtr = (PKMDWORD)Align32Malloc( 2 * VERTEXBUFFERSIZE );
    kmsc.pVertexBuffer = (PKMDWORD)SH4_P2NonCachedMem( VertexBufferPtr );
    kmsc.nVertexBufferSize = VERTEXBUFFERSIZE * 2;
    kmsc.pBufferDesc = &VertexBufferDesc;
    kmsc.nNumOfVertexBank = 2;
    kmsc.nPassDepth = 1;
    kmsc.Pass[0].dwRegionArrayFlag = KM_PASSINFO_AUTOSORT;
    kmsc.Pass[0].nDirectTransferList = KM_TRANS_POLYGON;//KM_OPAQUE_POLYGON;
    kmsc.Pass[0].fBufferSize[0] = 40.0f;   // % Opaque polygons (0% if sent direct).
    kmsc.Pass[0].fBufferSize[1] = 10.0f;    // % Opaque modifier.
    kmsc.Pass[0].fBufferSize[2] = 0.0f;    // % Translucent.
    kmsc.Pass[0].fBufferSize[3] = 10.0f;    // % Translucent modifier.
    kmsc.Pass[0].fBufferSize[4] = 40.0f;   // % Punchthrough.

    //set the system configuration
    kmSetSystemConfiguration( &kmsc );

    //initialise background
    InitBackground();

    //fade in from the white (used by the shinobi splash screen) to the cheezy cyan colour
    CheapAndNastyFade( 0.75f, 0.75f, 0.75f, 0.0f, 0.0f, 0.0f,  15 );

    //set border colour
    Border.dwPacked = 0x00000000;
    kmSetBorderColor( Border );
}


void DCShutdown()
{
    CheapAndNastyFade( 0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  15 );
    sbExitSystem();
}


PKMDWORD LoadFile( char* pszFilename )
{
    PKMDWORD pData;
    GDFS gdfs;
    long nFileBlocks;

    if( !( gdfs = gdFsOpen( pszFilename, NULL ) ) ) return NULL;
    gdFsGetFileSctSize( gdfs, &nFileBlocks );
    pData = Align32Malloc( nFileBlocks * 2048 );
    gdFsReqRd32( gdfs, nFileBlocks, pData );
    while( gdFsGetStat( gdfs ) != GDD_STAT_COMPLETE );
    gdFsClose( gdfs );
    return pData;
}

BOOL LoadTexture( KMSURFACEDESC* pSurfaceDesc, const char* pszFilename )
{
    PKMDWORD pTexture, pTwiddled;
    KMTEXTURETYPE nTextureType;
    short nWidth, nHeight;
    int i = 0;

    //load the texture into a file
    pTexture = LoadFile( pszFilename );
    if( pTexture == NULL ) return FALSE;

    //check header
    while(1)
    {
  		if( pTexture[i] == 0x54525650) //"PVRT"
		{
			nTextureType = pTexture[i+2];
            nWidth = (pTexture[i+3] & 0xFFFF );
			nHeight = ((pTexture[i+3]>>16) & 0xFFFF );
			break;
	    }
        i++;
    }

    //create texture surface and load texture
    kmCreateTextureSurface( pSurfaceDesc, nWidth, nHeight, nTextureType );
    kmLoadTexture( pSurfaceDesc, pTexture + (16 / sizeof(KMDWORD)) + i );

    //clean up and return
    Align32Free( pTexture );
    return TRUE;
}

