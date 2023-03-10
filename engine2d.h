/******************************************************************************
 This file implements a 2d engine that uses Windows and Direct3D 11 APIs.
 NOTE: This file development is currently IN PROGRESS.
******************************************************************************/


// COBJMACROS to be able to use Direct3D 11 API in C
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN

// Windows API headers
#include <windows.h>
#include <windowsx.h>

// Direct Sound 8
#include <mmeapi.h>
#include <dsound.h>

// Direct3D 11 API headers
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>

// C headers
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include <tchar.h>

// Short version of common types
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uint32_t bool;
#define false 0
#define true 1
#define global static
#define function static

// Array count
#define array_count(a) (sizeof(a) / sizeof((a)[0]))

// Math
inline float
lerpf(float a, float t, float b)
{
    float result = (1-t)*a+t*b;
    return result;
}

// Vector2 float type
typedef struct
{
    float x, y;
} Vector2;

// Vector2 initializer v2(1,2)
inline Vector2
v2(float x, float y)
{
    Vector2 result = { x, y };
    return result;
}

inline Vector2
v2f(s32 x, s32 y)
{
    Vector2 result = { (float)x, (float)y };
    return result;
}


inline Vector2 
v2_bezier(float x1, float y1, float x2, float y2, float t)
{
    Vector2 p0 = v2(0,0);
    Vector2 p1 = v2(x1,y1);
    Vector2 p2 = v2(x2,y2);
    Vector2 p3 = v2(1,1);
    
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    Vector2 p =
    {
        uuu * p0.x + 3.0f * uu * t * p1.x + 3.0f * u * tt * p2.x + ttt * p3.x,
        uuu * p0.y + 3.0f * uu * t * p1.y + 3.0f * u * tt * p2.y + ttt * p3.y
    };
    return p;
}

// Vector2 Addition
inline Vector2
v2_add(Vector2 a, Vector2 b)
{
    Vector2 result = 
    {
        a.x + b.x,
        a.y + b.y
    };
    return result;
}

// Vector2 Subtraction
inline Vector2
v2_sub(Vector2 a, Vector2 b)
{
    Vector2 result = 
    {
        a.x - b.x,
        a.y - b.y
    };
    return result;
}

// Vector2 Multiplication by scalar
inline Vector2
v2_mul(float k, Vector2 a)
{
    Vector2 result = 
    {
        a.x*k,
        a.y*k
    };
    return result;
}

// Vector2 Inner/Dot product
inline float
v2_inner(Vector2 a, Vector2 b)
{
    float result = a.x*b.x + a.y*b.y;
    return result;
}

// Vector2 Length squared
inline float
v2_length2(Vector2 a)
{
    return v2_inner(a, a);
}

// Vector2 Length
inline float
v2_length(Vector2 a)
{
    return sqrtf(v2_length2(a));
}

inline Vector2
v2_lerp(Vector2 a, float t, Vector2 b)
{
    Vector2 result = 
        v2(lerpf(a.x, t, b.x),
           lerpf(a.y, t, b.y));
    return result;
}

// Vector2 Integer type
typedef struct
{
    s32 x, y;
} Vector2i;

// Vector3 float type
typedef struct
{
    float x, y, z;
} Vector3;

// Vector4 float type
typedef struct
{
    float x, y, z, w;
} Vector4;

// Color type [0,1]
typedef union
{
    float data[4];
    struct
    {
        float r, g, b, a;
    };
} Color;

// Color rgba initializer rgba(0,0.2f,1,1) [0,1]
inline Color
rgba(float r, float g, float b, float a)
{
    Color result = {r, g, b, a};
    return result;
}

inline Color
color_lerp(Color a, float t, Color b)
{
    Color result = 
        rgba(lerpf(a.r, t, b.r),
             lerpf(a.g, t, b.g),
             lerpf(a.b, t, b.b),
             lerpf(a.a, t, b.a));
    return result;
}

// Matrix 4x4 float type
typedef struct
{
    float data[4][4];
} Matrix4x4;

// Rect 2D float type
typedef struct
{
    Vector2 min;
    Vector2 max;
} Rect2;

// Rect 2D float initializer rect2(0,0,20,20) [pixels]
inline Rect2
rect2(float minX, float minY, float maxX, float maxY)
{
    Rect2 result =
    {
        {minX, minY},
        {maxX, maxY},
    };
    return result;
}

// Strings
function void 
string_copy(wchar_t *dest, u32 destSize,
            wchar_t *src)
{
    wcscpy_s(dest, destSize, src);
}

function void 
string_copy_size(wchar_t *dest, u32 destSize,
                 wchar_t *src, u32 copySize)
{
    wcsncpy_s(dest, destSize, src, copySize);
}

function void 
string_copy_size_ascii(char *dest, u32 destSize,
                       char *src, u32 copySize)
{
    strncpy_s(dest, destSize, src, copySize);
}

function bool
string_equal(wchar_t *a, wchar_t *b)
{
    return (wcscmp(a, b) == 0);
}

function s32
string_length(wchar_t *s)
{
    return (u32)wcslen(s);
}

function void
string_append(wchar_t *dest, u32 destSize, wchar_t *b)
{
    wcscat_s(dest, destSize, b);
}

#define alloc_size(size) _safe_alloc(size)
#define alloc_type(type) (type *)_safe_alloc(sizeof(type))
#define alloc_array(count, type) (type *)_safe_alloc((count)*sizeof(type))

function void *_safe_alloc(u32 size)
{
    // Allocate it
    void *result = malloc(size);
    
    // Clear it to zero
    memset(result, 0, size);
    
    // Return the pointer
    return result;
}

#define clear_size(ptr, size) _safe_clear(ptr, size)
#define clear_type(ptr, type) _safe_clear(ptr, sizeof(type))
#define clear_array(ptr, count, type) _safe_clear(ptr, (count)*sizeof(type))

function void _safe_clear(void *ptr, u32 size)
{
    memset(ptr, 0, size);
}

#define copy_size(dst, src, size) _safe_copy(dst, src, size)
#define copy_struct(dst, src, type) _safe_copy(dst, src, sizeof(type))
#define copy_array(dst, src, count, type) _safe_copy(dst, src, (count)*sizeof(type))

function void _safe_copy(void *dst, void *src, u32 size)
{
    memcpy(dst, src, size);
}

// File
typedef struct 
{
    wchar_t name[512];
    bool exists;
    u32 size;
    u8 *bytes;
} File;

// Read the full contents of a file at once
function File
read_file(wchar_t *filePath)
{
    File result = {0};
    
    FILE *file = 0;
    _wfopen_s(&file, filePath, L"rb, ccs=UTF-16LE");
    if (file != 0)
    {
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        rewind(file);
        
        result.bytes = alloc_size(fileSize+1);
        if (result.bytes)
        {
            size_t readCount = fread((wchar_t *)result.bytes, sizeof(wchar_t),
                                     fileSize/sizeof(wchar_t), file);
            if (readCount == fileSize/sizeof(wchar_t))
            {
                result.exists = true;
                result.size = fileSize;
            }
        }
    }
    
    return result;
}

// Write the entire contents of a file at once
function bool
write_file(wchar_t *filePath, wchar_t *data, u32 size)
{
    bool result = false;
    
    HANDLE handle = CreateFileW(filePath,
                                GENERIC_WRITE,
                                FILE_SHARE_WRITE,
                                0,
                                CREATE_ALWAYS,
                                0,
                                0);
    
    DWORD bytesWritten = 0;
    if (!WriteFile(handle, data, size,
                   &bytesWritten, 0))
    {
        CloseHandle(handle);
        return result;
    }
    
    assert(size == bytesWritten);
    result = true;
    CloseHandle(handle);
    
    return result;
}

// Sorting acceleration data structure
typedef struct
{
    u32 index;
    float comparisonValue;
} SortIndex;

// Quick sort
function int
quick_sort_indices_partition(SortIndex a[], int beg, int end)
{
    SortIndex temp;
    
    // This is the classic quick sort partition implementation
    int left, right, loc, flag;
    loc = beg;
    left = beg;
    right = end;
    flag = 0;
    while (flag != 1)
    {
        while ((a[loc].comparisonValue <= a[right].comparisonValue) &&
               (loc != right))
        {
            right--;
        }
        
        if (loc == right)
        {
            flag = 1;
        }
        else if (a[loc].comparisonValue > a[right].comparisonValue)
        {
            temp = a[loc];
            a[loc] = a[right];
            a[right] = temp;
            loc = right;
        }
        
        if (flag != 1)
        {
            while ((a[loc].comparisonValue >= a[left].comparisonValue) && 
                   (loc != left))
            {
                left++;
            }
            
            if (loc == left)
            {
                flag = 1;
            }
            else if (a[loc].comparisonValue < a[left].comparisonValue)
            {
                temp = a[loc];
                a[loc] = a[left];
                a[left] = temp;
                loc = left;
            }
        }
    }
    
    return loc;
}

// Quick sort
function void
quick_sort_indices(SortIndex a[], int beg, int end)
{
    // This is the classic quick sort algorithm implementation
    
    int loc;
    if (beg < end)
    {
        loc = quick_sort_indices_partition(a, beg, end);
        quick_sort_indices(a, beg, loc-1);
        quick_sort_indices(a, loc+1, end);
    }
}

// Vertex 3D for usage with Direct3D vertex buffer
typedef struct
{
    Vector3 pos;
    Vector2 uv;
    Color color;
} Vertex3D;

// Line Vertex 3D for usage with Direct3D vertex buffer
typedef struct
{
    Vector3 pos;
    Color color;
} LineVertex3D;

// Constant buffre for usage with vertex buffer (WVP matrix)
typedef struct 
{
    Matrix4x4 WVP;
} VertexConstantBuffer;

// Structure to hold a command to render a sprite
typedef struct
{
    Vector2 pos;
    Vector2 size;
    Rect2 uv;
    Color col;
    float layer;
} SpriteCommand;

// Structure to hold a command to render a line
typedef struct
{
    Vector2 a;
    Vector2 b;
    Color col;
    float layer;
} LineCommand;

// Structure to hold many commands to render sprites
typedef struct
{
    SpriteCommand *spriteCommands;
    u32 spriteCommandsPushedCount;
    
} SpriteGroup;

// Structure to hold many commands to render lines
typedef struct
{
    LineCommand *lineCommands;
    u32 lineCommandsPushedCount;
    
} LineGroup;

// Structure to hold sprite information
typedef struct
{
    bool exists;
    char name[512];
    Vector2 size;
    Rect2 uv;
    Vector2 align;
} Sprite;

// Structure to help with sprite texture atlas construction
typedef struct
{
    u32 size;
    u32 atX;
    u32 atY;
    u32 bottom;
    u8 *bytes;
} SpriteAtlas;

// Structure to hold a key information (keyboard or mouse buttons)
typedef struct 
{
    bool down;
    bool pressed;
    bool released;
} Key;

// Structure to hold the many keys we support (the keyboard)
typedef union
{
    Key all[14];
    struct
    {
        Key up;
        Key left;
        Key down;
        Key right;
        Key backspace;
        Key alt;
        Key f1;
        Key s;
        Key d;
        Key c;
        Key v;
        Key t;
        Key control;
        Key space;
    };
} Keys;

// Structure to hold mouse information (position and buttons)
typedef struct
{
    Vector2 pos;
    Key left;
    Key right;
    float wheel;
    
    bool dragging;
    Vector2 dragLastP;
    void *draggingAddress;
    
} Mouse;

// Structure to hold all data the engine uses
typedef struct
{
    bool running;
    HDC deviceContext;
    HWND window;
    Vector2 backBufferSize;
    ID3D11Device *device;
    ID3D11DeviceContext *context;
    IDXGISwapChain *swapChain;
    ID3D11Debug *debugger;
    ID3D11Texture2D *bbTexture; // Bback buffer texture
    ID3D11Texture2D *dsbTexture; // Depth stencil buffer texture
    ID3D11RenderTargetView *rtv;
    ID3D11DepthStencilView *dsv;
    ID3D11DepthStencilState *dss;
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    D3D11_TEXTURE2D_DESC dsbTexture2DDesc;
    ID3D11BlendState *blendState;
	ID3D11RasterizerState *rasterizerState;
    ID3D11SamplerState *samplerState;
    // Sprites
    ID3D11VertexShader *spritesVertexShader;
    ID3D11PixelShader *spritesPixelShader;
    ID3D11InputLayout *spritesInputLayout;
    ID3D11Buffer *spritesVertexBuffer;
    SpriteGroup spriteGroups[16];
    // Lines
    ID3D11VertexShader *linesVertexShader;
    ID3D11PixelShader *linesPixelShader;
    ID3D11InputLayout *linesInputLayout;
    ID3D11Buffer *linesVertexBuffer;
    LineGroup lineGroup;
    //
    ID3D11Buffer *vertexCBuffer; // Constant buffer for vertex shader
    VertexConstantBuffer vertexCBufferData; // Data for above
    ID3D11Texture2D *atlasTexture;
    ID3D11ShaderResourceView *atlasTexSRV;
    SpriteAtlas spriteAtlas;
    Keys key;
    Mouse mouse;
    bool inputCharEntered;
    wchar_t inputChar;
    
    WAVEFORMATEX waveFormat;
    IDirectSound8 *dsound;
    IDirectSoundBuffer *audioBuffers[32];
    u32 audioBufferIndex;
    
    u64 perfFrequency;
    float dt;
} Engine;

global Engine engine;

function void init();
function void update();
function void resized();

/******************************************************************************
*** [CONFIGURATION]
******************************************************************************/

#ifndef BACKBUFFER_WIDTH
#define BACKBUFFER_WIDTH 800
#endif

#ifndef BACKBUFFER_HEIGHT
#define BACKBUFFER_HEIGHT 600
#endif

#ifndef WINDOW_TITLE
#define WINDOW_TITLE L"Engine 2D"
#endif

#ifndef TOP_DOWN
#define TOP_DOWN false
#endif

#ifndef SPRITE_ATLAS_SIZE
#define SPRITE_ATLAS_SIZE 4096
#endif

#ifndef CLEAR_COLOR
#define CLEAR_COLOR rgba(.1f,.1f,.1f,1)
#endif

#ifndef MAX_ALLOWED_RENDERED_SPRITES
#define MAX_ALLOWED_RENDERED_SPRITES 50000
#endif

#ifndef MAX_ALLOWED_RENDERED_LINES
#define MAX_ALLOWED_RENDERED_LINES 10000
#endif

/*****************************************************************************
*** [API]
******************************************************************************/

typedef struct HashTableNode
{
    float id;
    void *data;
    struct HashTableNode *next;
} HashTableNode;

typedef struct
{
    u32 storageLength;
    HashTableNode **storage;
} HashTable;

function HashTable
hash_table_create(s32 storageLength)
{
    HashTable result = {0};
    result.storageLength = storageLength;
    result.storage = alloc_size(storageLength*sizeof(HashTableNode*));
    return result;
}

function HashTableNode *
hash_table_get(HashTable *table, u32 hashIndex, float id)
{
    assert(hashIndex < table->storageLength);
    
    // Look at hash table position
    HashTableNode *node = table->storage[hashIndex];
    
    // While there are nodes in the collision chain
    while (node)
    {
        // If the ids are the same
        if (node->id == id)
        {
            // Found the item
            return node;
        }
        
        // Advance in the collision chain
        node = node->next;
    }
    
    // Didn't find the node
    return 0;
}

function bool
hash_table_set(HashTable *table, u32 hashIndex, void *data, float id)
{
    assert(hashIndex < table->storageLength);
    
    HashTableNode *found = hash_table_get(table, hashIndex, id);
    if (!found)
    {
        // Allocate node and set data
        HashTableNode *newNode = alloc_type(HashTableNode);
        newNode->id = id;
        newNode->data = data;
        
        // Put the new node at the head of the chain
        newNode->next = table->storage[hashIndex];
        table->storage[hashIndex] = newNode;
        
        return true;
    }
    
    return false;
}

function HashTableNode *
hash_table_remove(HashTable *table, u32 hashIndex, float id)
{
    assert(hashIndex < table->storageLength);
    
    // Look at hash table position
    HashTableNode *node = table->storage[hashIndex];
    
    // While there are nodes in the collision chain
    HashTableNode *lastNode = 0;
    while (node)
    {
        // If the ids are the same
        if (node->id == id)
        {
            // If the node is not the head
            if (node != table->storage[hashIndex])
            {
                // Remove from chain
                lastNode->next = node->next;
                
                node->next = 0;
            }
            // If the node is the head
            else
            {
                // Set the head of the chain as null
                table->storage[hashIndex] = 0;
            }
            
            // Found the item
            return node;
        }
        
        lastNode = node;
        
        // Advance in the collision chain
        node = node->next;
    }
    
    return 0;
}

// Helper function to copy text to the clipboard
function void 
copy_text_to_clipboard(wchar_t *text)
{
    HGLOBAL globalMemory = GlobalAlloc(GMEM_MOVEABLE, (string_length(text)+1) * sizeof(wchar_t));
    
    wchar_t *data = (wchar_t *)GlobalLock(globalMemory);
    
    u32 copiedLength = string_length(text)+1;
    
    string_copy(data, copiedLength, text);
    
    assert(copiedLength < 1000);
    
    GlobalUnlock(globalMemory);
    
    if (OpenClipboard(NULL))
    {
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, globalMemory);
        CloseClipboard();
    }
    
    GlobalFree(globalMemory);
}

// Helper function to paste text from the clipboard
function s32 
paste_text_from_clipboard(wchar_t *text, int maxLength)
{
    s32 pastedLength = 0;
    
    if (OpenClipboard(NULL))
    {
        HGLOBAL globalMemory = GetClipboardData(CF_UNICODETEXT);
        if (globalMemory != NULL)
        {
            wchar_t *data = (wchar_t *)GlobalLock(globalMemory);
            pastedLength = string_length(data);
            
            assert(pastedLength < 1000);
            
            string_copy_size(text, maxLength*sizeof(wchar_t), data, pastedLength);
            GlobalUnlock(globalMemory);
        }
        
        CloseClipboard();
    }
    
    return pastedLength;
}


function bool
mouse_dragged_handle(Vector2 p, float maxDist, void *address,
                     bool *hover, Vector2 *delta)
{
    bool dragged = false;
    
    if (v2_length2(v2_sub(p,engine.mouse.pos)) < maxDist*maxDist)
    {
        *hover = true;
        
        if (engine.mouse.left.down && !engine.mouse.dragging)
        {
            engine.mouse.dragging = true;
            engine.mouse.dragLastP = engine.mouse.pos;
            engine.mouse.draggingAddress = address;
        }
    }
    else
    {
        *hover = false;
    }
    
    if (engine.mouse.dragging && engine.mouse.draggingAddress == address)
    {
        Vector2 deltaP = v2_sub(engine.mouse.pos,engine.mouse.dragLastP);
        engine.mouse.dragLastP = engine.mouse.pos;
        *delta = deltaP;
        dragged = true;
    }
    
    return dragged;
}


function void
get_exe_path(wchar_t *path, DWORD size)
{
    GetModuleFileNameW(NULL, path, size);
}

function void
get_exe_path_ascii(char *path, DWORD size)
{
    GetModuleFileNameA(NULL, path, size);
}

function void
build_absolute_path(wchar_t *dest, u32 destSize, 
                    wchar_t *fileName)
{
    wchar_t exePath[MAX_PATH];
    get_exe_path(exePath, MAX_PATH);
    
    /** We want to copy the exe path until the last slash
    *** c:/my/path/to/the/app/main.exe
    ***                      ^                         */
    
    // Find the last slash
    wchar_t *slashPos = 0;
    wchar_t *at = exePath;
    while (*at)
    {
        if (*at == '\\' || *at == '/')
        {
            slashPos = at;
        }
        
        at++;
    }
    
    // After going through all the chars, slashPos poitns to the last slash 
    wchar_t *lastSlash = slashPos;
    
    // Extract the directory from the exe path
    wchar_t dir[260];
    
    // The address of lashSlash minus the address of exePath is equal to
    // the amount of chars we must copy. That is because arrays are placed
    // in memory one byte after the other.
    string_copy_size(dir, 260, exePath, (u32)(lastSlash - exePath));
    
    // Combine the directory and file name to form the absolute path
    _snwprintf_s(dest, destSize, _TRUNCATE, L"%s\\%s", dir, fileName);
}

function void
build_absolute_path_ascii(char *dest, u32 destSize, 
                          char *fileName)
{
    char exePath[MAX_PATH];
    get_exe_path_ascii(exePath, MAX_PATH);
    
    /** We want to copy the exe path until the last slash
    *** c:/my/path/to/the/app/main.exe
    ***                      ^                         */
    
    // Find the last slash
    char *slashPos = 0;
    char *at = exePath;
    while (*at)
    {
        if (*at == '\\' || *at == '/')
        {
            slashPos = at;
        }
        
        at++;
    }
    
    // After going through all the chars, slashPos poitns to the last slash 
    char *lastSlash = slashPos;
    
    // Extract the directory from the exe path
    char dir[260];
    
    // The address of lashSlash minus the address of exePath is equal to
    // the amount of chars we must copy. That is because arrays are placed
    // in memory one byte after the other.
    string_copy_size_ascii(dir, 260, exePath, (u32)(lastSlash - exePath));
    
    // Combine the directory and file name to form the absolute path
    _snprintf_s(dest, destSize, _TRUNCATE, "%s\\%s", dir, fileName);
}


function u8 *
load_png(wchar_t *filePath, u32 *width, u32 *height)
{
    u8 *result = 0;
    
    int asciiSize = WideCharToMultiByte(CP_ACP, 0, filePath, -1, NULL, 0, NULL, NULL);
    char *asciiFilename = (char *)alloc_size(asciiSize);
    WideCharToMultiByte(CP_ACP, 0, filePath, -1, asciiFilename, asciiSize, NULL, NULL);
    
    // Load the image using stbi_load function from stb_image.h library
    int w, h, nrChannels;
    
    size_t byteSize = 0;
    unsigned char *data = stbi_load(asciiFilename, &w, &h, &nrChannels, 0);
    
    // If the load function returned valid data
	if (data)
    {
        // Set the with and height return variables with the correct dimensions
        *width = w;
        *height = h;
        
        // The image data size is width * height * 4
		int dataSize = (*width) * (*height) * nrChannels;
        
        // Allocate space for the copied data
		result = (u8 *)malloc(dataSize);
		
        // Get u8 pointers to the rows of dest and source
        // We are copying
        //   from data loaded by stbi_load
        //   to result allocated by us to hold the data
        u8 *rowDst = result;
        u8 *rowSrc = data;
        
        // For each row of the source image [0,height]
        for (u32 j = 0; j < *height; j++)
        {
            // Get u32 pointers to the pixels of dest and source
            u32 *pxDst = (u32 *)rowDst;
            u32 *pxSrc = (u32 *)rowSrc;
            
            // For each pixel in the row [0, width]
            for (u32 i = 0; i < *width; i++)
            {
                // Extract the r g b a components of the source pixel
                u32 r = (*pxSrc >> 16) & 0xFF;
                u32 g = (*pxSrc >> 8) & 0xFF;
                u32 b = (*pxSrc >> 0) & 0xFF;
                u32 a = (*pxSrc >> 24) & 0xFF;
                
                // Convert the alpha from [0,255] int -> [0,1] float
                float realA = (float)a / 255.0f;
                
                // Premultiply the alpha of the r g b components
                /*
                r = (u32)(r * realA + .5f);
                g = (u32)(g * realA + .5f);
                b = (u32)(b * realA + .5f);
                */
                // Write the destination pixel with the premultiplied values
                // Note that we use the alpha without premultiplying it
                // since it already has the correct value
                *pxDst = ((r << 16) | 
                          (g << 8) |
                          (b << 0) |
                          (a << 24));
                
                // Advance the pixel pointers for dest and source
                pxDst++;
                pxSrc++;
            }
            
            // Advance the row pointers for dest and source
            rowDst += *width*4;
            rowSrc += *width*4;
        }
        
        // Free the data allocated from stbi_load function
        stbi_image_free(data);
	}
	else
	{
        // If the data returned was invalid, the image was not found
        // or it couldn't be read properly, so we mark the width and
        // height return values as zero to indicate that it failed
        *width = 0;
        *height = 0;
	}
    
    // Returns either 0 for fail or the pointer to the loaded image data
	return result;
}

// Performs a 1 to 1 blit of image data pointed by src to dest
function void
blit_simple_unchecked(u8 *dest, u32 destSize, u8 *src,
                      u32 atX, u32 atY,
                      u32 sourceWidth, u32 sourceHeight)
{
    u8 *rowSrc = src;
    u8 *rowDest = dest + atY * destSize * 4 + atX * 4;
    
    for (u32 y = 0; y < sourceHeight; ++y)
    {
        u32 *pixelSrc = (u32 *)rowSrc;
        u32 *pixelDest = (u32 *)rowDest;
        
        for (u32 x = 0; x < sourceWidth; ++x)
        {
            *pixelDest++ = *pixelSrc++;
        }
        
        rowSrc += 4 * sourceWidth;
        rowDest += 4 * engine.spriteAtlas.size;
    }
}

// Creates a sprite in the sprite atlas
function Sprite
sprite_create(u32 spriteWidth, u32 spriteHeight, u8 *bytes)
{
    Sprite result = {0};
    
    s32 margin = 1;
    
    SpriteAtlas *atlas = &engine.spriteAtlas;
    
    // If the sprite doesn't fit in current row, need to advance to next row
    if ((atlas->atX + spriteWidth + margin) > atlas->size)
    {
        atlas->atX = 0;
        atlas->atY = atlas->bottom;
    }
    
    // If the sprite is taller than previous ones in row, grow the bottom
    if (margin + atlas->bottom < (atlas->atY + spriteHeight))
    {
        atlas->bottom = atlas->atY + spriteHeight + margin;
    }
    
    assert(atlas->bottom <= atlas->size);
    
    // Render the png bytes to the atlas bytes
    u8 *dest = engine.spriteAtlas.bytes;
    u8 *src = bytes;
    blit_simple_unchecked(dest, engine.spriteAtlas.size, src,
                          atlas->atX, atlas->atY,
                          spriteWidth, spriteHeight);
    
    // Calculate uv based on position
    float minU = (float)atlas->atX / (float)atlas->size;
    float minV = (float)atlas->atY / (float)atlas->size;
    float maxU = ((float)atlas->atX + spriteWidth) / (float)atlas->size;
    float maxV = ((float)atlas->atY + spriteHeight) / (float)atlas->size;
    
    // Advance atX
    atlas->atX += spriteWidth + margin;
    
    result.size = v2((float)spriteWidth, (float)spriteHeight);
    result.uv.min = v2(minU, minV);
    result.uv.max = v2(maxU, maxV);
    
    result.exists = true;
    
    return result;
}

// Create sprite from file
function Sprite
sprite_create_from_file(wchar_t *filePath)
{
    Sprite result = {0};
    
    u32 spriteWidth = 0;
    u32 spriteHeight = 0;
    
    u8 *bytes = load_png(filePath, &spriteWidth, &spriteHeight);
    
    if (bytes)
    {
        result = sprite_create(spriteWidth, spriteHeight, bytes);
    }
    
    return result;
}

// Get the current bytes of the texture atlas
function u8 *
atlas_get_bytes()
{
    return engine.spriteAtlas.bytes;
}

// Update the texture atlas resource in the GPU based on updatedData
function void
atlas_update(u8 *updatedData)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    ID3D11DeviceContext_Map(engine.context,
                            (ID3D11Resource *)engine.atlasTexture,
                            0,
                            D3D11_MAP_WRITE_DISCARD,
                            0,
                            &mappedResource);
    
    memcpy(mappedResource.pData, updatedData, engine.spriteAtlas.size * engine.spriteAtlas.size * 4);
    
    ID3D11DeviceContext_Unmap(engine.context,
                              (ID3D11Resource *)engine.atlasTexture,
                              0);
}

function float
safe_divide(float a, float b)
{
    assert(b != 0);
    float result = 0;
    
    if (b != 0)
    {
        result = a / b;
    }
    
    return result;
}

function float
map_range_to_range(float a1, float a2, float s,
                   float b1, float b2)
{
    float t = (b1+(s-a1)*safe_divide((b2-b1),(a2-a1)));
    return t;
}

// Font

typedef struct
{
    s32 minX;
    s32 minY;
    s32 maxX;
    s32 maxY;
} Rect;

typedef struct
{
    HFONT handle;
    HBITMAP bitmap;
    TEXTMETRICW metrics;
    VOID *bytes;
    float lineAdvance;
    float charWidth;
    float maxDescent;
    
    HashTable glyphsTable;
    
} TruetypeFont;

#define FONT_GLYPH_MAKER_DIM 256

#define RGBA(r,g,b,a) (u32)((a << 24) | (b << 16) | (g << 8) | (r << 0))

#define red32 RGBA(237,28,36,255)
#define yellow32 RGBA(236,227,0,255)
#define orange32 RGBA(255,127,39,255)
#define cyan32 RGBA(0,198,191,255)
#define lime32 RGBA(7,198,0,255)
#define purple32 RGBA(190,0,198,255)
#define blue32 RGBA(0,0,255,255)

function u32
get_hash_for_unicode(HashTable *table, u32 unicode)
{
    assert(table->storageLength % 2 == 0);
    u32 hash = unicode & (table->storageLength-1);
    return hash;
}

function Sprite *
font_create_glyph(TruetypeFont *font, wchar_t *c, 
                  Rect *tightBounds, s32 *tightDescent)
{
    Sprite *result = alloc_type(Sprite);
    
    // Get the char width
    SIZE size;
    GetTextExtentPoint32W(engine.deviceContext, c, 1, &size);
    
    s32 width = size.cx;
    s32 height = size.cy;
    
    s32 charSize = (s32)wcslen(c);
    
    s32 prestepX = 30;
    s32 prestepY = 30;
    
    // Write the char to the bitmap
    TextOutW(engine.deviceContext, prestepX, prestepY, c, charSize);
    
    s32 minX = 0;
    s32 minY = 0;
    s32 maxX = width;
    s32 maxY = height;
    
    s32 tightMinX = 1000000;
    s32 tightMinY = 1000000;
    s32 tightMaxX =-1000000;
    s32 tightMaxY =-1000000;
    
#if 1
    // Find tight bounding box
    bool foundTightBox = false;
    
    // Walk from last row to last row minus height
    for (s32 j = 0;
         j < FONT_GLYPH_MAKER_DIM;
         ++j)
    {
        for (s32 i = 0; 
             i < FONT_GLYPH_MAKER_DIM;
             ++i)
        {
            u32 *pixel = (u32 *)((u8 *)font->bytes + 
                                 j*(FONT_GLYPH_MAKER_DIM*4) + 
                                 i*4);
            if (*pixel != 0)
            {
                foundTightBox = true;
                if (tightMinX > i) tightMinX = i;
                if (tightMinY > j) tightMinY = j;
                if (tightMaxX < i) tightMaxX = i;
                if (tightMaxY < j) tightMaxY = j;
            }
        }
    }
    
    s32 alignX = 0;
    s32 alignY = 0;
    
    // Recalculate tight width and height
    s32 tightWidth = 0;
    s32 tightHeight = 0;
    if (foundTightBox)
    {
        *tightDescent = font->metrics.tmDescent-(font->metrics.tmHeight-tightMaxY)-prestepY;
        
        // Pixels to move char to the right
        alignX = prestepX-tightMinX;
        alignY = *tightDescent;
        
        // Expand tight bounds to include 1 px boder
        --tightMinX;
        --tightMinY;
        ++tightMaxX;
        ++tightMaxY;
        
        tightWidth = tightMaxX-tightMinX+1;
        tightHeight = tightMaxY-tightMinY+1;
        
        
#if 1
        
        tightBounds->minX = tightMinX;
        tightBounds->minY = tightMinY;
        tightBounds->maxX = tightMaxX;
        tightBounds->maxY = tightMaxY;
        
        minX = tightMinX;
        minY = tightMinY;
        maxX = tightMaxX;
        maxY = tightMaxY;
        width = tightWidth;
        height = tightHeight;
        prestepX = 0;
        prestepY = 0;
#endif
    }
    
#endif
    
    // Allocate memory for the bytes
    u8 *bytes = (u8 *)malloc(width*height*4);
    
    // Clear to zero
    memset(bytes, 0, width*height*4);
    
    // For every row
    u8 *destRow = bytes;
    
    // Prestep the source pointer
    u8 *srcRow = ((u8 *)font->bytes + 
                  minY*FONT_GLYPH_MAKER_DIM*4 +
                  (minX+prestepX)*4);
    
    for (s32 y = minY; y < maxY; ++y)
    {
        // For every col
        u32 *pixel = (u32 *)srcRow;
        u32 *dest = (u32 *)destRow;
        
        for (s32 x = minX; x < maxX; ++x)
        {
            u32 color = *pixel++;
            u32 a = ((color >> 16) & 0xff);
            
            // Write it to all components
            *dest++ =  RGBA(255,255,255,a);
        }
        
        // Advance the row
        destRow += 4*width;
        srcRow += 4*FONT_GLYPH_MAKER_DIM;
    }
    
    // Create the sprite
    *result = sprite_create(width, height, bytes);
    
    result->align.x = (float)alignX;
    result->align.y = (float)alignY;
    
    // Free the allocated memory
    free(bytes);
    
    return result;
}

function s32
font_pixel_height(s32 pointHeight)
{
    s32 result = MulDiv(pointHeight, 
                        GetDeviceCaps(engine.deviceContext, LOGPIXELSY), 
                        GetDeviceCaps(engine.deviceContext, LOGPIXELSX));
    return result;
}

function TruetypeFont
font_create_from_file(wchar_t *fileName, wchar_t *fontName,
                      int fontHeightPoints)
{
    TruetypeFont result = {0};
    
    // Create the font
    int filesAdded = AddFontResourceW(fileName);
    assert(filesAdded == 1);
    
    result.handle = CreateFontW(font_pixel_height(fontHeightPoints), 
                                0, 0, 0, 
                                FW_NORMAL, 
                                false, // Italic
                                false, // Underline
                                false, // Strikeout
                                DEFAULT_CHARSET,
                                OUT_OUTLINE_PRECIS,
                                CLIP_DEFAULT_PRECIS,
                                CLEARTYPE_QUALITY,
                                DEFAULT_PITCH,
                                fontName);
    
    assert(result.handle &&
           result.handle != INVALID_HANDLE_VALUE);
    
    BITMAPINFO info = {0};
    info.bmiHeader.biSize = sizeof(info.bmiHeader);
    info.bmiHeader.biWidth = FONT_GLYPH_MAKER_DIM;
    info.bmiHeader.biHeight = -FONT_GLYPH_MAKER_DIM;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    
    // Create font bitmap
    result.bitmap = CreateDIBSection(engine.deviceContext,
                                     &info, DIB_RGB_COLORS, 
                                     &result.bytes, 0, 0);
    
    // Clear to zero
    memset(result.bytes, 0, 
           FONT_GLYPH_MAKER_DIM*FONT_GLYPH_MAKER_DIM*4);
    
    
    // Select font bitmap
    SelectObject(engine.deviceContext, result.bitmap);
    
    // Select font
    SelectObject(engine.deviceContext, result.handle);
    
    // Get font text metrics
    TEXTMETRICW metrics = {0};
    GetTextMetricsW(engine.deviceContext, &metrics);
    
    result.metrics = metrics;
    
    // Set the background color to back
    SetBkColor(engine.deviceContext, RGB(0,0,0));
    
    // Set the text color to white
    SetTextColor(engine.deviceContext, RGB(255,255,255));
    
    // Get the line advance
    result.lineAdvance = (float)metrics.tmHeight - metrics.tmInternalLeading;
    result.charWidth = (float)metrics.tmAveCharWidth;
    
    // Create the glyphs hash table
    result.glyphsTable = hash_table_create(512);
    
    s32 maxGlyphW = -10000;
    s32 maxGlyphH = -10000;
    s32 maxDescent = -10000;
    
    // Create the basic Latin glyphs
    for (s32 i = 32; i <= 255; i++)
    {
        if (i == 253)
        {
            int y = 3;
        }
        
        wchar_t character[2] = { (wchar_t)i, 0 };
        
        // Allocate the glyph with dynamic memory
        Rect tightBounds = {0};
        s32 tightDescent = 0;
        Sprite *glyph = font_create_glyph(&result, character, 
                                          &tightBounds, &tightDescent);
        
        if (tightBounds.maxX != 0 && tightBounds.maxY != 0)
        {
            s32 tightWidth = tightBounds.maxX-tightBounds.minX;
            s32 tightHeight = tightBounds.maxY-tightBounds.minY;
            
            if (maxGlyphW < tightWidth) maxGlyphW = tightWidth;
            if (maxGlyphH < tightHeight) maxGlyphH = tightHeight;
            if (maxDescent < tightDescent) maxDescent = tightDescent;
        }
        
        // Set the glyph into the glyph hash table
        hash_table_set(&result.glyphsTable, i, glyph, (float)i);
    }
    
    result.lineAdvance = (float)maxGlyphH;
    //result.charWidth = (float)maxGlyphW;
    result.maxDescent = (float)maxDescent;
    
    DeleteObject(result.bitmap);
    DeleteObject(result.handle);
    RemoveFontResourceW(fileName);
    
    return result;
}

// Get a new sprite layer to use
function SpriteGroup *
sprite_group_get_layer(u32 layer)
{
    assert(layer < array_count(engine.spriteGroups));
    return engine.spriteGroups + layer;
}

// Push a draw line command into the line group
function void
base_draw_line(float aX, float aY, 
               float bX, float bY,
               float r, float g, float b, float a,
               float layer)
{
    LineGroup *group = &engine.lineGroup;
    LineCommand *command = group->lineCommands + 
        group->lineCommandsPushedCount;
    command->a = v2(aX,aY);
    command->b = v2(bX,bY);
    float eps = 0.000f;
    command->col = rgba(r, g, b, a);
    command->layer = layer;
    
    group->lineCommandsPushedCount++;
}

// Push a draw sprite command into the sprite group
function void
base_draw_rect(SpriteGroup *group,
               float x, float y, 
               float width, float height, 
               float minU, float minV,
               float maxU, float maxV,
               float r, float g, float b, float a,
               float layer)
{
    SpriteCommand *command = group->spriteCommands + 
        group->spriteCommandsPushedCount;
    command->pos = v2(x,y);
    command->size = v2(width,height);
    float eps = 0.000f;
    command->uv = rect2(minU + eps, maxV - eps, maxU - eps, minV + eps);
    command->col = rgba(r, g, b, a);
    command->layer = layer;
    
    group->spriteCommandsPushedCount++;
}

// Push a sprite command into the sprite group
function void
draw_rect(SpriteGroup *group,
          Sprite sprite,
          Vector2 pos, Vector2 size,
          Color col, float layer)
{
    base_draw_rect(group, 
                   pos.x, pos.y,
                   size.x, size.y,
                   sprite.uv.min.x, sprite.uv.min.y,
                   sprite.uv.max.x, sprite.uv.max.y,
                   col.r, col.g, col.b, col.a,
                   layer);
}

function void
base_draw_outline_rect(float x, float y,
                       float width, float height,
                       float r, float g, float b, float a,
                       float layer)
{
    // Draw bottom line
    base_draw_line(x, y, 
                   x + width, y,
                   r, g, b, a,
                   layer);
    
    // Draw top line
    base_draw_line(x, y + height, 
                   x + width, y + height,
                   r, g, b, a,
                   layer);
    
    // Draw left line
    base_draw_line(x, y, 
                   x, y + height + 1,
                   r, g, b, a,
                   layer);
    // Draw right line
    base_draw_line(x + width, y, 
                   x + width, y + height,
                   r, g, b, a,
                   layer);
}

// Push a sprite command into the sprite group (same as above)
function void
draw_sprite(SpriteGroup *group,
            Sprite sprite, 
            Vector2 pos, Vector2 scale,
            Color col, float layer)
{
    base_draw_rect(group,
                   pos.x, pos.y,
                   scale.x * sprite.size.x, scale.y * sprite.size.y,
                   sprite.uv.min.x, sprite.uv.min.y,
                   sprite.uv.max.x, sprite.uv.max.y,
                   col.r, col.g, col.b, col.a,
                   layer);
}

function void
draw_glyph(SpriteGroup *group, TruetypeFont *font,
           u32 unicode, Vector2 pos, Color col, float layer)
{
#if 0
    // Draw the baseline
    base_draw_line(pos.x, pos.y,
                   pos.x + font->charWidth-1, pos.y,
                   .8f,.8f,.8f,.8f, 
                   -100);
#endif
    
    HashTableNode *node = hash_table_get(&font->glyphsTable, unicode, (float)unicode);
    if (node)
    {
        Sprite *glyphSprite = (Sprite *)node->data;
        if (glyphSprite)
        {
            pos.x -= glyphSprite->align.x;
            pos.y -= glyphSprite->align.y;
            
            draw_sprite(group, *glyphSprite, pos, v2(1,1), col, layer);
        }
    }
}

function void
draw_string(SpriteGroup *group, TruetypeFont *font,
            wchar_t *s, Vector2 pos, Color col, float layer)
{
    wchar_t *at = s;
    while (*at)
    {
        draw_glyph(group, font, *at, pos, col, layer);
        
        pos.x += font->charWidth;
        
        at++;
    }
}



/******************************************************************************
*** [WINDOWPROC]
******************************************************************************/

LRESULT window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message) {
        
        case WM_DESTROY:
        case WM_CLOSE:
        {
            engine.running = false;
        } break;
        
        case WM_SIZE:
        {
            // TODO: Paint the window black or something
        } break;
        
        case WM_CHAR:
        {
            engine.inputChar = (wchar_t)wParam;
            engine.inputCharEntered = true;
        } break;
        
        case WM_MOUSEWHEEL:
        {
            engine.mouse.wheel = ((float)GET_WHEEL_DELTA_WPARAM(wParam) / 
                                  (float)WHEEL_DELTA);
        } break;
        
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            Key *key = 0;
            
            if (wParam == VK_UP)              key = &engine.key.up;
            else if (wParam == VK_LEFT)       key = &engine.key.left;
            else if (wParam == VK_DOWN)       key = &engine.key.down;
            else if (wParam == VK_RIGHT)      key = &engine.key.right;
            else if (wParam == VK_BACK)       key = &engine.key.backspace;
            else if (wParam == VK_MENU)       key = &engine.key.alt;
            else if (wParam == VK_F1)         key = &engine.key.f1;
            else if (wParam == 'S')           key = &engine.key.s;
            else if (wParam == 'D')           key = &engine.key.d;
            else if (wParam == 'C')           key = &engine.key.c;
            else if (wParam == 'V')           key = &engine.key.v;
            else if (wParam == 'T')           key = &engine.key.t;
            else if (wParam == VK_CONTROL)    key = &engine.key.control;
            else if (wParam == VK_SPACE)      key = &engine.key.space;
            
            if (key)
            {
                key->down = (message == WM_KEYDOWN);
                key->pressed = (message == WM_KEYDOWN);
                key->released = (message == WM_KEYUP);
            }
            
        } break;
        
        case WM_LBUTTONDOWN:
        {
            engine.mouse.left.down = true;
            engine.mouse.left.pressed = true;
        } break;
        
        case WM_RBUTTONDOWN:
        {
            engine.mouse.right.down = true;
            engine.mouse.right.pressed = true;
        } break;
        
        case WM_LBUTTONUP:
        {
            engine.mouse.left.down = false;
            engine.mouse.left.released = true;
        } break;
        
        case WM_RBUTTONUP:
        {
            engine.mouse.right.down = false;
            engine.mouse.right.released = true;
        } break;
        
        // [W3] . Call DefWindowProcA for every message we don't handle
        default:
        {
            result = DefWindowProcW(window, message, wParam, lParam);
        } break;
    }
    return result;
}

function void
create_sprites_shaders()
{
    // First we will declare the vertex shader code
    char *vertexShaderSource = 
        "struct vs_in"
        "{"
        "float3 position_local : POS;"
        "float2 uv             : TEX;"
        "float4 color          : COL;"
        "};"
        
        "struct vs_out"
        "{"
        "float4 position_clip : SV_POSITION;"
        "float2 uv            : TEXCOORD;"
        "float4 color         : COLOR;"
        "};"
        
        "cbuffer cbPerObject"
        "{"
        "float4x4 WVP;"
        "};"
        
        "vs_out vs_main(vs_in input)"
        "{"
        "vs_out output = (vs_out)0;"
        "float4 pos = float4(input.position_local, 1.0);"
        "pos.z += 500;"
        
        "output.position_clip = mul(pos, WVP);"
        "output.uv = input.uv;"
        "output.color = input.color;"
        "return output;"
        "}";
    
    // Then we will compile the vertex shader
    ID3DBlob *compiledVS;
    ID3DBlob *errorVertexMessages;
    if (FAILED(D3DCompile(vertexShaderSource, strlen(vertexShaderSource) + 1,
                          0, 0, 0, "vs_main", "vs_5_0",
                          0, 0, &compiledVS, &errorVertexMessages)))
    {
        if (errorVertexMessages)
        {
            wchar_t *msg = (wchar_t *)
            (ID3D10Blob_GetBufferPointer(errorVertexMessages));
            OutputDebugStringW(msg);
            exit(1);
        }
        else
        {
            exit(1);
        }
    }
    
    // And create the vertex shader if compilation went ok
    ID3D11Device_CreateVertexShader(engine.device,
                                    ID3D10Blob_GetBufferPointer(compiledVS),
                                    ID3D10Blob_GetBufferSize(compiledVS),
                                    NULL,
                                    &engine.spritesVertexShader);
    
    // Then the pixel shader code
    char *pixelShaderSource = 
        "struct ps_in"
        "{"
        "float4 position_clip : SV_POSITION;"
        "float2 uv            : TEXCOORD;"
        "float4 color         : COLOR;"
        "};"
        
        "Texture2D tex;"
        "SamplerState samp;"
        
        "float4 ps_main(ps_in input) : SV_TARGET"
        "{"
        "  return tex.Sample(samp, input.uv) * input.color;"
        "}";
    
    // And compilation of the pixel shader
    ID3DBlob *compiledPS;
    ID3DBlob *errorPixelMessages;
    if (FAILED(D3DCompile(pixelShaderSource, strlen(pixelShaderSource) + 1,
                          0, 0, 0, "ps_main", "ps_5_0",
                          0, 0, &compiledPS, &errorPixelMessages)))
    {
        if (errorPixelMessages)
        {
            wchar_t *msg = (wchar_t *)
            (ID3D10Blob_GetBufferPointer(errorPixelMessages));
            OutputDebugStringW(msg);
            exit(1);
        }
        else
        {
            exit(1);
        }
    }
    // And create the pixel shader if compilation went ok
    ID3D11Device_CreatePixelShader(engine.device,
                                   ID3D10Blob_GetBufferPointer(compiledPS),
                                   ID3D10Blob_GetBufferSize(compiledPS),
                                   NULL,
                                   &engine.spritesPixelShader);
    
    // Create the input layout
    
    // Make the input element desc array that describes the vertex input 
    // layout we will be using
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        
        { "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        
        { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    // And create the input layout
    void *vsPointer = ID3D10Blob_GetBufferPointer(compiledVS);
    u32 vsSize = (u32)ID3D10Blob_GetBufferSize(compiledVS);
    
    if (FAILED(ID3D11Device_CreateInputLayout(engine.device,
                                              inputElementDesc,
                                              array_count(inputElementDesc),
                                              vsPointer,
                                              vsSize,
                                              &engine.spritesInputLayout)))
    {
        exit(1);
    }
}

function void
create_lines_shaders()
{
    // First we will declare the vertex shader code
    char *vertexShaderSource = 
        "struct vs_in"
        "{"
        "float3 position_local : POS;"
        "float4 color          : COL;"
        "};"
        
        "struct vs_out"
        "{"
        "float4 position_clip : SV_POSITION;"
        "float4 color         : COLOR;"
        "};"
        
        "cbuffer cbPerObject"
        "{"
        "float4x4 WVP;"
        "};"
        
        "vs_out vs_main(vs_in input)"
        "{"
        "vs_out output = (vs_out)0;"
        "float4 pos = float4(input.position_local, 1.0);"
        "pos.z += 500;"
        
        "output.position_clip = mul(pos, WVP);"
        "output.color = input.color;"
        "return output;"
        "}";
    
    // Then we will compile the vertex shader
    ID3DBlob *compiledVS;
    ID3DBlob *errorVertexMessages;
    if (FAILED(D3DCompile(vertexShaderSource, strlen(vertexShaderSource) + 1,
                          0, 0, 0, "vs_main", "vs_5_0",
                          0, 0, &compiledVS, &errorVertexMessages)))
    {
        if (errorVertexMessages)
        {
            wchar_t *msg = (wchar_t *)
            (ID3D10Blob_GetBufferPointer(errorVertexMessages));
            OutputDebugStringW(msg);
            exit(1);
        }
        else
        {
            exit(1);
        }
    }
    
    // And create the vertex shader if compilation went ok
    ID3D11Device_CreateVertexShader(engine.device,
                                    ID3D10Blob_GetBufferPointer(compiledVS),
                                    ID3D10Blob_GetBufferSize(compiledVS),
                                    NULL,
                                    &engine.linesVertexShader);
    
    // Then the pixel shader code
    char *pixelShaderSource = 
        "struct ps_in"
        "{"
        "float4 position_clip : SV_POSITION;"
        "float4 color         : COLOR;"
        "};"
        
        "float4 ps_main(ps_in input) : SV_TARGET"
        "{"
        "  return input.color;"
        "}";
    
    // And compilation of the pixel shader
    ID3DBlob *compiledPS;
    ID3DBlob *errorPixelMessages;
    if (FAILED(D3DCompile(pixelShaderSource, strlen(pixelShaderSource) + 1,
                          0, 0, 0, "ps_main", "ps_5_0",
                          0, 0, &compiledPS, &errorPixelMessages)))
    {
        if (errorPixelMessages)
        {
            wchar_t *msg = (wchar_t *)
            (ID3D10Blob_GetBufferPointer(errorPixelMessages));
            OutputDebugStringW(msg);
            exit(1);
        }
        else
        {
            exit(1);
        }
    }
    // And create the pixel shader if compilation went ok
    ID3D11Device_CreatePixelShader(engine.device,
                                   ID3D10Blob_GetBufferPointer(compiledPS),
                                   ID3D10Blob_GetBufferSize(compiledPS),
                                   NULL,
                                   &engine.linesPixelShader);
    
    //  Create the input layout
    
    // Make the input element desc array that describes the vertex input 
    // layout we will be using
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        
        { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 
            D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    // And create the input layout
    void *vsPointer = ID3D10Blob_GetBufferPointer(compiledVS);
    u32 vsSize = (u32)ID3D10Blob_GetBufferSize(compiledVS);
    
    if (FAILED(ID3D11Device_CreateInputLayout(engine.device,
                                              inputElementDesc,
                                              array_count(inputElementDesc),
                                              vsPointer,
                                              vsSize,
                                              &engine.linesInputLayout)))
    {
        exit(1);
    }
}

function void
create_sprites_vertex_buffer()
{
    // To reuse the buffer later to update its content, it has to be big  
    // enough to hold maxAllowedRenderedSprites worth of sprites.
    // Each sprite requires 6 vertices, each with sizeof(Vertex3D) bytes
    uint32_t vertexBufferSize = 
        MAX_ALLOWED_RENDERED_SPRITES * 6 * sizeof(Vertex3D);
    
    // Then we describe the vertex buffer that we want
    D3D11_BUFFER_DESC vertexBufferDesc =
    {
        vertexBufferSize,
        D3D11_USAGE_DYNAMIC,
        D3D11_BIND_VERTEX_BUFFER,
        D3D11_CPU_ACCESS_WRITE,
        0,
        sizeof(Vertex3D),
    };
    
    // And create the vertex buffer
    if (FAILED(ID3D11Device_CreateBuffer(engine.device,
                                         &vertexBufferDesc, 
                                         0,
                                         &engine.spritesVertexBuffer)))
    {
        exit(1);
    }
}

function void
create_lines_vertex_buffer()
{
    // Limit the number of concurrent rendered sprites to improve performance
    uint32_t maxAllowedRenderedLines = 10000;
    
    // To reuse the buffer later to update its content, it has to be big  
    // enough to hold maxAllowedRenderedLines worth of lines.
    // Each line requires 2 vertices, each with sizeof(LineVertex3D) bytes
    uint32_t vertexBufferSize = 
        maxAllowedRenderedLines * 2 * sizeof(LineVertex3D);
    
    // Then we describe the vertex buffer that we want
    D3D11_BUFFER_DESC vertexBufferDesc =
    {
        vertexBufferSize,
        D3D11_USAGE_DYNAMIC,
        D3D11_BIND_VERTEX_BUFFER,
        D3D11_CPU_ACCESS_WRITE,
        0,
        sizeof(LineVertex3D),
    };
    
    // And create the vertex buffer
    if (FAILED(ID3D11Device_CreateBuffer(engine.device,
                                         &vertexBufferDesc, 
                                         0,
                                         &engine.linesVertexBuffer)))
    {
        exit(1);
    }
}

function u32
produce_vertices_from_sprite_groups()
{
    // Declare a variable to hold the count of all vertices produced
    u32 allSpritesVerticesCount = 0;
    
    // Count how many vertices in total was pushed
    // TODO: Optimize by holding a count variable at push time
    for (u32 i = 0; i < array_count(engine.spriteGroups); i++)
    {
        SpriteGroup *group = engine.spriteGroups + i;
        allSpritesVerticesCount += 6*group->spriteCommandsPushedCount;
    }
    
    // Allocate array to hold all vertices
    Vertex3D *vertices = (Vertex3D *)
        malloc(allSpritesVerticesCount * sizeof(Vertex3D));
    
    // Declare a variable to count vertices copied so far
    s32 verticesCopiedSoFar = 0;
    
    // For each sprite group
    for (u32 j = 0; j < array_count(engine.spriteGroups); j++)
    {
        SpriteGroup *group = engine.spriteGroups + j;
        
        if (group->spriteCommandsPushedCount > 0)
        {
            u32 verticesCount = 6 * group->spriteCommandsPushedCount;
            
            // Start the count from all vertices so far
            u32 vertexIndex = verticesCopiedSoFar;
            
            // Go through the entire render commands array and produce the 
            // vertices for each command
            for (u32 i = 0; i < group->spriteCommandsPushedCount; ++i)
            {
                SpriteCommand *cmd = group->spriteCommands + i;
                
                Vertex3D a =
                {
                    cmd->pos.x, cmd->pos.y, cmd->layer,
                    cmd->uv.min.x, cmd->uv.min.y,
                    cmd->col.r, cmd->col.g, cmd->col.b, cmd->col.a
                };
                
                Vertex3D b =
                {
                    cmd->pos.x+cmd->size.x, cmd->pos.y, cmd->layer,
                    cmd->uv.max.x, cmd->uv.min.y,
                    cmd->col.r, cmd->col.g, cmd->col.b, cmd->col.a
                };
                
                Vertex3D c =
                {
                    cmd->pos.x+cmd->size.x, cmd->pos.y+cmd->size.y, cmd->layer,
                    cmd->uv.max.x, cmd->uv.max.y,
                    cmd->col.r, cmd->col.g, cmd->col.b, cmd->col.a
                };
                
                Vertex3D d =
                {
                    cmd->pos.x, cmd->pos.y+cmd->size.y, cmd->layer,
                    cmd->uv.min.x, cmd->uv.max.y,
                    cmd->col.r, cmd->col.g, cmd->col.b, cmd->col.a
                };
                
                // Copy the vertices to the vertices array
                vertices[vertexIndex++] = a;
                vertices[vertexIndex++] = (TOP_DOWN ? b : c);
                vertices[vertexIndex++] = (TOP_DOWN ? c : b);
                
                vertices[vertexIndex++] = a;
                vertices[vertexIndex++] = (TOP_DOWN ? c : d);
                vertices[vertexIndex++] = (TOP_DOWN ? d : c);
            }
            
            // Add to copied so far count
            verticesCopiedSoFar += verticesCount;
            
            // Reset the count
            group->spriteCommandsPushedCount = 0;
        }
    }
    
    // Copy the data to the sprites vertex buffer
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    
    // Map the vertex buffer resource
    ID3D11DeviceContext_Map(engine.context,
                            (ID3D11Resource *)engine.spritesVertexBuffer,
                            0, 
                            D3D11_MAP_WRITE_DISCARD, 
                            0, 
                            &mappedResource);
    
    // Copy the vertices into the mapped resource memory
    memcpy(mappedResource.pData, vertices, 
           allSpritesVerticesCount * sizeof(Vertex3D));
    
    // Unmap the vertex buffer
    ID3D11DeviceContext_Unmap(engine.context,
                              (ID3D11Resource *)engine.spritesVertexBuffer,
                              0);
    
    // Free the vertices array
    free(vertices);
    
    return allSpritesVerticesCount;
}

function u32
produce_vertices_from_line_group()
{
    // Declare a variable to hold the count of all vertices produced
    u32 allLinesVerticesCount = 0;
    
    // Count how many vertices in total was pushed
    // TODO: Optimize by holding a count variable at push time
    LineGroup *group = &engine.lineGroup;
    allLinesVerticesCount += 2*group->lineCommandsPushedCount;
    
    // Allocate array to hold all vertices
    LineVertex3D *vertices = (LineVertex3D *)
        malloc(allLinesVerticesCount * sizeof(LineVertex3D));
    
    // Declare a variable to count vertices copied so far
    s32 verticesCopiedSoFar = 0;
    
    if (group->lineCommandsPushedCount > 0)
    {
        u32 verticesCount = 2*group->lineCommandsPushedCount;
        
        // Start the count from all vertices so far
        u32 vertexIndex = verticesCopiedSoFar;
        
        // Go through the entire render commands array and produce the 
        // vertices for each command
        for (u32 i = 0; i < group->lineCommandsPushedCount; ++i)
        {
            LineCommand *cmd = group->lineCommands + i;
            
            LineVertex3D a =
            {
                cmd->a.x, cmd->a.y, cmd->layer,
                cmd->col.r, cmd->col.g, cmd->col.b, cmd->col.a
            };
            
            LineVertex3D b =
            {
                cmd->b.x, cmd->b.y, cmd->layer,
                cmd->col.r, cmd->col.g, cmd->col.b, cmd->col.a
            };
            
            // Copy the vertices to the vertices array
            vertices[vertexIndex++] = a;
            vertices[vertexIndex++] = b;
        }
        
        // Add to copied so far count
        verticesCopiedSoFar += verticesCount;
        
        // Reset the count
        group->lineCommandsPushedCount = 0;
    }
    
    // Copy the data to the lines vertex buffer
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    
    // Map the vertex buffer resource
    ID3D11DeviceContext_Map(engine.context,
                            (ID3D11Resource *)engine.linesVertexBuffer,
                            0, 
                            D3D11_MAP_WRITE_DISCARD, 
                            0, 
                            &mappedResource);
    
    // Copy the vertices into the mapped resource memory
    memcpy(mappedResource.pData, vertices, 
           allLinesVerticesCount*sizeof(LineVertex3D));
    
    // Unmap the vertex buffer
    ID3D11DeviceContext_Unmap(engine.context,
                              (ID3D11Resource *)engine.linesVertexBuffer,
                              0);
    
    // Free the vertices array
    free(vertices);
    
    return allLinesVerticesCount;
}

function void
render_pass(D3D11_PRIMITIVE_TOPOLOGY topology, 
            ID3D11InputLayout *inputLayout,
            u32 verticesCount,
            ID3D11Buffer *vertexBuffer,
            u32 vertexBufferStride,
            ID3D11VertexShader *vertexShader,
            ID3D11PixelShader *pixelShader)
{
    // Configure the input assembly pipeline stage
    
    // Set the input assembly topology to use triangle list
    ID3D11DeviceContext_IASetPrimitiveTopology(engine.context, topology);
    
    // Set the input layout
    ID3D11DeviceContext_IASetInputLayout(engine.context, inputLayout);
    
    // Set the vertex buffers (only 1 atm)
    ID3D11Buffer *vertexBuffers[1] = {vertexBuffer};
    UINT vertexBufferStrides[1] = {vertexBufferStride};
    UINT vertexBufferOffsets[1] = {0};
    ID3D11DeviceContext_IASetVertexBuffers(engine.context,
                                           0,
                                           1, 
                                           vertexBuffers,
                                           vertexBufferStrides,
                                           vertexBufferOffsets);
    
    //  Configure the shaders
    
    // Set the vertex shader
    ID3D11DeviceContext_VSSetShader(engine.context,
                                    vertexShader, 
                                    NULL, 
                                    0);
    
    // Set the vertex constant buffer to the pipeline
    ID3D11DeviceContext_VSSetConstantBuffers(engine.context,
                                             0, 
                                             1, 
                                             &engine.vertexCBuffer);
    
    // Set the pixel shader
    ID3D11DeviceContext_PSSetShader(engine.context,
                                    pixelShader, 
                                    NULL, 
                                    0);
    
    // Set the pixel shader state sampler state
    ID3D11DeviceContext_PSSetSamplers(engine.context,
                                      0, 
                                      1,
                                      &engine.samplerState);
    
    // Set the pixel shader resource for the texture
    ID3D11DeviceContext_PSSetShaderResources(engine.context,
                                             0, 
                                             1, 
                                             &engine.atlasTexSRV);
    
    // Configure the rasterizer
    
    // Set the viewports (only 1 atm)
    D3D11_VIEWPORT viewPorts[1] = 
    {
        {
            0, 0, 
            engine.backBufferSize.x, engine.backBufferSize.y, 
            0.0f, 1.0f
        }
    };
    
    ID3D11DeviceContext_RSSetViewports(engine.context,
                                       1, 
                                       viewPorts);
    
    // Set the scissor rects (only 1 atm)
    D3D11_RECT scissorRects[1] = 
    {
        {
            0, 0, 
            (LONG)engine.backBufferSize.x, (LONG)engine.backBufferSize.y
        }
    };
    
    ID3D11DeviceContext_RSSetScissorRects(engine.context,
                                          1, 
                                          scissorRects);
    
    // Set the rasterizer state
    ID3D11DeviceContext_RSSetState(engine.context,
                                   engine.rasterizerState);
    
    // Configure the output merger
    
    // Set the depth stencil state
    ID3D11DeviceContext_OMSetDepthStencilState(engine.context,
                                               engine.dss,
                                               0);
    
    // Set the blend state
    ID3D11DeviceContext_OMSetBlendState(engine.context,
                                        engine.blendState, 
                                        NULL, 
                                        0xffffffff);
    
    // Draw
    ID3D11DeviceContext_Draw(engine.context, verticesCount, 0);
    
}

function void
swap_chain_resize()
{
    RECT rect;
    if (!GetClientRect(engine.window, &rect))
    {
        exit(1);
    }
    
    Vector2 backBufferSize =
    {
        (float)(rect.right - rect.left),
        (float)(rect.bottom - rect.top),
    };
    
    if (((UINT)backBufferSize.x != 0 && (UINT)backBufferSize.y != 0) &&
        (((UINT)backBufferSize.x != engine.backBufferSize.x) ||
         ((UINT)backBufferSize.y != engine.backBufferSize.y)))
    {
        engine.backBufferSize = backBufferSize;
        
        // Release render target view, depth stencil view
        // back buffer texture and depth stencil buffer texture
        ID3D11RenderTargetView_Release(engine.rtv);
        ID3D11DepthStencilView_Release(engine.dsv);
        ID3D11Texture2D_Release(engine.bbTexture);
        ID3D11Texture2D_Release(engine.dsbTexture);
        
        // Resize buffers
        IDXGISwapChain_ResizeBuffers(engine.swapChain,
                                     2,
                                     (UINT)backBufferSize.x,
                                     (UINT)backBufferSize.y,
                                     DXGI_FORMAT_R8G8B8A8_UNORM,
                                     0);
        
        // Get new back buffer texture and create render target view
        if (FAILED(IDXGISwapChain_GetBuffer(engine.swapChain, 
                                            0, 
                                            &IID_ID3D11Texture2D, 
                                            (void **)
                                            &engine.bbTexture)))
        {
            exit(1);
        }
        
        // Now we can create the views, first the render target view
        if (FAILED(ID3D11Device_CreateRenderTargetView(engine.device, 
                                                       (ID3D11Resource *)
                                                       engine.bbTexture, 
                                                       0,
                                                       &engine.rtv)))
        {
            exit(1);
        }
        
        // Create new depth stencil buffer texture
        
        // Update the description to have the new width and height
        engine.dsbTexture2DDesc.Width = (UINT)backBufferSize.x;
        engine.dsbTexture2DDesc.Height = (UINT)backBufferSize.y;
        
        // We create the texture for the depth stencil buffer
        if (FAILED(ID3D11Device_CreateTexture2D(engine.device, 
                                                &engine.dsbTexture2DDesc, 
                                                0, 
                                                &engine.dsbTexture)))
        {
            exit(1);
        }
        
        // Create depth stencil view
        if (FAILED(ID3D11Device_CreateDepthStencilView(engine.device, 
                                                       (ID3D11Resource *)
                                                       engine.dsbTexture, 
                                                       &engine.dsvDesc, 
                                                       &engine.dsv)))
        {
            exit(1);
        }
    }
    
    resized();
}

typedef struct
{
    char chunkID[4];
    u32 chunkSize;
    char format[4];
} WavRIFFHeader;

typedef struct
{
    char subChunkID[4];
    u32 subChunkSize;
    u16 audioFormat;
    u16 numChannels;
    u32 sampleRate;
    u32 byteRate;
    u16 blockAlign;
    u16 bitsPerSample;
} WavFMTChunk;

typedef struct
{
    char subChunkID[4];
    u32 subChunkSize;
} WavDataChunk;

typedef struct
{
    WavRIFFHeader riffHeader;
    WavFMTChunk fmtChunk;
    WavDataChunk dataChunk;
    
    wchar_t fileName[260];
    s16 *sampleData;
    u32 sampleCount;
} Sound;

function Sound
load_wav(wchar_t *filePath, wchar_t *fileName)
{
    Sound result = {0};
    
    // Open file
    FILE *file;
    _wfopen_s(&file, filePath, L"rb");
    
    // If the file exists
    if (file != 0)
    {
        // Read the RIFF
        fread(&result.riffHeader, sizeof(WavRIFFHeader), 1, file);
        
        // Read the FMT chunk
        fread(&result.fmtChunk, sizeof(WavFMTChunk), 1, file);
        
        // Read the Data chunk
        fread(&result.dataChunk, sizeof(WavDataChunk), 1, file);
        
        // Calculate sample count
        result.sampleCount = (result.dataChunk.subChunkSize
                              / (result.fmtChunk.numChannels *
                                 (result.fmtChunk.bitsPerSample/8)));
        
        // If the audio format is PCM
        if (result.fmtChunk.audioFormat == 1)
        {
            // Allocate space for audio bytes
            result.sampleData = alloc_size(result.dataChunk.subChunkSize);
            if (result.sampleData != 0)
            {
                // Read the audio bytes
                fread(result.sampleData, result.dataChunk.subChunkSize, 1, file);
                
                // Copy the file name
                string_copy(result.fileName, 260, fileName);
            }
            
        }
        
        // Close the file handle
        fclose(file);
    }
    
    return result;
}

// Direct Sound 8
// Initialize DirectSound
function bool
dsound8_init(void)
{
    // Create DirectSound object
    if (FAILED(DirectSoundCreate8(0, &engine.dsound, 0))) return false;
    
    // Set the wav format
    engine.waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    engine.waveFormat.nChannels = 2;
    engine.waveFormat.nSamplesPerSec = 44100;
    engine.waveFormat.wBitsPerSample = 16;
    engine.waveFormat.nBlockAlign = (engine.waveFormat.nChannels*engine.waveFormat.wBitsPerSample)/8;
    engine.waveFormat.nAvgBytesPerSec = engine.waveFormat.nSamplesPerSec*engine.waveFormat.nBlockAlign;
    
    // Set cooperative level
    if (FAILED(IDirectSound8_SetCooperativeLevel(engine.dsound, 
                                                 engine.window, 
                                                 DSSCL_PRIORITY))) return false;
    
    // Primary buffer is only used to set the format
    IDirectSoundBuffer *primaryBuffer;
    
    DSBUFFERDESC primaryBufferDesc = {0};
    primaryBufferDesc.dwSize = sizeof(primaryBufferDesc);
    primaryBufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
    
    // Create primary buffer to set the format
    if (FAILED(IDirectSound8_CreateSoundBuffer(engine.dsound, 
                                               &primaryBufferDesc, 
                                               &primaryBuffer, 
                                               0))) return false;
    
    // Set the format
    if (FAILED(IDirectSoundBuffer_SetFormat(primaryBuffer,
                                            &engine.waveFormat))) return false;
    
    
    // If got to this point, all is good
    return true;
}

function IDirectSoundBuffer *
dsound8_create_buffer(wchar_t *filePath, wchar_t *fileName)
{
    Sound sound = load_wav(filePath, fileName);
    
    IDirectSoundBuffer *result = 0;
    if (engine.audioBufferIndex == array_count(engine.audioBuffers))
    {
        return false;
    }
    
    // The actual audio buffer description
    DSBUFFERDESC bufferDescription = {0};
    bufferDescription.dwSize = sizeof(bufferDescription);
    bufferDescription.dwFlags = 0;
    bufferDescription.dwBufferBytes = sound.sampleCount*2*sizeof(s16);
    bufferDescription.lpwfxFormat = &engine.waveFormat;
    
    // Create the actual audio buffer
    if (FAILED(IDirectSound8_CreateSoundBuffer(engine.dsound,
                                               &bufferDescription, 
                                               &engine.audioBuffers[engine.audioBufferIndex],
                                               0))) return false;
    
    result = engine.audioBuffers[engine.audioBufferIndex];
    
    engine.audioBufferIndex++;
    
    return result;
}

/******************************************************************************
*** [ENTRYPOINT]
******************************************************************************/

// Here is the entry point of our application that uses Windows API 
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, 
                     PSTR cmdline, int cmdshow)
{
    // Create device context
    engine.deviceContext = CreateCompatibleDC(GetDC(0));
    
    // We register a window class with the window characteristcs we desire
    WNDCLASSEXW windowClass = 
    {
        sizeof(windowClass), 
        CS_HREDRAW | CS_VREDRAW, 
        window_proc, 
        0, 0, 
        hInst, 
        NULL, NULL, NULL, NULL, 
        L"engine2d_window_class", 
        NULL,
    };
    
    if (RegisterClassExW(&windowClass) == 0) exit(1);
    
    // To create the window the size, we need to expand the width and height
    // to account for the border of the window, hence AdjustWindowRect
    u32 windowWidth = BACKBUFFER_WIDTH;
    u32 windowHeight = BACKBUFFER_HEIGHT;
    RECT size = {0, 0, windowWidth, windowHeight};
    AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, FALSE);
    windowWidth = size.right - size.left;
    windowHeight = size.bottom - size.top;
    
    // Save the backbuffer size to compare it later to see if it changes
    engine.backBufferSize.x = BACKBUFFER_WIDTH;
    engine.backBufferSize.y = BACKBUFFER_HEIGHT;
    
    // We create the window where we will show the buffer we render to
    engine.window = CreateWindowExW(0, 
                                    L"engine2d_window_class", 
                                    WINDOW_TITLE,
                                    WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
                                    0, 0, windowWidth, windowHeight, 
                                    NULL, NULL, 
                                    hInst, 
                                    NULL);
    
    if (engine.window == NULL) exit(1);
    
    // Now for initializing the Direct3D 11 API, start with refresh rate in Hz
    DXGI_RATIONAL refreshRate = 
    { 
        .Numerator = 60, 
        .Denominator = 1,
    };
    
    // Specify a buffer desc that describes the back buffer we will render to
    DXGI_MODE_DESC bufferDesc = 
    { 
        BACKBUFFER_WIDTH, 
        BACKBUFFER_HEIGHT, 
        refreshRate, 
        DXGI_FORMAT_R8G8B8A8_UNORM, 
        DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED, 
        DXGI_MODE_SCALING_CENTERED,
    };
    
    // Specify a sample desc that describes multisample quality if used at all
    DXGI_SAMPLE_DESC backBufferSampleDesc = 
    { 
        .Count = 1, 
        .Quality = 0, 
    };
    
    // Specify the swap chain desc that we will use to create a swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = 
    { 
        bufferDesc, 
        backBufferSampleDesc, 
        DXGI_USAGE_RENDER_TARGET_OUTPUT,
        2, 
        engine.window, 
        true, 
        DXGI_SWAP_EFFECT_FLIP_DISCARD, 
        0, 
    };
    
    // Define an array of feature levels that we accept if the pc doesn't have
    // Direct3D 11.1 available.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };
    
    // Create the Device, the DeviceContext and the SwapChain all in
    // one go with this super function
    if (FAILED(D3D11CreateDeviceAndSwapChain(NULL,
                                             D3D_DRIVER_TYPE_HARDWARE,
                                             NULL,
                                             D3D11_CREATE_DEVICE_DEBUG,
                                             featureLevels,
                                             array_count(featureLevels),
                                             D3D11_SDK_VERSION,
                                             &swapChainDesc,
                                             &engine.swapChain,
                                             &engine.device,
                                             NULL,
                                             &engine.context)))
    {
        exit(1);
    }
    
    // We will get a debugger from the device
    if (FAILED(ID3D11Device_QueryInterface(engine.device, &IID_ID3D11Debug, 
                                           (void **)&engine.debugger)))
    {
        exit(1);
    }
    
    // Get an InfoQueue to set the severity of messages the debugger breaks at
    ID3D11InfoQueue *infoQueue;
    ID3D11Device_QueryInterface(engine.device, &IID_ID3D11InfoQueue, 
                                (void **)&infoQueue);
    
    // Set that severity as we said
    ID3D11InfoQueue_SetBreakOnSeverity(infoQueue, 
                                       D3D11_MESSAGE_SEVERITY_WARNING, TRUE);
    
    // And release the info queue since we no longer need it, what short life!
    ID3D11InfoQueue_Release(infoQueue);
    
    // Get the backbuffer from the swap chain
    if (FAILED(IDXGISwapChain_GetBuffer(engine.swapChain, 
                                        0, 
                                        &IID_ID3D11Texture2D, 
                                        (void **)&engine.bbTexture)))
    {
        exit(1);
    }
    
    // To create a depth stencil buffer texture we need a tex2d desc
    DXGI_FORMAT dsbFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    D3D11_TEXTURE2D_DESC dsbTexture2DDesc = 
    {
        BACKBUFFER_WIDTH,
        BACKBUFFER_HEIGHT,
        0,
        1,
        dsbFormat,
        backBufferSampleDesc,
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_DEPTH_STENCIL,
        0,
        0,
    };
    engine.dsbTexture2DDesc = dsbTexture2DDesc;
    
    // We create the texture for the depth stencil buffer
    if (FAILED(ID3D11Device_CreateTexture2D(engine.device, 
                                            &dsbTexture2DDesc, 
                                            0, 
                                            &engine.dsbTexture)))
    {
        exit(1);
    }
    
    // Configure the texture atlas
    engine.spriteAtlas.size = SPRITE_ATLAS_SIZE;
    engine.spriteAtlas.atX = 0;
    engine.spriteAtlas.atY = 0;
    engine.spriteAtlas.bottom = 0;
    
    // Describe the texture format
    DXGI_FORMAT atlasTextureFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    D3D11_TEXTURE2D_DESC atlasTextureDesc =
    {
        engine.spriteAtlas.size,
        engine.spriteAtlas.size,
        1,
        1,
        atlasTextureFormat,
        {1,0},
        D3D11_USAGE_DYNAMIC,
        D3D11_BIND_SHADER_RESOURCE,
        D3D11_CPU_ACCESS_WRITE,
        0,
    };
    
    // Allocate memory for the texture atlas
    u32 atlasMemorySize = engine.spriteAtlas.size * 
        engine.spriteAtlas.size * 4;
    engine.spriteAtlas.bytes = (u8 *)malloc(atlasMemorySize);
    
    // Get perf frequency to measure fps
    LARGE_INTEGER perfFrequency;
    QueryPerformanceFrequency(&perfFrequency);
    engine.perfFrequency = perfFrequency.QuadPart;
    
    // Init the audio engine
    dsound8_init();
    
    // Call the app because they will be the ones loading the pngs 
    init();
    
    // Fill the subresource data
    D3D11_SUBRESOURCE_DATA atlasData =
    {
        engine.spriteAtlas.bytes,
        (UINT)(engine.spriteAtlas.size * 4),
        0
    };
    
    // Create the texture
    if (FAILED(ID3D11Device_CreateTexture2D(engine.device,
                                            &atlasTextureDesc,
                                            &atlasData,
                                            &engine.atlasTexture)))
    {
        exit(1);
    }
    
    // Now we can create the views, first the render target view
    if (FAILED(ID3D11Device_CreateRenderTargetView(engine.device, 
                                                   (ID3D11Resource *)
                                                   engine.bbTexture, 
                                                   0,
                                                   &engine.rtv)))
    {
        exit(1);
    }
    
    // Then, to create the ds view we need a depth stencil view description
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = 
    {
        dsbFormat,
        D3D11_DSV_DIMENSION_TEXTURE2D,
        0,
        .Texture2D = { 0 },
    };
    engine.dsvDesc = dsvDesc;
    
    // So we create the depth stencil view as well
    if (FAILED(ID3D11Device_CreateDepthStencilView(engine.device, 
                                                   (ID3D11Resource *)
                                                   engine.dsbTexture, 
                                                   &dsvDesc, 
                                                   &engine.dsv)))
    {
        exit(1);
    }
    
    // Create the shader resource view for the texture
    D3D11_SHADER_RESOURCE_VIEW_DESC atlasTextureSRVDesc;
    atlasTextureSRVDesc.Format = atlasTextureFormat;
    atlasTextureSRVDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
    atlasTextureSRVDesc.Texture2D.MostDetailedMip = 0;
    atlasTextureSRVDesc.Texture2D.MipLevels = 1;
    
    if (FAILED(ID3D11Device_CreateShaderResourceView(engine.device,
                                                     (ID3D11Resource *)
                                                     engine.atlasTexture, 
                                                     &atlasTextureSRVDesc, 
                                                     &engine.atlasTexSRV)))
    {
        exit(1);
    }
    
    // Now we will need to create some D3D11 states, we need a ds desc
    D3D11_DEPTH_STENCIL_DESC dsDesc = 
    {
        .DepthEnable = true,
        D3D11_DEPTH_WRITE_MASK_ALL,
        D3D11_COMPARISON_GREATER_EQUAL,
        .StencilEnable = false,
        .StencilReadMask = 0,
        .StencilWriteMask = 0,
        .FrontFace = {0},
        .BackFace = {0},
    };
    
    // To make the depth stencil state
    if (FAILED(ID3D11Device_CreateDepthStencilState(engine.device, 
                                                    &dsDesc, 
                                                    &engine.dss)))
    {
        exit(1);
    }
    
    // We need an array of render target blend descs. We could potentially
    // render up to 8 render targets at once, hence 8 blend states.
    // But we will only be using one.
    D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = 
    {
        .BlendEnable = true,
        .SrcBlend = D3D11_BLEND_SRC_ALPHA,
        .DestBlend = D3D11_BLEND_INV_SRC_ALPHA,
        .BlendOp = D3D11_BLEND_OP_ADD,
        .SrcBlendAlpha = D3D11_BLEND_ONE,
        .DestBlendAlpha = D3D11_BLEND_ZERO,
        .BlendOpAlpha = D3D11_BLEND_OP_ADD,
        .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
    };
    
    // We will be needing a blend desc, it has an array of render targets
    // So we will clear this to zero to avoid having garbage set.
    D3D11_BLEND_DESC blendDesc;
    memset(&blendDesc, 0, sizeof(blendDesc));
    
    // Now we fill the blend desc
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0] = renderTargetBlendDesc;
    
    // And create the blend state
    if (FAILED(ID3D11Device_CreateBlendState(engine.device, 
                                             &blendDesc, 
                                             &engine.blendState)))
    {
        exit(1);
    }
    
    // Now for the rasterizer state we need a rasterizer desc
    D3D11_RASTERIZER_DESC rasterizerDesc =
    {
        D3D11_FILL_SOLID,
        D3D11_CULL_BACK,
        .FrontCounterClockwise = false,
        .DepthBias = 0,
        .DepthBiasClamp = 0,
        .SlopeScaledDepthBias = 0,
        .DepthClipEnable = true,
        .ScissorEnable = true,
        .MultisampleEnable = false,
        .AntialiasedLineEnable = false,
    };
    
    // And create the rasterizer state
    if (FAILED(ID3D11Device_CreateRasterizerState(engine.device, 
                                                  &rasterizerDesc, 
                                                  &engine.rasterizerState)))
    {
        exit(1);
    }
    
    // Then we will need a sampler state, so sampler desc it is
    D3D11_SAMPLER_DESC samplerDesc;
    memset(&samplerDesc, 0, sizeof(samplerDesc));
    // Clear to zero above then fill it
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    // Then create the sampler state
    if (FAILED(ID3D11Device_CreateSamplerState(engine.device, 
                                               &samplerDesc, 
                                               &engine.samplerState)))
    {
        exit(1);
    }	
    
    create_sprites_shaders();
    create_lines_shaders();
    
    create_sprites_vertex_buffer();
    create_lines_vertex_buffer();
    
    // A constant buffer represents a single data structure
    // thus no stride is needed
    // Unlike vertex and index buffers, which represent arrays of structures
    uint32_t constantBufferStride = 0;
    
    // We will use this constant buffer to upload the World View Projection 
    // matrix to the vertex shader stage of the pipeline, thus the 
    // vertexConstantBufferDesc name
    D3D11_BUFFER_DESC vertexConstantBufferDesc =
    {
        sizeof(VertexConstantBuffer),
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_CONSTANT_BUFFER,
        0,
        0,
        constantBufferStride,
    };
    
    // Then we make the constant buffer to use with the vertex buffer
    if (FAILED(ID3D11Device_CreateBuffer(engine.device,
                                         &vertexConstantBufferDesc, 
                                         0,
                                         &engine.vertexCBuffer)))
    {
        exit(1);
    }
    
    u32 perGroupCount = MAX_ALLOWED_RENDERED_SPRITES /
        array_count(engine.spriteGroups);
    
    // For each sprite group
    for (u32 i = 0; i < array_count(engine.spriteGroups); i++)
    {
        SpriteGroup *group = engine.spriteGroups + i;
        
        // Create arrays of SpriteCommand's
        group->spriteCommands = (SpriteCommand *)
            malloc(perGroupCount * sizeof(SpriteCommand));
        
        group->spriteCommandsPushedCount = 0;
    }
    
    // For the line group
    LineGroup *group = &engine.lineGroup; 
    group->lineCommands = (LineCommand *)
        malloc(MAX_ALLOWED_RENDERED_LINES * sizeof(LineCommand));
    
    LARGE_INTEGER lastCounter;
    QueryPerformanceCounter(&lastCounter);
    
    engine.running = true;
    while (engine.running)
    {
        MSG message;
        while (PeekMessageW(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        
        // Get mouse position
        POINT mousePoint;
        if (GetCursorPos(&mousePoint))
        {
            if (ScreenToClient(engine.window, &mousePoint))
            {
                engine.mouse.pos.x = (float)mousePoint.x; 
                engine.mouse.pos.y = engine.backBufferSize.y - (float)mousePoint.y; 
            }
        }
        
        // Mouse dragging
        if (engine.mouse.dragging && !engine.mouse.left.down)
        {
            engine.mouse.dragging = false;
        }
        
        // Measure fps
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        
        // Calculater fps
        engine.dt = ((float)(counter.QuadPart - lastCounter.QuadPart) / 
                     (float)engine.perfFrequency);
        
        // Save counter
        lastCounter = counter;
        
        
        // Call update for the app
        update();
        
        swap_chain_resize();
        
        u32 allSpritesVerticesCount = produce_vertices_from_sprite_groups();
        
        u32 allLinesVerticesCount = produce_vertices_from_line_group();
        
        float scaleX = 2.0f / engine.backBufferSize.x;
        float scaleY = (TOP_DOWN ? -2.0f : 2.0f) / engine.backBufferSize.y;
        Matrix4x4 wvpMatrix = 
        {
            scaleX, 0, 0, -1,
            0, scaleY, 0, (TOP_DOWN ? 1.f : -1.f),
            0, 0, .001f, 0,
            0, 0, 0, 1,
        };
        
        // Update the vertex shader constant buffer wvp matrix
        engine.vertexCBufferData.WVP = wvpMatrix;
        
        // Update the actual vertex shader constant buffer data
        ID3D11DeviceContext_UpdateSubresource(engine.context,
                                              (ID3D11Resource *)
                                              engine.vertexCBuffer,
                                              0,
                                              NULL,
                                              &engine.vertexCBufferData,
                                              0,
                                              0);
        
        // Set render target and clear
        
        // Set the render target views (only 1 atm)
        ID3D11RenderTargetView *views[1] = 
        {
            engine.rtv
        };
        
        ID3D11DeviceContext_OMSetRenderTargets(engine.context,
                                               1, 
                                               views,
                                               engine.dsv);
        
        // Clear the render target view and depth stencil view
        ID3D11DeviceContext_ClearRenderTargetView(engine.context,
                                                  engine.rtv, 
                                                  CLEAR_COLOR.data);
        
        ID3D11DeviceContext_ClearDepthStencilView(engine.context,
                                                  engine.dsv, 
                                                  D3D11_CLEAR_DEPTH | 
                                                  D3D11_CLEAR_STENCIL, 
                                                  0, 
                                                  0);
        
        
        // Lines
        render_pass(D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
                    engine.linesInputLayout, allLinesVerticesCount,
                    engine.linesVertexBuffer, sizeof(LineVertex3D),
                    engine.linesVertexShader,
                    engine.linesPixelShader);
        // Sprites
        render_pass(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 
                    engine.spritesInputLayout, allSpritesVerticesCount,
                    engine.spritesVertexBuffer, sizeof(Vertex3D),
                    engine.spritesVertexShader,
                    engine.spritesPixelShader);
        
        IDXGISwapChain_Present(engine.swapChain,
                               1, 
                               0);
        
        // Clear keyboard pressed state from last frame
        for (u32 i = 0; i < array_count(engine.key.all); ++i)
        {
            engine.key.all[i].pressed = false;
        }
        
        // Clear mouse pressed state
        engine.mouse.left.pressed = false;
        engine.mouse.right.pressed = false;
        
        engine.mouse.left.released = false;
        engine.mouse.right.released = false;
        
        engine.mouse.wheel = 0;
        
        // Clear the input char
        engine.inputChar = 0;
        engine.inputCharEntered = false;
    }
    
    // We need to release all Direct3D 11 resources and objects we used
    if (engine.swapChain) IDXGISwapChain_Release(engine.swapChain);
    if (engine.context) ID3D11DeviceContext_Release(engine.context);
    if (engine.device) ID3D11Device_Release(engine.device);
    
    return 0;
}