#define SPRITE_ATLAS_SIZE 1024
#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600
#define WINDOW_TITLE "Runeforma"

#define STB_IMAGE_IMPLEMENTATION
#include "w:\libs\stb_image.h"

#include "engine2d.h"
#include "datastructures.h"

global Editor editor;

#include "render.h"
#include "gapbuffer.h"
#include "buffer.h"
#include "world.h"

function void init()
{
    editor.font = sprite_create( "images/font.png");
    editor.white = sprite_create( "images/white.png");
    editor.naruto = sprite_create( "images/naruto.png");
    
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
    
    s32 bufferSize = gap_buffer_current_size(&buffer->contents);
    
    // Handle keyboard input
    if (engine.key.left.pressed)
    {
        if (buffer->point > 0)
        {
            buffer->point--;
        }
    }
    
    if (engine.key.right.pressed)
    {
        if (buffer->point < bufferSize)
        {
            buffer->point++;
        }
    }
    
    if (engine.inputCharEntered)
    { 
        char c = engine.inputChar;
        if (c != 8 &&  // backspace
            (c == 10 || c == 13) || // newline / carriage return
            (c >= 32 && c < 127))  
        {
            gap_buffer_insert_char(&buffer->contents, c, &buffer->point);
        }
    }
    
    if (engine.key.backspace.pressed)
    {
        if (buffer->point > 0)
        {
            gap_buffer_delete_char(&buffer->contents, &buffer->point);
        }
    }
    
    if (engine.key.alt.down)
    {
        if (engine.key.f1.pressed)
        {
            editor.showGap = !editor.showGap;
        }
    }
    
    if (engine.key.control.down)
    {
        if (engine.key.s.pressed)
        {
            buffer_save(editor.world.currentBuffer);
        }
    }
    
    // Draw gap buffer buckets and coordinate systems
    
    // Define a size to draw each bucket
    Vector2 bucketSize = v2(15,25);
    
    draw_rect(editor.white, v2(0, engine.backBufferSize.y - bucketSize.y), v2(engine.backBufferSize.x, bucketSize.y),
              rgba(.7f,.7f,.7f,1), 0);
    
    draw_string(buffer->bufferName, v2(0, 1.0f*winHeight - bucketSize.y), bucketSize.x, rgba(0,0,0,1), 1);
    
    // Calculate origin (bottom left) based on buffer size
    Vector2 origin = v2(0, 1.0f*winHeight - 2.0f*bucketSize.y);
    
    if (editor.showGap)
    {
        gap_buffer_draw_with_gap(&buffer->contents, buffer->point, origin, bucketSize);
    }
    else
    {
        gap_buffer_draw(&buffer->contents, buffer->point, origin, bucketSize);
    }
    
    // Search for occurences of the word "to"
    
    s32 *foundPositions, foundCount;
    string_search_naive(buffer->contents.array, 
                        buffer->contents.gapLeft, "to", 2,
                        &foundPositions, &foundCount);
    
    if (foundCount > 0)
    {
        for (s32 i = 0; i < foundCount; ++i)
        {
            s32 loc = foundPositions[i];
            
            Vector2 bucketPos = 
                gap_buffer_point_to_screen_pos(&buffer->contents, loc, 
                                               origin, bucketSize);
            
            draw_rect(editor.white, bucketPos, 
                      v2(bucketSize.x*2, bucketSize.y), rgba(1,0,0,0.5f), 1);
        }
    }
    
    // Has to free the allocated array
    free(foundPositions);
}