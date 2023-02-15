#pragma once

// Helper function to copy text to the clipboard
function void 
copy_text_to_clipboard(wchar_t *text)
{
    HGLOBAL globalMemory = GlobalAlloc(GMEM_MOVEABLE, (string_length(text)+1) * sizeof(wchar_t));
    
    wchar_t *data = (wchar_t *)GlobalLock(globalMemory);
    
    u32 copiedLength = string_length(text)+1;
    
    string_copy(data, copiedLength, text);
    
    assert(copiedLength < 1000);
    
    GlobalUnlock(globalMemory);
    
    if (OpenClipboard(NULL))
    {
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, globalMemory);
        CloseClipboard();
    }
    
    GlobalFree(globalMemory);
}

// Helper function to paste text from the clipboard
function s32 
paste_text_from_clipboard(wchar_t *text, int maxLength)
{
    s32 pastedLength = 0;
    
    if (OpenClipboard(NULL))
    {
        HGLOBAL globalMemory = GetClipboardData(CF_UNICODETEXT);
        if (globalMemory != NULL)
        {
            wchar_t *data = (wchar_t *)GlobalLock(globalMemory);
            pastedLength = string_length(data);
            
            assert(pastedLength < 1000);
            
            string_copy_size(text, maxLength*sizeof(wchar_t), data, pastedLength);
            GlobalUnlock(globalMemory);
        }
        
        CloseClipboard();
    }
    
    return pastedLength;
}

function void
handle_user_navigation()
{
    Buffer *buffer = editor.world.currentBuffer;
    s32 bufferSize = gap_buffer_current_length(&buffer->gapBuffer);
    
    // Scroll bar input
    if (engine.mouse.wheel != 0)
    {
        editor.scrollBarPoint += 0.016f*engine.mouse.wheel;
    }
    
    
    // If control is down
    if (engine.key.control.down)
    {
        // If the t key is pressed
        if (engine.key.down.pressed)
        {
            // Save the point
            s32 oldPoint = buffer->point;
            
            // Set the point where the first line currently is
            buffer_point_set(buffer->firstLine);
            
            // Search the first line break from there
            bool found = true;
            if (!buffer_search_forward(L"\n"))
            {
                // If couldn't find any, flag to do nothing
                found = false;
            }
            
            if (found)
            {
                // Set new first line with one past the found break line
                buffer->firstLine = buffer->point+1;
            }
            
            // Restore the point
            buffer_point_set(oldPoint);
        }
        
        // If the t key is pressed
        if (engine.key.up.pressed)
        {
            // Save the point
            s32 oldPoint = buffer->point;
            
            // Set the point where the first line currently is
            buffer_point_set(buffer->firstLine-1);
            
            // Search the first line break from there
            if (!buffer_search_backward(L"\n"))
            {
                // If couldn't find any, set to the start of the buffer
                buffer_point_set(0);
            }
            
            if (buffer->point > 0)
            {
                // set new first line with found break line
                buffer->firstLine = buffer->point+1;
            }
            else
            {
                buffer->firstLine = 0;
            }
            
            // Restore the point
            buffer_point_set(oldPoint);
        }
        
        
        // If the c key is pressed
        if (engine.key.c.pressed)
        {
            s32 maxLength = 1024*1024;
            wchar_t *copiedText = alloc_array(maxLength, wchar_t);
            
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
                                                  copiedText, maxLength,
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
            s32 maxLength = 1024*1024;
            wchar_t *pastedText = alloc_array(maxLength, wchar_t);
            s32 pastedLength = paste_text_from_clipboard(pastedText, maxLength);
            
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
            wchar_t c = engine.inputChar;
            if (c != 8 &&  // backspace
                (c == 10 || c == 13) || // newline / carriage return
                (c >= 32 && c < 256))  
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
                                         L"  ", 
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
            wchar_t sasukePath[260];
            build_absolute_path(sasukePath, 260, L"images/sasuke.png");
            
            sasuke = sprite_create_from_file(sasukePath);
            assert(sasuke.exists);
            
            u8 *bytes = atlas_get_bytes();
            atlas_update(bytes);
            
            sasukeUploaded = true;
        }
    }
    
    if (engine.key.up.pressed)
    {
        if (!buffer_search_backward(L"\n"))
        {
            buffer_point_set(0);
        }
    }
    
    if (engine.key.down.pressed)
    {
        if (!buffer_search_forward(L"\n"))
        {
            buffer_point_set(gap_buffer_current_length(&buffer->gapBuffer));
        }
    }
    
    if (engine.key.left.pressed)
    {
        if (buffer->point > 0)
        {
            if (engine.key.control.down)
            {
                if (!buffer_search_backward(L" "))
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
                if (!buffer_search_forward(L" "))
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