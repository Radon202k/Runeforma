#pragma once

typedef struct
{
    s32 gapLeft;
    s32 gapRight;
    
    s32 arraySize;
    char *array;
} GapBuffer;

typedef struct Buffer
{
    struct Buffer *nextChainEntry;
    char bufferName[512];
    
    s32 point;
    s32 currentLine;
    s32 numChars;
    s32 numLines;
    
    // Mark *markList;
    
    GapBuffer contents;
    
    char fileName[512];
    FILETIME fileTime;
    bool isModified;
    
} Buffer;

typedef struct
{
    Buffer *bufferChain;
    Buffer *currentBuffer;
    
} World;

typedef struct
{
    Sprite font;
    Sprite white;
    Sprite naruto;
    bool showGap;
    
    World world;
    
} Editor;