#define STB_IMAGE_IMPLEMENTATION
#include "w:\libs\stb_image.h"

#define SPRITE_ATLAS_SIZE 1024
#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600
#define WINDOW_TITLE L"Runeforma"
#define CLEAR_COLOR rgba(.099f,.099f,.099f,1)

#include "engine2d.h"

global bool sasukeUploaded;
global Sprite sasuke;
global TCHAR exePath[MAX_PATH];

#include "string.h"
#include "subeditor.h"
#include "display.h"
#include "usercommands.h"


function void init()
{
    int maxWidth = 1024;
    
    
    wchar_t fontFileName[260];
    build_absolute_path(fontFileName, 260, L"fonts/Inconsolata-Regular.ttf");
    wchar_t *fontName = L"Inconsolata";
    editor.font32 = font_create_from_file(fontFileName, fontName, 24);
    editor.font16 = font_create_from_file(fontFileName, fontName, 16);
    
    
    wchar_t whitePath[260];
    build_absolute_path(whitePath, 260, L"images/white.png");
    
    wchar_t debugFontPath[260];
    build_absolute_path(debugFontPath, 260, L"images/debugFont.png");
    editor.debugFont = sprite_create_from_file(debugFontPath);
    editor.white = sprite_create_from_file(whitePath);
    
    
    assert(editor.debugFont.exists);
    assert(editor.white.exists);
    
    
    // Init
    world_init(&editor.world);
    
    editor.scrollBarPoint = 1;
}

function void update()
{
    float winWidth = engine.backBufferSize.x;
    float winHeight = engine.backBufferSize.y;
    
    Buffer *buffer = editor.world.currentBuffer;
    s32 bufferSize = gap_buffer_current_length(&buffer->gapBuffer);
    
    handle_user_navigation();
    
    // Draw gap buffer buckets and coordinate systems
    
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    SpriteGroup *layer2 = sprite_group_push_layer(2);
    SpriteGroup *layer3 = sprite_group_push_layer(3);
    SpriteGroup *layer4 = sprite_group_push_layer(4);
    
    if (sasukeUploaded)
    {
        draw_rect(layer2, sasuke, v2(300,0), v2(200,200), rgba(1,1,1,.8f), 0);
    }
    
    // Define a size to draw each bucket
    Vector2 bucketSize = v2(editor.font32.charWidth, editor.font32.lineAdvance);
    
    // Draw buffer name
    float bufferNameWidth = string_length(buffer->bufferName)*bucketSize.x;
    draw_string(layer4, &editor.font32, 
                buffer->bufferName, 
                v2(engine.backBufferSize.x-bufferNameWidth-10, 
                   1.0f*winHeight-1.2f*bucketSize.y), 
                rgba(.5f,.5f,.5f,1), 11);
    
    // Calculate origin (bottom left) based on buffer size
    Vector2 origin = v2(0, winHeight - 1.0f*bucketSize.y);
    
    origin.x += 2*bucketSize.x;
    origin.y -= bucketSize.x;
    
    if (editor.showGap)
    {
        gap_buffer_draw_with_gap(&buffer->gapBuffer, buffer->point, buffer->mark,
                                 origin, bucketSize);
    }
    else
    {
        float contentH = gap_buffer_content_height(&buffer->gapBuffer, bucketSize.y);
        
        // Draw the text
        gap_buffer_draw(&buffer->gapBuffer, buffer->firstLine, 
                        buffer->point, buffer->mark,
                        origin, bucketSize);
    }
    
#if 0
    // TODO: Update to handle variable char widths
    // Search for occurences of the word "to"
    s32 *foundPositions, foundCount;
    string_search_naive(buffer->gapBuffer.storage, 
                        buffer->gapBuffer.left, "to", 2,
                        &foundPositions, &foundCount);
    
    if (foundCount > 0)
    {
        for (s32 i = 0; i < foundCount; ++i)
        {
            s32 loc = foundPositions[i];
            
            Vector2 bucketPos = 
                gap_buffer_point_to_screen_pos(&buffer->gapBuffer, loc, 
                                               origin, bucketSize);
            
            draw_rect(layer1, editor.white, bucketPos, 
                      v2(bucketSize.x*2, bucketSize.y), rgba(1,0,0,0.5f), 1);
        }
    }
    
    
    // Has to free the allocated array
    free(foundPositions);
    
#endif
    
}