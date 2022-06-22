
extern KMSTRIPCONTEXT DefaultContext;
extern KMSYSTEMCONFIGSTRUCT kmsc;
extern KMVERTEXBUFFDESC VertexBufferDesc;

extern void InitStripContext( PKMSTRIPHEAD pStripHead, PKMSURFACEDESC pSurfaceDesc, KMVERTEXTYPE nVertexType, KMBOOL bTransparent, KMBOOL bClamp );
extern BOOL LoadTexture( KMSURFACEDESC* pSurfaceDesc, const char* pszFilename );
extern void DrawBackground();
extern void DCInitialise();
extern void DCShutdown();

extern void DrawSprite( KMSTRIPHEAD* pkmsh, float x, float y, float w, float h );
extern void DrawSpriteColoured( KMSTRIPHEAD* pkmsh, float x, float y, float w, float h, KMDWORD dwColour );
extern void DrawSubSprite( KMSTRIPHEAD* pkmsh, float x, float y, float scale, int i, int nTexW, int nTexH, int nSprW, int nSprH );
extern void DrawRotatedRectangleThing( KMSTRIPHEAD* pkmsh, float ox, float oy, float x, float y, float w, float h, float angle );

extern PKMDWORD LoadFile( char* pszFilename );

extern void *(*MallocPtr)(unsigned long);
extern void (*FreePtr)(void *);
