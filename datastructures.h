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
    
    s32 mark;
    
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
    Sprite white;
    Sprite debugFont;
    bool showGap;
    
    World world;
    
    TruetypeFont font32;
    TruetypeFont font16;
    
    Sprite glyphs32[94];
    Sprite glyphs16[94];
    
} Editor;