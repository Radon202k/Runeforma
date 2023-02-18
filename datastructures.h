#pragma once

typedef struct
{
    s32 left;
    s32 right;
    
    s32 storageLength;
    wchar_t *storage;
} GapBuffer;

typedef struct Buffer
{
    struct Buffer *nextChainEntry;
    wchar_t bufferName[512];
    
    s32 firstLineCharP;
    s32 lastLineCharP;
    
    s32 point;
    s32 currentLine;
    s32 numChars;
    s32 numLines;
    
    s32 mark;
    
    // Mark *markList;
    
    GapBuffer gapBuffer;
    
    wchar_t fileName[512];
    FILETIME fileTime;
    bool isModified;
    
} Buffer;

typedef struct
{
    Buffer *bufferChain;
    Buffer *currentBuffer;
    
    Vector2 bucketSize;
    Vector2 origin;
    
} World;

typedef struct
{
    void *a;
    void *b;
    
    float t;
    float tVel;
    
    Vector2 c0;
    Vector2 c1;
    
    bool loop;
    bool backAndForward;
    bool finished;
    bool isPlaying;
} Animator;

typedef struct
{
    Sprite white;
    Sprite debugFont;
    bool showGap;
    
    World world;
    
    TruetypeFont font32;
    TruetypeFont font16;
    
    float scrollBarPoint;
    
    float contentHCache;
    bool contentHCached;
    
    Vector2 lastPointP;
    Animator pointPosAnimator;
    Animator originAnimator;
    
} Editor;