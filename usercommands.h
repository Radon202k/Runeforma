#pragma once

function void handle_user_navigation()
{
    Buffer *buffer = editor.world.currentBuffer;
    s32 bufferSize = gap_buffer_current_size(&buffer->gapBuffer);
    
    // Handle keyboard input
    if (engine.key.alt.down)
    {
        if (engine.key.up.pressed)
        {
            char sasukePath[260];
            build_absolute_path(sasukePath, 260, "images/sasuke.png");
            
            sasuke = sprite_create(sasukePath);
            assert(sasuke.exists);
            
            u8 *bytes = atlas_get_bytes();
            atlas_update(bytes);
            
            sasukeUploaded = true;
        }
    }
    
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
            buffer_point_set(gap_buffer_current_size(&buffer->gapBuffer));
        }
    }
    
    if (engine.key.left.pressed)
    {
        if (buffer->point > 0)
        {
            if (engine.key.control.down)
            {
                if (!buffer_search_backward(" "))
                {
                    buffer_point_set(0);
                }
            } 
            else
            {
                buffer->point--;
            }
        }
    }
    
    if (engine.key.right.pressed)
    {
        if (buffer->point < bufferSize)
        {
            if (engine.key.control.down)
            {
                if (!buffer_search_forward(" "))
                {
                    buffer_point_set(bufferSize-1);
                }
            }
            else
            {
                buffer->point++;
            }
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
            
            gap_buffer_insert_char(&buffer->gapBuffer, c, &buffer->point);
        }
    }
    
    if (engine.key.backspace.pressed)
    {
        if (buffer->point > 0)
        {
            gap_buffer_delete_char(&buffer->gapBuffer, &buffer->point);
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
            buffer_write();
        }
    }
    
}