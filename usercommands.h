#pragma once

// Helper function to copy text to the clipboard
function void copy_text_to_clipboard(char *text)
{
    HGLOBAL globalMemory = GlobalAlloc(GMEM_MOVEABLE, (strlen(text) + 1) * sizeof(char));
    char *data = (char *)GlobalLock(globalMemory);
    strcpy_s(data, strnlen_s(text, INT_MAX)+1, text);
    GlobalUnlock(globalMemory);
    
    if (OpenClipboard(NULL))
    {
        EmptyClipboard();
        SetClipboardData(CF_TEXT, globalMemory);
        CloseClipboard();
    }
    
    GlobalFree(globalMemory);
}

// Helper function to paste text from the clipboard
function s32 paste_text_from_clipboard(char *text, int maxLength)
{
    s32 pastedSize = 0;
    
    if (OpenClipboard(NULL))
    {
        HGLOBAL globalMemory = GetClipboardData(CF_TEXT);
        if (globalMemory != NULL)
        {
            char *data = (char *)GlobalLock(globalMemory);
            pastedSize = (s32)min(strnlen_s(data, INT_MAX)+1, maxLength-1);
            strncpy_s(text, maxLength, data, pastedSize);
            GlobalUnlock(globalMemory);
        }
        
        CloseClipboard();
    }
    
    return pastedSize;
}

function void handle_user_navigation()
{
    Buffer *buffer = editor.world.currentBuffer;
    s32 bufferSize = gap_buffer_current_size(&buffer->gapBuffer);
    
    // Handle keyboard input
    
    // If contorl is down
    if (engine.key.control.down)
    {
        // If the c key is pressed
        if (engine.key.c.pressed)
        {
            s32 maxSize = sizeof(char)*1024*1024;
            char *copiedText = (char *)malloc(maxSize);
            
            bool swapedPointAndMark = false;
            // If the point is after the mark
            if (buffer->point > buffer->mark)
            {
                // Secretly reverse them right before copying the range
                s32 temp = buffer->point;
                buffer->point = buffer->mark;
                buffer->mark = temp;
                swapedPointAndMark = true;
            }
            
            s32 copiedSize = gap_buffer_get_range(&buffer->gapBuffer, 
                                                  copiedText, maxSize,
                                                  buffer->point, buffer->mark);
            
            // If swapped point and mark
            if (swapedPointAndMark)
            {
                // Secretly swap them back (nobody saw it :P)
                s32 temp = buffer->point;
                buffer->point = buffer->mark;
                buffer->mark = temp;
            }
            
            copy_text_to_clipboard(copiedText);
        }
        // Else if the v key is pressed
        else if (engine.key.v.pressed)
        {
            s32 maxSize = sizeof(char)*1024*1024;
            char *pastedText = (char *)malloc(maxSize);
            s32 pastedSize = paste_text_from_clipboard(pastedText, maxSize);
            
            s32 len = gap_buffer_insert_string(&buffer->gapBuffer, 
                                               pastedText, 
                                               buffer->point);
            
            free(pastedText);
            
            // Advance the point len worth
            buffer->point += len;
        }
        
        // If d key was pressed
        if (engine.key.d.pressed)
        {
            if (buffer->point != buffer->mark)
            {
                // If the point is after the mark
                if (buffer->point > buffer->mark)
                {
                    // Secretly reverse them right before deleting the range
                    s32 temp = buffer->point;
                    buffer->point = buffer->mark;
                    buffer->mark = temp;
                }
                
                // Delete the range between the point and the mark
                gap_buffer_delete_range(&buffer->gapBuffer, buffer->point, buffer->mark);
                
                // Move the mark to the point
                buffer->mark = buffer->point;
            }
        }
        
        // If space was pressed
        if (engine.key.space.pressed)
        {
            buffer->mark = buffer->point;
        }
        
        // If s key was pressed
        if (engine.key.s.pressed)
        {
            buffer_write();
        }
    }
    // If control is not down
    else
    {
        // If char was inputed
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
                
                // Insert the char into the gap buffer
                gap_buffer_insert_char(&buffer->gapBuffer, c, buffer->point);
                
                // Increment the point
                buffer->point++;
                
                // Increment the mark if it is after the point
                if (buffer->mark > buffer->point)
                {
                    buffer->mark++;
                }
            }
            else if(c == 9) // tab
            {
                gap_buffer_insert_string(&buffer->gapBuffer, 
                                         "  ", 
                                         buffer->point);
                buffer->point += 2;
            }
        }
        
        // If delete key was pressed
        if (engine.key.backspace.pressed)
        {
            if (buffer->point > 0)
            {
                // Delete key from gap buffer
                gap_buffer_delete_char(&buffer->gapBuffer, buffer->point);
                
                // Decrement the point
                buffer->point--;
                
                // Decrement the mark if it is after the point
                if (buffer->mark > buffer->point)
                {
                    buffer->mark--;
                }
            }
        }
    }
    
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
    
    if (engine.key.alt.down)
    {
        if (engine.key.f1.pressed)
        {
            editor.showGap = !editor.showGap;
        }
    }
}