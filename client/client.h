/*++

    Black Hat USA 2014
    ------------------
    Understanding TOCTTOU in the Windows Kernel Font Scaler Engine

Module Name:

    client.h

Revision History:

--*/

#ifndef __CLIENT_EXE_H__
#define __CLIENT_EXE_H__


#define FS_PUBLIC
#define FS_ENTRY int32
#define FS_MEMORY_SIZE int32
#define FS_ENTRY_PROTO __cdecl
#define FS_CALLBACK_PROTO __cdecl

typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef long int32;
typedef unsigned long uint32;
typedef long Fixed;

typedef void *voidPtr;
typedef void (FS_CALLBACK_PROTO *ReleaseSFNTFunc)(voidPtr);
typedef void *(FS_CALLBACK_PROTO *GetSFNTFunc)(int32, int32, int32);

#define F26Dot6 long
#define ONEFIX (1L << 16)
#define FIXEDSQRT2 0x00016A0A
#define SIGN(in) ((in) < 0 ? -1 : 1)
#define ABS(in) ((in) < 0 ? -(in) : (in))
#define ROUND_FIXPT(fx) ((SHORT) (SIGN(fx) * (ONEFIX / 2 + ABS(fx)) / ONEFIX))


typedef struct
{
    int16 top;
    int16 left;
    int16 bottom;
    int16 right;
} Rect;

typedef struct
{
    char *baseAddr;
    int16 rowBytes;
    Rect bounds;
} BitMap;

typedef struct
{
    Fixed x, y;
} vectorType;

typedef struct
{
    vectorType advanceWidth;
    vectorType leftSideBearing;
    vectorType leftSideBearingLine;
    vectorType devLeftSideBearingLine;
    vectorType devAdvanceWidth;
    vectorType devLeftSideBearing;
} metricsType;

typedef struct
{
    Fixed transform[3][3];
} transMatrix;

#define MEMORYFRAGMENTS 9           /* extra memory base for overscaled bitmap */

typedef struct
{
    FS_MEMORY_SIZE memorySizes[MEMORYFRAGMENTS];
    uint16 glyphIndex;
    uint16 numberOfBytesTaken;      /* from the character code */
    metricsType metricInfo;
    BitMap bitMapInfo;
    int32 outlineCacheSize;         /* spline data */
    uint16 outlinesExist;
    uint16 numberOfContours;
    F26Dot6 *xPtr, *yPtr;
    int16 *startPtr;
    int16 *endPtr;
    uint8 *onCurve;
    F26Dot6 *scaledCVT;
#ifdef FSCFG_GRAY_SCALE
    uint16 usOverScale;             /* gray scale outline magnification */
#endif
} fs_GlyphInfoType;

typedef struct
{
    Fixed version;
    char *memoryBases[MEMORYFRAGMENTS];
    int32 *sfntDirectory;
    GetSFNTFunc GetSfntFragmentPtr;
    ReleaseSFNTFunc ReleaseSfntFrag;
    int32 clientID;
    union
    {
        struct
        {
            uint16 platformID;
            uint16 specificID;
        } newsfnt;
        struct
        {
            Fixed pointSize;
            int16 xResolution;
            int16 yResolution;
            Fixed pixelDiameter;
            transMatrix *transformMatrix;
            void *traceFunc;
        } newtrans;
        struct
        {
            uint16 characterCode;
            uint16 glyphIndex;
        } newglyph;
        struct
        {
            void (*styleFunc)(fs_GlyphInfoType *);
            void *traceFunc;
        } gridfit;
        struct
        {
            int16 bottomClip;
            int16 topClip;
            int32 *outlineCache;
        } scan;
    } param;
} fs_GlyphInputType;

typedef struct
{
    fs_GlyphInputType input;        /* client interface input */
    fs_GlyphInfoType output;        /* client interface output */
    transMatrix matrix;             /* client matrix transform */
} CLIENTDATA;

static CLIENTDATA client;

#define GRID_FIT_FLAG

#define CALLBACK_TRACE
#ifdef CALLBACK_TRACE
typedef short ShortFract;
#define VECTORTYPE ShortFract

typedef struct
{
    VECTORTYPE x;
    VECTORTYPE y;
} VECTOR;

typedef struct
{
    F26Dot6 *x, *y;
    F26Dot6 *ox, *oy;
    F26Dot6 *oox, *ooy;
    uint8 *onCurve;
    int16 *sp;
    int16 *ep;
    uint8 *f;
    int16 nc;
} fnt_ElementType;

typedef int32 ErrorCode;
typedef void (*FntMoveFunc)(struct fnt_LocalGraphicStateType*, fnt_ElementType*, int32, F26Dot6);
typedef F26Dot6 (*FntProject)(struct fnt_LocalGraphicStateType*, F26Dot6, F26Dot6);
typedef void (*InterpreterFunc)(struct fnt_LocalGraphicStateType*, uint8*, uint8*);
typedef void (FS_CALLBACK_PROTO *FntTraceFunc)(struct fnt_LocalGraphicStateType*, uint8*);

typedef struct
{
    fnt_ElementType *CE0, *CE1, *CE2;
    VECTOR proj;
    VECTOR free;
    VECTOR oldProj;
    F26Dot6 *stackPointer;
    uint8 *insPtr;
    fnt_ElementType *elements;
    void *globalGS;                 /* please fix it yourself, i am afraid i cannot leak more ;) */
    FntTraceFunc TraceFunc;
    int32 Pt0, Pt1, Pt2;
    int16 roundToGrid;
    int32 loop;
    uint8 opCode;
    uint8 padByte;
    int16 padWord;
    VECTORTYPE pfProj;
    FntMoveFunc MovePoint;
    FntProject Project;
    FntProject OldProject;
    InterpreterFunc Interpreter;
#ifdef FSCFG_REENTRANT
    F26Dot6 (*GetCVTEntry)(struct fnt_LocalGraphicStateType*, int32);
    F26Dot6 (*GetSingleWidth)(struct fnt_LocalGraphicStateType*);
#else
    F26Dot6 (*GetCVTEntry)(int32);
    F26Dot6 (*GetSingleWidth)(void);
#endif
    FntMoveFunc ChangeCvt;
    Fixed cvtDiagonalStretch;
    int16 MIRPCode;
    ErrorCode ercReturn;
    uint8 *pbyEndInst;
} fnt_LocalGraphicStateType;
#endif

#define DEBUG_STRING
#ifdef DEBUG_STRING
#define OutLength (1L << 12)
UCHAR OutBuffer[OutLength] = {0};
#endif

#define CHAR_CODE 'C'
#define PLATFORM_ID 1
#define ENCODING_ID 0
#define POINT_SIZE 200


FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_OpenFonts(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_Initialize(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_NewSfnt(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_NewTransformation(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_NewGlyph(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_ContourGridFit(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_ContourNoGridFit(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_FindBitMapSize(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_ContourScan(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

FS_PUBLIC
FS_ENTRY
FS_ENTRY_PROTO
fs_CloseFonts(
    fs_GlyphInputType *inputPtr,
    fs_GlyphInfoType *outputPtr
    );

#endif