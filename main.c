#define STB_IMAGE_IMPLEMENTATION
#include "w:\libs\stb_image.h"

#define SPRITE_ATLAS_SIZE 1024
#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600
#define WINDOW_TITLE "Runeforma"
#define CLEAR_COLOR rgba(.95f,.95f,.95f,1)

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
    char narutoPath[260];
    build_absolute_path(narutoPath, 260, "images/naruto.png");
    
    char whitePath[260];
    build_absolute_path(whitePath, 260, "images/white.png");
    
    char fontPath[260];
    build_absolute_path(fontPath, 260, "images/font.png");
    
    editor.font = sprite_create(fontPath);
    editor.white = sprite_create(whitePath);
    editor.naruto = sprite_create(narutoPath);
    
    assert(editor.font.exists);
    assert(editor.white.exists);
    assert(editor.naruto.exists);
    
    // Tests
    test_gap_buffer_insert_char();
    
    // Init
    world_init(&editor.world);
}

function void update()
{
    float winWidth = engine.backBufferSize.x;
    float winHeight = engine.backBufferSize.y;
    
    Buffer *buffer = editor.world.currentBuffer;
    s32 bufferSize = gap_buffer_current_size(&buffer->gapBuffer);
    
    handle_user_navigation();
    
    // Draw gap buffer buckets and coordinate systems
    
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    SpriteGroup *layer2 = sprite_group_push_layer(2);
    
    if (sasukeUploaded)
    {
        draw_rect(layer2, sasuke, v2(300,0), v2(200,200), rgba(1,1,1,1), 0);
    }
    
    base_draw_line(0, 0, 
                   engine.backBufferSize.x, engine.backBufferSize.y,
                   1, 0, 1, 1,
                   100);
    
    
    base_draw_line(0, 0, 
                   engine.mouse.pos.x, engine.mouse.pos.y,
                   1, 0, 1, 1,
                   100);
    
    
    // Define a size to draw each bucket
    Vector2 bucketSize = v2(15,17);
    
    draw_rect(layer1, editor.white, v2(0, engine.backBufferSize.y - bucketSize.y), v2(engine.backBufferSize.x, bucketSize.y),
              rgba(.7f,.7f,.7f,1), 0);
    
    draw_string(layer1, editor.font,
                buffer->bufferName, v2(0, 1.0f*winHeight - bucketSize.y), bucketSize.x, rgba(.95f,.95f,.95f,1), 1);
    
    // Calculate origin (bottom left) based on buffer size
    Vector2 origin = v2(0, 1.0f*winHeight - 2.0f*bucketSize.y);
    
    origin.x += 2*bucketSize.x;
    origin.y -= bucketSize.x;
    
    if (editor.showGap)
    {
        gap_buffer_draw_with_gap(&buffer->gapBuffer, buffer->point, origin, bucketSize);
    }
    else
    {
        gap_buffer_draw(&buffer->gapBuffer, buffer->point, origin, bucketSize);
    }
    
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
    
    Vector2i charLineCount;
    charLineCount.x = buffer->numChars;
    charLineCount.y = buffer->numLines;
    draw_label_v2i(layer2, editor.font, charLineCount, v2(0,20), 
                   10, rgba(.5f,.5f,.5f,1), 0, false);
    
    // Has to free the allocated array
    free(foundPositions);
}