#define STB_IMAGE_IMPLEMENTATION
#include "w:\libs\stb_image.h"

#define SPRITE_ATLAS_SIZE 1024
#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600
#define WINDOW_TITLE "Runeforma"
#define CLEAR_COLOR rgba(.95f,.95f,.95f,1)

#include "engine2d.h"

#include "string.h"
#include "subeditor.h"
#include "display.h"
#include "usercommands.h"

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
    
    if (engine.key.up.pressed)
    {
        if (!buffer_search_backward("\n"))
        {
            buffer_point_set(0);
        }
    }
    
    if (engine.key.down.pressed)
    {
        if (!buffer_search_forward("\n"))
        {
            buffer_point_set(gap_buffer_current_size(&buffer->contents));
        }
    }
    
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
            if (c == 13 || c == 10)
            {
                c = 10;
                buffer->numLines++;
            }
            else
            {
                buffer->numChars++;
            }
            
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
    
    RenderGroup *layer1 = render_group_push_layer(1);
    RenderGroup *layer2 = render_group_push_layer(2);
    
    // Define a size to draw each bucket
    Vector2 bucketSize = v2(15,17);
    
    draw_rect(layer1, editor.white, v2(0, engine.backBufferSize.y - bucketSize.y), v2(engine.backBufferSize.x, bucketSize.y),
              rgba(.7f,.7f,.7f,1), 0);
    
    draw_string(layer1, buffer->bufferName, v2(0, 1.0f*winHeight - bucketSize.y), bucketSize.x, rgba(.95f,.95f,.95f,1), 1);
    
    // Calculate origin (bottom left) based on buffer size
    Vector2 origin = v2(0, 1.0f*winHeight - 2.0f*bucketSize.y);
    
    origin.x += 2*bucketSize.x;
    origin.y -= bucketSize.x;
    
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
            
            draw_rect(layer1, editor.white, bucketPos, 
                      v2(bucketSize.x*2, bucketSize.y), rgba(1,0,0,0.5f), 1);
        }
    }
    
    Vector2i charLineCount;
    charLineCount.x = buffer->numChars;
    charLineCount.y = buffer->numLines;
    draw_label_v2i(layer2, charLineCount, v2(0,20), 
                   10, rgba(.5f,.5f,.5f,1), 0, false);
    
    // Has to free the allocated array
    free(foundPositions);
}