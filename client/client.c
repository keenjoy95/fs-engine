/*++

    Black Hat USA 2014
    ------------------
    Understanding TOCTTOU in the Windows Kernel Font Scaler Engine

Module Name:

    client.c

Revision History:

--*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include "client.h"


FORCEINLINE
PVOID
MallocData(
    IN FS_MEMORY_SIZE Size
    )
{
    if (Size == 0) return NULL;

    return malloc(Size);
}


FORCEINLINE
VOID
FreeData(
    IN PVOID Data
    )
{
    free(Data);
}


VOID
AllocateClientData(
    IN ULONG Begin,
    IN ULONG End
    )
{
    ULONG Index = 0;
    FS_MEMORY_SIZE Size = 0;

    for (Index = Begin; Index <= End; Index++)
    {
        if (client.output.memorySizes[Index] == 0)
            client.input.memoryBases[Index] = NULL;
        else
        {
            Size = client.output.memorySizes[Index];
            client.input.memoryBases[Index] = MallocData(Size);
        }
    }
}


FORCEINLINE
VOID
DumpSpaces(
    IN SHORT Spaces,
    IN FILE *Output
    )
{
    SHORT Index = 0;

    for (Index; Index < Spaces; Index++) putc(' ', Output);
}


FORCEINLINE
VOID
PutNewLine(
    IN FILE *Output
    )
{
    putc('\r', Output);
    putc('\n', Output);
}


FORCEINLINE
VOID
DumpPixels(
    IN PCHAR Pixels,
    IN USHORT Number,
    IN FILE *Output
    )
{
    CHAR OutChar = 0;
    UCHAR Bits = 0, Mask = 0;
    USHORT Index = 0, Count = 0;

#ifdef DEBUG_STRING
    memset(OutBuffer, 0, OutLength);

    for (Index; Index < Number; Index++)
        _snprintf(OutBuffer + strlen(OutBuffer), OutLength, "%c%c", '+', '-');
    _snprintf(OutBuffer + strlen(OutBuffer), OutLength, "+\n");
    OutputDebugString(OutBuffer);

    memset(OutBuffer, 0, OutLength);
#endif

    for (Index = 0; Index < Number; Index += 8, Pixels++)
    {
        Bits = *Pixels;

        for (Count = 0, Mask = 0x80; Mask && ((Index + Count) < Number); Mask >>= 1, Count++)
        {
        #ifdef DEBUG_STRING
            _snprintf(OutBuffer + strlen(OutBuffer), OutLength, "|%c", ((Mask & Bits) == 0x00) ? ' ' : '#');
        #endif
            OutChar = ((Mask & Bits) == 0x00) ? ' ' : '#';
            putc(OutChar, Output);
        }
    }

#ifdef DEBUG_STRING
    _snprintf(OutBuffer + strlen(OutBuffer), OutLength, "|\n");
    OutputDebugString(OutBuffer);
#endif
}


FORCEINLINE
ULONG
DumpBitmap(
    IN PCHAR BaseAddress,
    IN SHORT Left,
    IN SHORT Top,
    IN SHORT Right,
    IN SHORT Bottom,
    IN SHORT Raster,
    IN SHORT AdvanceWidth,
    IN FILE *Output
    )
{
    USHORT cxBlack = Right - Left, cyBlack = Top - Bottom;
    USHORT Index = 0, cxAdvanceWidth = (USHORT) AdvanceWidth;
    SHORT RightSideBearing = cxAdvanceWidth - (Left + cxBlack);

    PutNewLine(Output);
    for (Index; Index < cyBlack; Index++, BaseAddress += Raster)
    {
        DumpSpaces(Left, Output);
        DumpPixels(BaseAddress, cxBlack, Output);
        DumpSpaces(RightSideBearing, Output);
        PutNewLine(Output);
    }

#ifdef DEBUG_STRING
    memset(OutBuffer, 0, OutLength);

    for (Index = 0; Index < cxBlack; Index++)
        _snprintf(OutBuffer + strlen(OutBuffer), OutLength, "%c%c", '+', '-');
    _snprintf(OutBuffer + strlen(OutBuffer), OutLength, "+\n");
    OutputDebugString(OutBuffer);
#endif

    return ferror(Output) == 0 && feof(Output) == 0;
}


PVOID
FS_CALLBACK_PROTO
CallbackGetSfntFragment(
    IN LONG ClientID,
    IN LONG Offset,
    IN LONG Length
    )
{
    PVOID Buffer = NULL;
    ULONG NumberOfBytesRead = 0;

    if (Length == 0 || (Buffer = MallocData(Length)) == NULL) return NULL;

    if (INVALID_SET_FILE_POINTER != SetFilePointer((HANDLE) ClientID, Offset, NULL, FILE_BEGIN))
    {
        if (ReadFile((HANDLE) ClientID, Buffer, Length, &NumberOfBytesRead, NULL)) return Buffer;
    }

    if (Buffer) FreeData(Buffer);

    return NULL;
}


VOID
FS_CALLBACK_PROTO
CallbackReleaseSfntFragment(
    IN PVOID Buffer
    )
{
    if (Buffer) FreeData(Buffer);
}


#ifdef CALLBACK_TRACE

VOID
FS_CALLBACK_PROTO
CallbackTracePreProgram(
    IN fnt_LocalGraphicStateType *LocalGS,
    IN uint8 *pbyEndInst
    )
{
    //
    // ChildEBP RetAddr  Args to Child
    // 000cfe10 040109d1 040186a8 001cae76 000cfe90 client!CallbackTracePreProgram
    // 000cfe24 04010ba6 001c97f0 001c97bc 001c9764 client!itrp_InnerTraceExecute+0x31
    // 000cfe90 0401426d 001c9f0e 001cae76 001c9764 client!itrp_Execute+0x1a6
    // 000cfec4 0400bb43 001c9764 04002030 001c1c78 client!itrp_ExecutePrePgm+0x5d
    // 000cfee0 040074dc 001c87a4 001c9764 001c1ce4 client!fsg_RunPreProgram+0x73
    // 000cff18 040021ed 04016fc0 0401700c 00000004 client!fs_NewTransformation+0x18c
    // 000cff44 04002b08 00000002 001c0dc8 001c0e38 client!main+0x19d
    //

    return;
}


VOID
FS_CALLBACK_PROTO
CallbackTraceExecuteGlyph(
    IN fnt_LocalGraphicStateType *LocalGS,
    IN uint8 *pbyEndInst
    )
{
    //
    // ChildEBP RetAddr  Args to Child
    // 000cfd00 040109d1 040186a8 0045b178 000cfd80 client!CallbackTraceExecuteGlyph
    // 000cfd14 04010ba6 004597f0 004597bc 00459764 client!itrp_InnerTraceExecute+0x31
    // 000cfd80 040142f5 0045b076 0045b178 00459764 client!itrp_Execute+0x1a6
    // 000cfda8 0400bc0f 0045b076 0045b178 00459764 client!itrp_ExecuteGlyphPgm+0x45
    // 000cfdd8 0400c22d 00451ec8 00000001 04002040 client!fsg_SimpleInnerGridFit+0xbf
    // 000cfe38 0400c559 00451c78 00000008 00459764 client!fsg_ExecuteGlyph+0x12d
    // 000cfe80 0400c626 00451c78 00459764 00451ce4 client!fsg_CreateGlyphData+0xc9
    // 000cfeb8 04007851 00451c78 00459764 00451ce4 client!fsg_GridFit+0x46
    // 000cff0c 040078b2 00000001 000cff44 0400223d client!fs__Contour+0x211
    // 000cff18 0400223d 04016fc0 0401700c 00000004 client!fs_ContourGridFit+0x12
    // 000cff44 04002b08 00000002 00450dc8 00450e38 client!main+0x1ed
    //

    return;
}

#endif


VOID
__cdecl main(
    IN ULONG argc,
    IN PCHAR argv[]
    )
{
    FS_ENTRY Code = NO_ERROR;
    SHORT Left = 0, AdvanceWidth = 0, cx = 0;
    HANDLE FontHandle = INVALID_HANDLE_VALUE;

    FontHandle = CreateFile(argv[1],
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if (FontHandle == INVALID_HANDLE_VALUE)
    {
        printf("CreateFile(%s) failed with status %d!\n", argv[1], GetLastError()); return;
    }

    //
    // step 1 - opens the font scaler
    //

    Code = fs_OpenFonts(&client.input, &client.output);
    assert(Code == NO_ERROR);

    //
    // step 2 - initializes the font scaler
    //

    AllocateClientData(0, 2);

    Code = fs_Initialize(&client.input, &client.output);
    assert(Code == NO_ERROR);

    //
    // step 3 - specifies the sfnt data structure
    //

    client.input.clientID = (int32) FontHandle;
    client.input.param.newsfnt.platformID = PLATFORM_ID;
    client.input.param.newsfnt.specificID = ENCODING_ID;
    client.input.GetSfntFragmentPtr = (GetSFNTFunc) CallbackGetSfntFragment;
    client.input.ReleaseSfntFrag = (ReleaseSFNTFunc) CallbackReleaseSfntFragment;
    client.input.sfntDirectory = NULL;

    Code = fs_NewSfnt(&client.input, &client.output);
    assert(Code == NO_ERROR);

    //
    // step 4 - specifies the point size, the transformation matrix, the pixel diameter, and the device resolution
    //

    AllocateClientData(3, 4);

    //
    // 3 * 3 transform matrix
    //
    //  | 1  0  0 |         | 2  0  0 |         | 1  0  0 |         |   1    0    0 |
    //  | 0  1  0 |         | 0  1  0 |         | 0  2  0 |         | tan45  1    0 |
    //  | 0  0  1 |         | 0  0  1 |         | 0  0  1 |         |   0    0    1 |
    //     normal          twice as wide       twice as tall     rotated by tan45 degree
    //

    client.matrix.transform[0][0] = ONEFIX;
    client.matrix.transform[0][1] = 0;
    client.matrix.transform[0][2] = 0;

    client.matrix.transform[1][0] = 0;
    client.matrix.transform[1][1] = ONEFIX;
    client.matrix.transform[1][2] = 0;

    client.matrix.transform[2][0] = 0;
    client.matrix.transform[2][1] = 0;
    client.matrix.transform[2][2] = ONEFIX;

    client.input.param.newtrans.pointSize = (Fixed) (POINT_SIZE) << 16;
    client.input.param.newtrans.xResolution = 10;
    client.input.param.newtrans.yResolution = 10;
    client.input.param.newtrans.pixelDiameter = FIXEDSQRT2;
    client.input.param.newtrans.transformMatrix = &client.matrix;
#ifdef CALLBACK_TRACE
    client.input.param.newtrans.traceFunc = CallbackTracePreProgram;
#else
    client.input.param.newtrans.traceFunc = NULL;
#endif

    Code = fs_NewTransformation(&client.input, &client.output);
    assert(Code == NO_ERROR);

    //
    // step 5 - computes the glyph index from the character code
    //

    client.input.param.newglyph.glyphIndex = 0;
    client.input.param.newglyph.characterCode = CHAR_CODE;

    Code = fs_NewGlyph(&client.input, &client.output);
    assert(Code == NO_ERROR);

    //
    // step 6 - converts the glyph description into an outline with / without executing the instructions
    //

#ifdef CALLBACK_TRACE
    client.input.param.gridfit.traceFunc = CallbackTraceExecuteGlyph;
#else
    client.input.param.gridfit.traceFunc = NULL;
#endif
    client.input.param.gridfit.styleFunc = NULL;

#ifdef GRID_FIT_FLAG
    Code = fs_ContourGridFit(&client.input, &client.output);
#else
    Code = fs_ContourNoGridFit(&client.input, &client.output);
#endif
    assert(Code == NO_ERROR);

    //
    // step 7 - determines the amount of memory to create a bitmap of the glyph
    //

    Code = fs_FindBitMapSize(&client.input, &client.output);
    assert(Code == NO_ERROR);

    //
    // step 8 - converts the outline into a bitmap
    //

    AllocateClientData(5, 7);

    client.input.param.scan.bottomClip = client.output.bitMapInfo.bounds.top;
    client.input.param.scan.topClip = client.output.bitMapInfo.bounds.bottom;
    client.input.param.scan.outlineCache = NULL;

    Code = fs_ContourScan(&client.input, &client.output);
    assert(Code == NO_ERROR);

    //
    // dump bitmap info
    //

    Left = ROUND_FIXPT(client.output.metricInfo.devLeftSideBearing.x);
    AdvanceWidth = ROUND_FIXPT(client.output.metricInfo.devAdvanceWidth.x);
    cx = client.output.bitMapInfo.bounds.right - client.output.bitMapInfo.bounds.left;

    DumpBitmap(client.output.bitMapInfo.baseAddr,
               Left,
               client.output.bitMapInfo.bounds.bottom,
               Left + cx,
               client.output.bitMapInfo.bounds.top,
               client.output.bitMapInfo.rowBytes,
               AdvanceWidth,
               stdout);

    //
    // step 9 - closes the font scaler
    //

    Code = fs_CloseFonts(&client.input, &client.output);
    assert(Code == NO_ERROR);

    CloseHandle(FontHandle);
}