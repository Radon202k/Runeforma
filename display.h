#pragma once

#include "render.h"

function void
gap_buffer_draw(GapBuffer *buffer, s32 point, s32 mark,
                Vector2 origin, Vector2 bucketSize)
{
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    SpriteGroup *layer2 = sprite_group_push_layer(2);
    
    s32 bufferSize = gap_buffer_current_size(buffer);
    
    Vector2 bucketPos = v2(origin.x, origin.y);
    
    s32 lineCount = 1;
    
    // Draw the string without the gap
    for (s32 i = 0; i <= bufferSize; ++i)
    {
        char c = buffer_get_char_at(i);
        
        // Draw the bucket of the gap buffer
        Color bucketColor = rgba(1,1,1,1);
        if (i == point)
        {
            bucketColor = rgba(.4f,.4f,1.0f,1);
        }
        else if (c == ' ' || c == '.')
        {
            // bucketColor = rgba(.2f,.2f,.2f,1);
        }
        
#if 0
        draw_rect(layer1, editor.white, bucketPos, bucketSize, bucketColor, 0);
#endif
        
        if (i < bufferSize)
        {
            if (c != 10)   // new line
            {
                draw_char(layer1, editor.font, c, bucketPos, 
                          bucketSize.x, rgba(.2f,.2f,.2f,1), 0);
            }
        }
        
        // Draw the point position
        if (point == i)
        {
            // Base
            draw_rect(layer1, editor.white, v2(bucketPos.x, bucketPos.y-2), 
                      v2(0.5f*bucketSize.x+2, 2), rgba(1,0,0,1), 1);
            
            // Right side
            draw_rect(layer1, editor.white, v2(bucketPos.x, bucketPos.y), 
                      v2(2, bucketSize.y), rgba(1,0,0,1), 1);
            
        }
        
        // Draw the mark position
        if (mark == i)
        {
#if 0
            // Base
            draw_rect(layer1, editor.white, v2(bucketPos.x, bucketPos.y-2), 
                      v2(0.5f*bucketSize.x+2, 2), rgba(0,0,1,1), 1);
            
            // Right side
            draw_rect(layer1, editor.white, v2(bucketPos.x, bucketPos.y), 
                      v2(2, bucketSize.y), rgba(0,0,1,1), 1);
#endif
            
            base_draw_outline_rect(bucketPos.x, bucketPos.y,
                                   bucketSize.x, bucketSize.y,
                                   1,0,0,1,
                                   10);
            
            
        }
        
        
        bucketPos.x += bucketSize.x;
        if (((bucketPos.x + bucketSize.x) > engine.backBufferSize.x) ||
            (c == 10))
        {
            bucketPos.x = origin.x;
            bucketPos.y -= bucketSize.y;
            
            
            draw_label_int(layer2, editor.font, lineCount++, 
                           v2(0, bucketPos.y+2*bucketSize.y), 
                           10, rgba(1,0,0,1), 0, false);
        }
    }
    
    bucketPos.y -= bucketSize.y;
    draw_label_int(layer2, editor.font, lineCount++, 
                   v2(0, bucketPos.y+2*bucketSize.y), 
                   10, rgba(1,0,0,1), 0, false);
    
    // Set the point back
    buffer_point_set(point);
}

function void
gap_buffer_draw_with_gap(GapBuffer *buffer, s32 point,
                         Vector2 origin, Vector2 bucketSize)
{
    // Store local character position to handle automatic line wrap 
    Vector2 bucketPos = v2(origin.x, origin.y);
    
    // Current buffer size 
    s32 bufferSize = gap_buffer_current_size(buffer);
    
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    SpriteGroup *layer2 = sprite_group_push_layer(2);
    
    // Draw each bucket
    for (s32 i = 0; i <= buffer->storageSize; ++i)
    {
        //  Draw the bucket of the gap buffer
        if (i < buffer->storageSize)
        {
            Color bucketColor = rgba(.5f,.5f,.5f,1);
            if (i >= buffer->left && i < buffer->right)
            {
                bucketColor = rgba(1.0f,.4f,.4f,1);
            }
            
            // Make it a different color if the cursor is in this bucket
            if (gap_buffer_user_to_gap_coords(buffer, point) == i)
            {
                bucketColor = rgba(.4f,.4f,1.0f,1);
                
                draw_label_int(layer2, editor.font, 
                               i, v2(bucketPos.x, bucketPos.y+32), 0.7f*bucketSize.x, rgba(0,1,0,1), 1, true);
            }
            
            draw_rect(layer1, editor.white, bucketPos, bucketSize, bucketColor, 0);
            
            // If there is contents in it, draw
            char *c = buffer->storage + i;
            if (c)   // new line
            {
                draw_char(layer1, editor.font,
                          *c, bucketPos, bucketSize.x, rgba(0,0,0,1), 0);
            }
        }
        
        // Draw the left and right positions of the gap 
        if (i == buffer->left || i == (buffer->right))
        {
            draw_label_int(layer2, editor.font,
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
    s32 bufferSize = gap_buffer_current_size(buffer);
    
    Vector2 bucketPos = v2(origin.x, origin.y);
    
    s32 lim = point;
    
    // calculate string position without the gap
    for (s32 i = 0; i <= lim; ++i)
    {
        char c = buffer_get_char_at(i);
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
