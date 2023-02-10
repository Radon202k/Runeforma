#define STB_IMAGE_IMPLEMENTATION
#include "w:\libs\stb_image.h"

#define SPRITE_ATLAS_SIZE 1024
#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600
#define WINDOW_TITLE "Runeforma"
#define CLEAR_COLOR rgba(.99f,.99f,.99f,1)

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
    
    char fontFileName[260];
    build_absolute_path(fontFileName, 260, "fonts/Nunito-Regular.ttf");
    char *fontName = "Nunito";
    editor.font32 = font_create_from_file(fontFileName, fontName, 32);
    
    for (s32 i = 32; i <= 126; i++)
    {
        editor.glyphs32[i-32] = font_create_glyph(&editor.font32, (char)i);
    }
    
    editor.font16 = font_create_from_file(fontFileName, fontName, 16);
    
    for (s32 i = 32; i <= 126; i++)
    {
        editor.glyphs16[i-32] = font_create_glyph(&editor.font16, (char)i);
    }
    
    
    
    char whitePath[260];
    build_absolute_path(whitePath, 260, "images/white.png");
    
    char debugFontPath[260];
    build_absolute_path(debugFontPath, 260, "images/debugFont.png");
    editor.debugFont = sprite_create_from_file(debugFontPath);
    editor.white = sprite_create_from_file(whitePath);
    
    
    assert(editor.debugFont.exists);
    assert(editor.white.exists);
    
    
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
    
    // Define a size to draw each bucket
    Vector2 bucketSize = v2(13,20);
    
    draw_rect(layer1, editor.white, v2(0, engine.backBufferSize.y - bucketSize.y), v2(engine.backBufferSize.x, bucketSize.y),
              rgba(.7f,.7f,.7f,1), 0);
    
    draw_string(layer1, editor.glyphs32, buffer->bufferName, 
                v2(0, 1.0f*winHeight - bucketSize.y), rgba(.95f,.95f,.95f,1), 1);
    
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
        gap_buffer_draw(&buffer->gapBuffer, buffer->point, buffer->mark,
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
    
    
    Vector2i charLineCount;
    charLineCount.x = buffer->numChars;
    charLineCount.y = buffer->numLines;
    draw_label_v2i(layer2, editor.glyphs16, charLineCount, 
                   v2(engine.backBufferSize.x-60,24), 
                   10, rgba(.5f,.5f,.5f,1), 0, false);
    
}