#pragma once

function void
command_animate_origin(Vector2 oldOrigin, Vector2 newOrigin)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    animator_set_v2(&editor.originAnimator, oldOrigin, newOrigin);
    animator_play(&editor.originAnimator);
}

function void
command_animate_point(s32 oldPoint)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    Vector2 lastPointPos = 
        gap_buffer_point_to_screen_pos(&buffer->gapBuffer, 
                                       buffer->firstLineCharP, 
                                       oldPoint, 
                                       world->origin, 
                                       world->bucketSize);
    
    Vector2 pointPos = 
        gap_buffer_point_to_screen_pos(&buffer->gapBuffer, 
                                       buffer->firstLineCharP, 
                                       buffer->point, 
                                       world->origin, 
                                       world->bucketSize);
    
    animator_set_v2(&editor.pointPosAnimator, lastPointPos, pointPos);
    animator_play(&editor.pointPosAnimator);
}

function void
command_go_to_next_line(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    wchar_t *searchStrings[1] = {L"\n"};
    s32 oldPoint = buffer->point;
    if (!buffer_search_forward(searchStrings, array_count(searchStrings)))
    {
        //
    }
    command_animate_point(oldPoint);
}

function void
command_first_line_go_to_next(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    // Save the point
    s32 oldPoint = buffer->point;
    
    Vector2 origin = world->origin;
    
    // Set the point where the first line currently is
    buffer_point_set(buffer->firstLineCharP);
    
    // Search the first line break from there
    bool found = true;
    wchar_t *searchStrings[1] = {L"\n"}; 
    if (!buffer_search_forward(searchStrings, array_count(searchStrings)))
    {
        // If couldn't find any, flag to do nothing
        found = false;
    }
    
    if (found)
    {
        // Set new first line with one past the found break line
        buffer->firstLineCharP = buffer->point+1;
    }
    
    // Restore the point
    buffer_point_set(oldPoint);
    
    command_animate_origin(v2_sub(origin, v2(0,world->bucketSize.y)),origin);
}

function void
command_first_line_go_to_previous(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    // Save the point
    s32 oldPoint = buffer->point;
    
    Vector2 origin = world->origin;
    
    // Set the point where the first line currently is
    buffer_point_set(buffer->firstLineCharP-1);
    
    // Search the first line break from there
    wchar_t *searchStrings[1] = {L"\n"};
    if (!buffer_search_backward(searchStrings, array_count(searchStrings)))
    {
        // If couldn't find any, set to the start of the buffer
        buffer_point_set(0);
    }
    
    if (buffer->point > 0)
    {
        // set new first line with found break line
        buffer->firstLineCharP = buffer->point+1;
    }
    else
    {
        buffer->firstLineCharP = 0;
    }
    
    // Restore the point
    buffer_point_set(oldPoint);
    
    command_animate_origin(v2_add(origin,v2(0,world->bucketSize.y)),origin);
}

function void
command_copy_range(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
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

function void
command_paste_range(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
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

function void
command_delete_range(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
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

function void
command_insert_char(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
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

function void
command_backspace(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
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

function void
command_go_to_previous_space_or_linebreak(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    if (buffer->point > 0)
    {
        s32 oldPoint = buffer->point;
        
        wchar_t *searchStrings[2] = {L" ", L"\n"};
        if (!buffer_search_backward(searchStrings, array_count(searchStrings)))
        {
            buffer_point_set(0);
        }
        
        command_animate_point(oldPoint);
    }
}

function void
command_go_to_previous_char(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    if (buffer->point > 0)
    {
        s32 oldPoint = buffer->point;
        buffer->point--;
        command_animate_point(oldPoint);
    }
}


function void
command_go_to_next_space_or_linebreak(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    s32 bufferSize = gap_buffer_length(&buffer->gapBuffer);
    if (buffer->point < bufferSize)
    {
        s32 oldPoint = buffer->point;
        
        wchar_t *searchStrings[2] = {L" ", L"\n"}; 
        if (!buffer_search_forward(searchStrings, array_count(searchStrings)))
        {
            buffer_point_set(bufferSize);
        }
        
        command_animate_point(oldPoint);
    }
}

function void
command_go_to_next_char(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    s32 bufferSize = gap_buffer_length(&buffer->gapBuffer);
    if (buffer->point < bufferSize)
    {
        s32 oldPoint = buffer->point;
        buffer->point++;
        command_animate_point(oldPoint);
    }
}