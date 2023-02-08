#pragma once

typedef struct
{
    s32 left;
    s32 right;
    
    s32 storageSize;
    char *storage;
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
    
    GapBuffer gapBuffer;
    
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