/******************************************************************************
*** This file implements a 2d engine that uses Windows and Direct3D 11 APIs.
*** NOTE: This file development is currently IN PROGRESS.
******************************************************************************/


/******************************************************************************
*********************************************************** START OF INTERFACE
******************************************************************************/


/******************************************************************************
*** NAME OF EXAMPLE FUNCTION should be all caps to highlight it
*******************************************************************************
*** void example_function(void);
*******************************************************************************
*** This is the example explanation of this example function. Explanations can
*** and often will span multiple lines. Which means it should make clear what
*** happens in the function and how the user uses it.
******************************************************************************/


/******************************************************************************
*** TODO: Describe the interface
******************************************************************************/


/******************************************************************************
************************************************************* END OF INTERFACE
*******************************************************************************
*** From this point on the user of this engine does not need to know anything.
******************************************************************************/


/******************************************************************************
************************************************************ START OF OVERVIEW
*******************************************************************************
*** Following there is a high level overwview of everything this file does.
*** All parts of the code are marked with [D1], [D2], [D3], [W1], [W2], etc...
*** So you can quickly navigate forward and back by searching for these terms.
******************************************************************************/


/******************************************************************************
*** [DEPENDENCIES]
*** [D1] . Include Windows and Direct3D, define COBJMACROS to use D3D11 with C
*** [D2] . Include C libs to use math, string processing, etc...
*** [D3] . Define useful/convenient types that C doesn't have
*** [D4] . Define data structures that are useful in general
*** [D5] . Define data structures that are used by the renderer core
******************************************************************************/


/******************************************************************************
*** [CONFIGURATION]
*** TODO
******************************************************************************/


/******************************************************************************
*** [API]
*** TODO
******************************************************************************/


/******************************************************************************
*** [WINDOWPROC]
*** [W1] . When Close/Destroy message, flag engine.running as false
*** [W2] . Handle keyboard messages
*** [W3] . Call DefWindowProcA for every message we don't handle
******************************************************************************/


/******************************************************************************
*** [ENTRYPOINT]
******************************************************************************/


/******************************************************************************
*** [INITIALIZATION]
*** [I1] . Register a window class and create a window
*** [I2] . Initialize Direct3D 11 API (Create Device, Context and SwapChain)
*** [I3] . Get Direct3D Debugger from Device and set break severity to warning
*** [I4] . Get backbuffer from swapchain and create texture for ds buffer
*** [I5] . Create views for render target and depth stencil buffer
*** [I6] . Create ds state, blend state, rasterizer state and sampler state
*** [I7] . Create vertex shader and pixel shader
*** [I8] . Create the input layout
*** [I9] . Create vertex buffer and constant buffer
******************************************************************************/


/******************************************************************************
*** [RUNTIME]
*** [R1] . Process Windows messages
*** [R2] . Resize the swap chain and buffers if the window size changed 
*** [R3] . Do the rendering
*** [R4] . Update the World View Projection matrix
*** [R5] . Configure the input assembly pipeline stage
*** [R6] . Configure the shaders
*** [R7] . Configure the rasterizer
*** [R8] . Configure the output merger
*** [R9] . Draw and present the frame           ******************************************************************************/


/******************************************************************************
************************************************************** END OF OVERVIEW
******************************************************************************/



/******************************************************************************
****************************************************** START OF IMPLEMENTATION
******************************************************************************/


/******************************************************************************
*** [DEPENDENCIES]
******************************************************************************/


/******************************************************************************
*** [D1] . Include Windows and Direct3D, define COBJMACROS to use D3D11 with C
******************************************************************************/

// We must define this to be able to use Direct3D 11 API in C
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
// Must include Windows API headers
#include <windows.h>
#include <windowsx.h>
// Must include Direct3D 11 API headers
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>


/******************************************************************************
*** [D2] . Include C libraries to use math, string processing, etc...
******************************************************************************/

// Must include C headers
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

/******************************************************************************
*** [D3] . Define useful/convenient types that C doesn't have
******************************************************************************/

// Define short version of common types
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Define a boolean type to make it clear in the code
// But actually it is just an integer with 0 or 1
typedef uint32_t bool;
#define false 0
#define true 1

// Define a global keyword to make it clear in the code
#define global static

// Define a function keyword to make it clear in the code
// and tell the compiler our functions are static
#define function static

#define array_count(a) (sizeof(a) / sizeof((a)[0]))

/******************************************************************************
*** [D4] . Define data structures that are useful in general
******************************************************************************/

typedef struct
{
    float x, y;
} Vector2;

inline Vector2
v2(float x, float y)
{
    Vector2 result = {x, y};
    return result;
}

typedef struct
{
    float x, y, z;
} Vector3;

typedef struct
{
    float x, y, z, w;
} Vector4;

typedef struct
{
    float r, g, b, a;
} Color;

inline Color
rgba(float r, float g, float b, float a)
{
    Color result = {r, g, b, a};
    return result;
}

typedef struct
{
    float data[4][4];
} Matrix4x4;

typedef struct
{
    Vector2 min;
    Vector2 max;
} Rect2;

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

typedef struct 
{
    char name[512];
    bool exists;
    u32 size;
    u8 *data;
} File;

function File
read_file(char *filePath)
{
    File result = {0};
    
    // 
    HANDLE handle = CreateFileA(filePath,
                                GENERIC_READ,
                                FILE_SHARE_READ,
                                0,
                                OPEN_EXISTING,
                                0,
                                0);
    
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(handle, &fileSize))
    {
        CloseHandle(handle);
        return result;
    }
    
    result.size = (u32)fileSize.QuadPart;
    
    result.data = (u8 *)malloc(result.size);
    
    DWORD bytesRead = 0;
    if (!ReadFile(handle, result.data, result.size,
                  &bytesRead, 0))
    {
        result.size = 0;
        
        free(result.data);
        result.data = 0;
        
        result.exists = false;
        
        CloseHandle(handle);
        return result;
    }
    
    assert(result.size == bytesRead);
    result.exists = true;
    CloseHandle(handle);
    
    strcpy_s(result.name, 512, filePath);
    
    return result;
}

function bool
write_file(char *filePath, char *data, u32 size)
{
    bool result = false;
    
    HANDLE handle = CreateFileA(filePath,
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

typedef struct
{
    u32 index;
    float comparisonValue;
} SortIndex;

function int quick_sort_indices_partition(SortIndex a[], int beg, int end)
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

function void quick_sort_indices(SortIndex a[], int beg, int end)
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

/******************************************************************************
*** [D5] . Define data structures that are used by the renderer core
******************************************************************************/

typedef struct
{
    Vector3 pos;
    Vector2 uv;
    Color color;
} Vertex3D;

typedef struct 
{
    Matrix4x4 WVP;
} VertexConstantBuffer;

typedef struct
{
    Vector2 pos;
    Vector2 size;
    Rect2 uv;
    Color col;
    float layer;
} RenderCommand;

typedef struct
{
    char name[512];
    Vector2 size;
    Rect2 uv;
} Sprite;

typedef struct
{
    int size;
    int atX;
    int atY;
    int bottom;
    u8 *bytes;
} SpriteAtlas;

typedef struct 
{
    bool down;
    bool pressed;
    bool released;
} Key;

typedef union
{
    Key all[7];
    struct
    {
        Key left;
        Key right;
        Key backspace;
        Key alt;
        Key f1;
        Key s;
        Key control;
    };
} Keys;

// This state will be global to the entire application
typedef struct
{
    bool running;
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
    ID3D11BlendState *blendState;
	ID3D11RasterizerState *rasterizerState;
    ID3D11SamplerState *samplerState;
    ID3D11VertexShader *vertexShader;
    ID3D11PixelShader *pixelShader;
    ID3D11InputLayout *inputLayout;
    ID3D11Buffer *vertexBuffer; // Vertex buffer for input assembler
    ID3D11Buffer *vertexCBuffer; // Constant buffer for vertex shader
    VertexConstantBuffer vertexCBufferData; // Data for above
    RenderCommand *renderCommands;
    u32 renderCommandsPushedCount;
    ID3D11Texture2D *atlasTexture;
    ID3D11ShaderResourceView *atlasTexSRV;
    SpriteAtlas spriteAtlas;
    Keys key;
    bool inputCharEntered;
    char inputChar;
} Engine;

global Engine engine;

function void init();
function void update();

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
#define WINDOW_TITLE "Engine 2D"
#endif

#ifndef TOP_DOWN
#define TOP_DOWN false
#endif

#ifndef SPRITE_ATLAS_SIZE
#define SPRITE_ATLAS_SIZE 4096
#endif

/*****************************************************************************
*** [API]
******************************************************************************/

function u8 *load_png(char *filePath, int *width, int *height)
{
	u8 *result = 0;
    
	int nrChannels;
	unsigned char *data = stbi_load(filePath, width, height, &nrChannels, 0);
    
	if (data)
	{
	    // Use the loaded image data...
		int dataSize = (*width) * (*height) * nrChannels;
        
		result = (u8 *)malloc(dataSize);
		memcpy(result, data, dataSize);
	}
	else
	{
	    // Handle the error case...
	    *width = 0;
	    *height = 0;
	}
    
	return result;
}

function void
render_to_atlas_unchecked(u8 *src,
                          u32 atX, u32 atY,
                          u32 sourceWidth, u32 sourceHeight)
{
    u8 *rowSrc = src;
    
    u8 *rowDest = engine.spriteAtlas.bytes + 
        atY * engine.spriteAtlas.size * 4 +
        atX * 4;
    
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


// Create sprite from file
function Sprite
sprite_create(char *filePath)
{
    Sprite result = {0};
    
    int spriteWidth = 0;
    int spriteHeight = 0;
    
    u8 *bytes = load_png(filePath, &spriteWidth, &spriteHeight);
    
    SpriteAtlas *atlas = &engine.spriteAtlas;
    
    // If the sprite doesn't fit in current row, need to advance to next row
    if ((atlas->atX + spriteWidth) > atlas->size)
    {
        atlas->atX = 0;
        atlas->atY = atlas->bottom;
    }
    
    // If the sprite is taller than previous ones in row, grow the bottom
    if (atlas->bottom < (atlas->atY + spriteHeight))
    {
        atlas->bottom = atlas->atY + spriteHeight;
    }
    
    assert(atlas->bottom <= atlas->size);
    
    // Render the png bytes to the atlas bytes
    render_to_atlas_unchecked(bytes,
                              atlas->atX, atlas->atY,
                              spriteWidth, spriteHeight);
    
    // Calculate uv based on position
    float minU = (float)atlas->atX / (float)atlas->size;
    float minV = (float)atlas->atY / (float)atlas->size;
    float maxU = ((float)atlas->atX + spriteWidth) / (float)atlas->size;
    float maxV = ((float)atlas->atY + spriteHeight) / (float)atlas->size;
    
    // Advance atX
    atlas->atX += spriteWidth;
    
    result.size = v2((float)spriteWidth, (float)spriteHeight);
    result.uv.min = v2(minU, minV);
    result.uv.max = v2(maxU, maxV);
    
    return result;
}

function void
base_draw_rect(float x, float y, 
               float width, float height, 
               float minU, float minV,
               float maxU, float maxV,
               float r, float g, float b, float a,
               float layer)
{
    RenderCommand *command = engine.renderCommands + 
        engine.renderCommandsPushedCount;
    command->pos = v2(x,y);
    command->size = v2(width,height);
    float eps = 0.001f;
    command->uv = rect2(minU + eps, maxV - eps, maxU - eps, minV + eps);
    command->col = rgba(r, g, b, a);
    command->layer = layer;
    
    
    engine.renderCommandsPushedCount++;
}

function void
draw_rect(Sprite sprite,
          Vector2 pos, Vector2 size,
          Color col, float layer)
{
    base_draw_rect(pos.x, pos.y,
                   size.x, size.y,
                   sprite.uv.min.x, sprite.uv.min.y,
                   sprite.uv.max.x, sprite.uv.max.y,
                   col.r, col.g, col.b, col.a,
                   layer);
}

function void
draw_sprite(Sprite sprite, 
            Vector2 pos, Vector2 scale,
            Color col, float layer)
{
    base_draw_rect(pos.x, pos.y,
                   scale.x * sprite.size.x, scale.y * sprite.size.y,
                   sprite.uv.min.x, sprite.uv.min.y,
                   sprite.uv.max.x, sprite.uv.max.y,
                   col.r, col.g, col.b, col.a,
                   layer);
}

/******************************************************************************
*** [WINDOWPROC]
******************************************************************************/

LRESULT window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    switch (message) {
        
        // [W1] . When Close/Destroy message, flag engine.running as false
        case WM_DESTROY:
        case WM_CLOSE:
        {
            engine.running = false;
        } break;
        
        // [W2] . Handle keyboard messages
        case WM_CHAR:
        {
            engine.inputChar = (char)wParam;
            engine.inputCharEntered = true;
        } break;
        
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            Key *key = 0;
            
            if (wParam == VK_LEFT)       key = &engine.key.left;
            else if (wParam == VK_RIGHT) key = &engine.key.right;
            else if (wParam == VK_BACK)  key = &engine.key.backspace;
            else if (wParam == VK_MENU)  key = &engine.key.alt;
            else if (wParam == VK_F1)  key = &engine.key.f1;
            else if (wParam == 'S')  key = &engine.key.s;
            else if (wParam == VK_CONTROL)  key = &engine.key.control;
            
            
            if (key)
            {
                key->down = (message == WM_KEYDOWN);
                key->pressed = (message == WM_KEYDOWN);
                key->released = (message == WM_KEYUP);
            }
            
        } break;
        
        
        // [W3] . Call DefWindowProcA for every message we don't handle
        default:
        {
            result = DefWindowProcA(window, message, wParam, lParam);
        } break;
    }
    return result;
}


/******************************************************************************
*** [ENTRYPOINT]
******************************************************************************/


// Here is the entry point of our application that uses Windows API 
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, 
                     PSTR cmdline, int cmdshow)
{
    /**************************************************************************
    *** [INITIALIZATION]
    **************************************************************************/
    
    /**************************************************************************
    *** [I1] . Register a window class and create a window
    **************************************************************************/
    
    // We register a window class with the window characteristcs we desire
    WNDCLASSEXA windowClass = 
    {
        sizeof(windowClass), 
        CS_HREDRAW | CS_VREDRAW, 
        window_proc, 
        0, 0, 
        hInst, 
        NULL, NULL, NULL, NULL, 
        "engine2d_window_class", 
        NULL,
    };
    
    if (RegisterClassExA(&windowClass) == 0) exit(1);
    
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
    engine.window = CreateWindowExA(0, 
                                    "engine2d_window_class", 
                                    WINDOW_TITLE,
                                    WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
                                    0, 0, windowWidth, windowHeight, 
                                    NULL, NULL, 
                                    hInst, 
                                    NULL);
    
    if (engine.window == NULL) exit(1);
    
    /**************************************************************************
    *** [I2] . Initialize Direct3D 11 (Create Device, Context and SwapChain)
    **************************************************************************/
    
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
    
    /**************************************************************************
    *** [I3] . Get D3D11 Debugger from Device, set break severity to warning
    **************************************************************************/
    
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
    
    
    /**************************************************************************
    *** [I4] . Get backbuffer from swapchain and create texture for ds buffer
    **************************************************************************/
    
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
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_SHADER_RESOURCE,
        0,
        0,
    };
    
    // Allocate memory for the texture atlas
    u32 atlasMemorySize = engine.spriteAtlas.size * 
        engine.spriteAtlas.size * 4;
    engine.spriteAtlas.bytes = (u8 *)malloc(atlasMemorySize);
    
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
    
    /**************************************************************************
    *** [I5] . Create views for render target and depth stencil buffer
    **************************************************************************/
    
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
    
    /**************************************************************************
    *** [I6] . Create ds state, blend state, rasterizer and sampler states
    **************************************************************************/
    
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
        .DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA,
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
    
    /**************************************************************************
    *** [I7] . Create vertex shader and pixel shader
    **************************************************************************/
    
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
            char *msg = (char *)
            (ID3D10Blob_GetBufferPointer(errorVertexMessages));
			OutputDebugStringA(msg);
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
                                    &engine.vertexShader);
    
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
            char *msg = (char *)
            (ID3D10Blob_GetBufferPointer(errorPixelMessages));
			OutputDebugStringA(msg);
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
                                   &engine.pixelShader);
    
    /**************************************************************************
    *** [I8] . Create the input layout
    **************************************************************************/
    
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
                                              &engine.inputLayout)))
    {
        exit(1);
	}
    
    /**************************************************************************
     *** [I9] . Create vertex buffer and constant buffer
    **************************************************************************/
    
    // Limit the number of concurrent rendered sprites to improve performance
    uint32_t maxAllowedRenderedSprites = 10000;
    
    // To reuse the buffer later to update its content, it has to be big  
    // enough to hold maxAllowedRenderedSprites worth of sprites.
    // Each sprite requires 6 vertices, each with sizeof(Vertex3D) bytes
    uint32_t vertexBufferSize = 
        maxAllowedRenderedSprites * 6 * sizeof(Vertex3D);
    
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
                                         &engine.vertexBuffer)))
	{
		exit(1);
	}
    
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
    
    /**************************************************************************
     *** [I10] . Create the render command arrays
    **************************************************************************/
    
    // Create an array of RenderCommand's
    engine.renderCommands = (RenderCommand *)
        malloc(maxAllowedRenderedSprites * sizeof(RenderCommand));
    engine.renderCommandsPushedCount = 0;
    
    /**************************************************************************
    *** [RUNTIME]
    **************************************************************************/
    
    engine.running = true;
    while (engine.running)
    {
        /**********************************************************************
        ***  [R1] . Process Windows messages
        **********************************************************************/
        
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
        
        // Call update for the app
        update();
        
        /**********************************************************************
        *** [R2] . Resize the swapchain and buffers if the window size changed
        **********************************************************************/
        
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
            dsbTexture2DDesc.Width = (UINT)backBufferSize.x;
            dsbTexture2DDesc.Height = (UINT)backBufferSize.y;
            
            // We create the texture for the depth stencil buffer
            if (FAILED(ID3D11Device_CreateTexture2D(engine.device, 
                                                    &dsbTexture2DDesc, 
                                                    0, 
                                                    &engine.dsbTexture)))
            {
                exit(1);
            }
            
            // Create depth stencil view
            if (FAILED(ID3D11Device_CreateDepthStencilView(engine.device, 
                                                           (ID3D11Resource *)
                                                           engine.dsbTexture, 
                                                           &dsvDesc, 
                                                           &engine.dsv)))
            {
                exit(1);
            }
        }
        
        
        /**********************************************************************
        *** [R3] . Do the rendering
        **********************************************************************/
        
        // We calculate the number of vertices we will draw
        // that is 6 per sprite, each render command is a sprite, so that 
        // makes, renderCommandsPushedCount * 6 = verticesCount
        u32 verticesCount = 6 * engine.renderCommandsPushedCount;
        
        Vertex3D *vertices = (Vertex3D *)
            malloc(verticesCount * sizeof(Vertex3D));
        u32 vertexIndex = 0;
        
#if 0
        // Sort commands based on layer
        
        // Build sort array
        SortIndex *sortArray = (SortIndex *)
            malloc(engine.renderCommandsPushedCount * sizeof(SortIndex));
        memset(sortArray, 0, 
            engine.renderCommandsPushedCount * sizeof(SortIndex));
        
        for (u32 i = 0; i < engine.renderCommandsPushedCount; ++i)
        {
            sortArray[i].comparisonValue = engine.renderCommands[i].layer;
            sortArray[i].index = i;
        }
        
        // Sort it based on comparison value
        quick_sort_indices(sortArray, 0, engine.renderCommandsPushedCount-1);
#endif
        
        // Go through the entire render commands array and produce the 
        // vertices for each command
        for (u32 i = 0; i < engine.renderCommandsPushedCount; ++i)
        {
#if 0
            // Get index from sorted array
            u32 actualIndex = sortArray[i].index;
#else
            // Get index from sorted array
            u32 actualIndex = i;
#endif
            
            RenderCommand *cmd = engine.renderCommands + actualIndex;
            
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
        
        assert(verticesCount == vertexIndex);
        
        // Copy the data to the vertex buffer
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        
        // Map the vertex buffer resource
        ID3D11DeviceContext_Map(engine.context,
                                (ID3D11Resource *)engine.vertexBuffer, 
                                0, 
                                D3D11_MAP_WRITE_DISCARD, 
                                0, 
                                &mappedResource);
        
        // Copy the vertices into the mapped resource memory
        memcpy(mappedResource.pData, vertices, 
            vertexIndex * sizeof(Vertex3D));
        
        // Unmap the vertex buffer
        ID3D11DeviceContext_Unmap(engine.context,
                                  (ID3D11Resource *)engine.vertexBuffer,
                                  0);
        
#if 0
        // Free sorti indices array
        free(sortArray);
#endif
        
        // Free the vertices array
        free(vertices);
        
        // Reset the count
        engine.renderCommandsPushedCount = 0;
        
        /**********************************************************************
        *** [R4] . Update the World View Projection matrix
        **********************************************************************/
        
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
        
        /**********************************************************************
        *** [R5] . Configure the input assembly pipeline stage
        **********************************************************************/
        
        // Set the input assembly topology to use triangle list
        D3D11_PRIMITIVE_TOPOLOGY topology = 
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        ID3D11DeviceContext_IASetPrimitiveTopology(engine.context, topology);
        
        // Set the input layout
        ID3D11DeviceContext_IASetInputLayout(engine.context,
                                             engine.inputLayout);
        
        // Set the vertex buffers (only 1 atm)
        ID3D11Buffer *vertexBuffers[1] = {engine.vertexBuffer};
        UINT vertexBufferStrides[1] = {sizeof(Vertex3D)};
        UINT vertexBufferOffsets[1] = {0};
        ID3D11DeviceContext_IASetVertexBuffers(engine.context,
                                               0,
                                               1, 
                                               vertexBuffers,
                                               vertexBufferStrides,
                                               vertexBufferOffsets);
        
        /**********************************************************************
        *** [R6] . Configure the shaders
        **********************************************************************/
        
        // Set the vertex shader
        ID3D11DeviceContext_VSSetShader(engine.context,
                                        engine.vertexShader, 
                                        NULL, 
                                        0);
        
        // Set the vertex constant buffer to the pipeline
        ID3D11DeviceContext_VSSetConstantBuffers(engine.context,
                                                 0, 
                                                 1, 
                                                 &engine.vertexCBuffer);
        
        // Set the pixel shader
        ID3D11DeviceContext_PSSetShader(engine.context,
                                        engine.pixelShader, 
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
        
        /**********************************************************************
        *** [R7] . Configure the rasterizer
        **********************************************************************/
        
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
        
        /**********************************************************************
        *** [R8] . Configure the output merger
        **********************************************************************/
        
        // Set the depth stencil state
        ID3D11DeviceContext_OMSetDepthStencilState(engine.context,
                                                   engine.dss,
                                                   0);
        
        // Set the blend state
        ID3D11DeviceContext_OMSetBlendState(engine.context,
                                            engine.blendState, 
                                            NULL, 
                                            0xffffffff);
        
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
        float clearColor[4] = {0,0,0,1};
        ID3D11DeviceContext_ClearRenderTargetView(engine.context,
                                                  engine.rtv, 
                                                  clearColor);
        
        ID3D11DeviceContext_ClearDepthStencilView(engine.context,
                                                  engine.dsv, 
                                                  D3D11_CLEAR_DEPTH | 
                                                  D3D11_CLEAR_STENCIL, 
                                                  0, 
                                                  0);
        
        /**********************************************************************
        *** [R9] . Draw and present the frame
        **********************************************************************/
        
        ID3D11DeviceContext_Draw(engine.context,
                                 vertexIndex, 
                                 0);
        
        // Clear the render command vector
        // m_RenderCommands.clear();
        
        IDXGISwapChain_Present(engine.swapChain,
                               1, 
                               0);
        
        // Clear keyboard pressed state from last frame
        for (u32 i = 0; i < array_count(engine.key.all); ++i)
        {
            engine.key.all[i].pressed = false;
        }
        
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