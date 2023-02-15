#pragma once

#include "render.h"

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
