#pragma once

#include "render.h"

function void
display_init(void)
{
    // Load fonts
    wchar_t fontFileName[260];
    build_absolute_path(fontFileName, 260, L"fonts/Inconsolata-Regular.ttf");
    wchar_t *fontName = L"Inconsolata";
    editor.font32 = font_create_from_file(fontFileName, fontName, 24);
    editor.font16 = font_create_from_file(fontFileName, fontName, 16);
    
    // Create sprites
    wchar_t whitePath[260];
    build_absolute_path(whitePath, 260, L"images/white.png");
    wchar_t debugFontPath[260];
    build_absolute_path(debugFontPath, 260, L"images/debugFont.png");
    editor.debugFont = sprite_create_from_file(debugFontPath);
    editor.white = sprite_create_from_file(whitePath);
    assert(editor.debugFont.exists);
    assert(editor.white.exists);
}

function float
gap_buffer_content_height(GapBuffer *buffer, float bucketHeight)
{
    float result = 0;
    
    // For every character in the buffer
    float atX = 0;
    for (s32 i = 0; i < gap_buffer_current_length(buffer); ++i)
    {
        wchar_t c = buffer_get_char_at(i);
        
        HashTableNode *node = hash_table_get(&editor.font32.glyphsTable, c, (float)c);
        if (node)
        {
            Sprite *glyphSprite = (Sprite *)node->data;
            if (glyphSprite)
            {
                float glyphWidth = glyphSprite->size.x;
                
                if ((c == '\n') ||
                    ((atX + glyphWidth) > engine.backBufferSize.x))
                {
                    // Advance the height
                    result += bucketHeight;
                    atX = 0;
                }
                
                atX += glyphWidth;
            }
        }
    }
    
    // TODO: Cache this result so we avoid computing it
    // every frame?
    
    return result;
}

function void
gap_buffer_draw(GapBuffer *buffer, s32 firstLine,
                s32 point, s32 mark,
                Vector2 origin, Vector2 bucketSize)
{
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    SpriteGroup *layer2 = sprite_group_push_layer(2);
    
    s32 bufferSize = gap_buffer_current_length(buffer);
    
    Vector2 bucketPos = v2(origin.x, origin.y);
    
    s32 lineCount = 1;
    
    for (s32 i = firstLine; i <= bufferSize; ++i)
    {
        wchar_t c = buffer_get_char_at(i);
        
        Color bucketColor = rgba(1,1,1,1);
        if (i == point)
        {
            bucketColor = rgba(.4f,.4f,1.0f,1);
        }
        else if (c == ' ' || c == '.')
        {
            // bucketColor = rgba(.2f,.2f,.2f,1);
        }
        
        // Draw the actual glyph
        if (i < bufferSize)
        {
            if (c != 10)   // new line
            {
                draw_glyph(layer1, 
                           &editor.font32, 
                           c, 
                           bucketPos, 
                           rgba(.4f,.4f,.4f,1), 
                           0);
            }
        }
        
        // Draw the point
        if (point == i)
        {
            draw_rect(layer2, 
                      editor.white, 
                      v2(bucketPos.x, bucketPos.y-editor.font32.maxDescent), 
                      v2(2, bucketSize.y+editor.font32.maxDescent), 
                      rgba(1,0,0,.5f), 100);
        }
        
        // Draw the mark
        if (mark == i)
        {
            // Right side
            draw_rect(layer1, 
                      editor.white, 
                      v2(bucketPos.x, bucketPos.y-editor.font32.maxDescent), 
                      v2(2, bucketSize.y+editor.font32.maxDescent), 
                      rgba(1,1,1,.5f), 99);
        }
        
        
        // Advance the x pos
        bucketPos.x += bucketSize.x;
        
        // Advance to next line
        if (c == 10)
        {
            bucketPos.x = origin.x;
            bucketPos.y -= bucketSize.y + .25f*editor.font16.lineAdvance;
        }
        
        // If the line cound exceeds the window maximum
        if (bucketPos.y < 0)
        {
            break;
        }
    }
    
    // Set the point back
    buffer_point_set(point);
}

function void
gap_buffer_draw_with_gap(GapBuffer *buffer, s32 point, s32 mark,
                         Vector2 origin, Vector2 bucketSize)
{
    bucketSize.x *= 1.0f;
    bucketSize.y *= 1.0f;
    
    // Store local character position to handle automatic line wrap 
    Vector2 bucketPos = v2(origin.x, origin.y);
    
    // Current buffer size 
    s32 bufferSize = gap_buffer_current_length(buffer);
    
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    SpriteGroup *layer2 = sprite_group_push_layer(2);
    
    // Draw each bucket
    for (s32 i = 0; i <= buffer->storageLength; ++i)
    {
        //  Draw the bucket of the gap buffer
        if (i < buffer->storageLength)
        {
            Color bucketColor = rgba(.5f,.5f,.5f,1);
            if (i >= buffer->left && i < buffer->right)
            {
                bucketColor = rgba(1.0f,.4f,.4f,1);
            }
            
            // Make it a different color if the mark is in this bucket
            if (gap_buffer_user_to_gap_coords(buffer, mark) == i)
            {
                bucketColor = rgba(1.0f,1.0f,0.4f,1);
                
                draw_label_int(layer2, &editor.font16, 
                               i, v2(bucketPos.x, bucketPos.y+32), 0.7f*bucketSize.x, 
                               bucketColor, 1, true);
            }
            
            
            // Make it a different color if the cursor is in this bucket
            if (gap_buffer_user_to_gap_coords(buffer, point) == i)
            {
                bucketColor = rgba(.4f,.4f,1.0f,1);
                
                draw_label_int(layer2, &editor.font16, 
                               i, v2(bucketPos.x, bucketPos.y+32), 0.7f*bucketSize.x, rgba(0,1,0,1), 1, true);
            }
            
            draw_rect(layer1, editor.white, bucketPos, bucketSize, bucketColor, 0);
            
            // If there is contents in it, draw
            wchar_t *c = buffer->storage + i;
            if (c)   // new line
            {
                draw_glyph(layer1, &editor.font16, *c, bucketPos, rgba(0,0,0,1), 0);
            }
        }
        
        // Draw the left and right positions of the gap 
        if (i == buffer->left || i == (buffer->right))
        {
            draw_label_int(layer2, &editor.font16,
                           i, bucketPos, 0.7f*bucketSize.x, rgba(1,1,0,1), 1, true);
        }
        
        // Advance the width of the bucket for the position of next one start at the end of this one
        bucketPos.x += (bucketSize.x + 1);
        
        if ((bucketPos.x + bucketSize.x) > engine.backBufferSize.x)
        {
            bucketPos.x = origin.x;
            bucketPos.y -= (bucketSize.y + 1);
        }
    }
}

function Vector2
gap_buffer_point_to_screen_pos(GapBuffer *buffer, s32 point, 
                               Vector2 origin, Vector2 bucketSize)
{
    s32 bufferSize = gap_buffer_current_length(buffer);
    
    Vector2 bucketPos = v2(origin.x, origin.y);
    
    s32 lim = point;
    
    // calculate string position without the gap
    for (s32 i = 0; i <= lim; ++i)
    {
        wchar_t c = buffer_get_char_at(i);
        bucketPos.x += bucketSize.x;
        if (((bucketPos.x + bucketSize.x) > engine.backBufferSize.x) ||
            (c == 10))
        {
            bucketPos.x = origin.x;
            bucketPos.y -= bucketSize.y;
        }
    }
    
    return bucketPos;
}


function void
display_render_frame(void)
{
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    SpriteGroup *layer2 = sprite_group_push_layer(2);
    SpriteGroup *layer3 = sprite_group_push_layer(3);
    SpriteGroup *layer4 = sprite_group_push_layer(4);
    
    Buffer *buffer = editor.world.currentBuffer;
    
    float winWidth = engine.backBufferSize.x;
    float winHeight = engine.backBufferSize.y;
    
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
    
    
    if (editor.sasukeUploaded)
    {
        draw_rect(layer2, editor.sasuke, v2(300,0), v2(200,200), rgba(1,1,1,.8f), 0);
    }
    
    
    
    // draw dt label
    draw_label_float(layer1, &editor.font32,
                     engine.dt, 
                     v2(engine.backBufferSize.x-100,32), 
                     1, rgba(1,1,0,1), 1, false);
    
}
