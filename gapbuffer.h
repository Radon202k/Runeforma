#pragma once

function void
gap_buffer_init(GapBuffer *buffer, s32 size, char *contents, s32 *point)
{
    buffer->arraySize = size;
    buffer->array = (char *)malloc(buffer->arraySize);
    memset(buffer->array, 0, buffer->arraySize);
    
    s32 length = (s32)strlen(contents);
    strcpy_s(buffer->array, buffer->arraySize, contents);
    
    buffer->gapLeft = length;
    buffer->gapRight = buffer->arraySize;
    
    *point = length;
}

function s32
gap_buffer_current_size(GapBuffer *buffer)
{
    assert(buffer->gapRight >= buffer->gapLeft);
    s32 gapSize = (buffer->gapRight - buffer->gapLeft);
    s32 result = buffer->arraySize - gapSize;
    return result;
}

function s32
gap_buffer_user_to_gap_coords(GapBuffer *buffer, s32 userPoint)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    assert(userPoint >= 0 && userPoint <= bufferSize);
    
    // If the point is before the start of the gap, the user coords
    // are the same as the internal coords
    s32 result = userPoint;
    
    // But if the point is equal or greater than the start of the gap
    // then the internal coord is equal to the user coord + gap size
    if (userPoint >= buffer->gapLeft)
    {
        result += (buffer->gapRight - buffer->gapLeft);
    }
    
    return result;
}

function void
gap_buffer_move_gap_to_point(GapBuffer *buffer, s32 *point)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    assert(*point >= 0 && *point <= bufferSize);
    
    // If the gap is in the right position
    if (buffer->gapLeft == *point)
    {
        // Do nothing
    }
    // If the gap is before the point
    else if (buffer->gapLeft < *point)
    {
        // The gap must be moved to the point. 
        // The characters **after the gap** but **before the point** must be moved
        
        // Calculate how many characters we will shift
        s32 charShiftCount = *point - buffer->gapLeft;
        
        // The size of the gap
        s32 gapSize = buffer->gapRight - buffer->gapLeft;
        
        // Begin at the start of the gap, go to the last one before the point
        for (s32 i = buffer->gapLeft; i < *point; ++i)
        {
            // Shift the character
            buffer->array[i] = buffer->array[i + gapSize];
            // Clear the other one for clarity while developing
            // TODO: Optimize
            buffer->array[i + gapSize] = 0;
        }
        
        // Move the gap "pointers"
        buffer->gapLeft += charShiftCount;
        buffer->gapRight += charShiftCount;
    }
    // If the gap is after the point
    else if (buffer->gapLeft > *point)
    {
        // The gap must be moved to the point. 
        // The characters **after the point** but **before the gap** must be moved
        
        // The amount of chars to shift
        s32 charShiftCount = buffer->gapLeft - *point;
        
        // Size of gap
        s32 gapSize = buffer->gapRight - buffer->gapLeft;
        
        // Start at the character just before the end of the gap
        // Continue until i - gapSize is lower than the point
        for (s32 i = buffer->gapRight-1; (i - gapSize) >= *point; --i)
        {
            // Shift the character
            buffer->array[i] = buffer->array[i - gapSize];
            buffer->array[i - gapSize] = 0;
        }
        
        // Move the gap "pointers"
        buffer->gapLeft -= charShiftCount;
        buffer->gapRight -= charShiftCount;
    }
}

function void
gap_buffer_insert_char(GapBuffer *buffer, char c, s32 *point)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    assert(*point >= 0 && *point <= bufferSize);
    
    // If there is no space left in the gap
    if (bufferSize == buffer->arraySize)
    {
        s32 oldArraySize = buffer->arraySize;
        s32 newArraySize = 2*oldArraySize;
        buffer->arraySize = newArraySize;
        
        // Alloc new array
        char *newArray = (char *)malloc(newArraySize);
        
        // Clear it to zero for easy memory visualization while developing
        // TODO: Optimize
        memset(newArray, 0, newArraySize);
        
        // Copy the contents
        memcpy(newArray, buffer->array, oldArraySize);
        
        // Free old array
        free(buffer->array);
        
        // Set the new pointer
        buffer->array = newArray;
        
        // Update the arraySize buffer variable
        buffer->arraySize = newArraySize;
        
        // Set the gap to be from the end of last size until the end of new size
        buffer->gapLeft = oldArraySize;
        buffer->gapRight = newArraySize;
    }
    
    gap_buffer_move_gap_to_point(buffer, point);
    
    // Increase the gap start pointer (reduce the gap size)
    buffer->gapLeft++;
    
    // Insert the character in the point position
    buffer->array[*point] = c;
    
    // Move the point one to the right
    (*point) = (*point) + 1;
}

function void
gap_buffer_delete_char(GapBuffer *buffer, s32 *point)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    assert(*point > 0 && *point <= bufferSize);
    
    gap_buffer_move_gap_to_point(buffer, point);
    
    // Move the point one to the left
    (*point) = (*point) - 1;
    
    // Clear the position for clarity while developing
    // TODO: Optimize
    buffer->array[*point] = 0;
    
    // Decrease the gap start pointer (reduce the gap size)
    buffer->gapLeft--;
}

function void
test_gap_buffer_insert_char()
{
    // Test case 1: inserting at the beginning of the buffer
    GapBuffer buffer1 = {
        .gapLeft = 0,
        .gapRight = 5,
        .arraySize = 10,
        .array = "#####67890"
    };
    
    s32 point = 0;
    gap_buffer_insert_char(&buffer1, 'A', &point);
    assert(buffer1.gapLeft == 1);
    assert(buffer1.gapRight == 5);
    assert(strcmp(buffer1.array, "A####67890") == 0);
    assert(point == 1);
    
    // Test case 2: inserting at the end of the buffer
    GapBuffer buffer2 = {
        .gapLeft = 5,
        .gapRight = 10,
        .arraySize = 10,
        .array = "12345#####"
    };
    point = 5;
    gap_buffer_insert_char(&buffer2, 'Z', &point);
    assert(buffer2.gapLeft == 6);
    assert(buffer2.gapRight == 10);
    assert(strcmp(buffer2.array, "12345Z####") == 0);
    assert(point == 6);
    
    // Add more test cases as needed
}

function void
gap_buffer_draw_with_gap(GapBuffer *buffer, s32 point,
                         Vector2 origin, Vector2 bucketSize)
{
    // Store local character position to handle automatic line wrap 
    Vector2 bucketPos = v2(origin.x, origin.y);
    
    // Draw each bucket
    for (s32 i = 0; i <= buffer->arraySize; ++i)
    {
        //  Draw the bucket of the gap buffer
        if (i < buffer->arraySize)
        {
            Color bucketColor = rgba(.5f,.5f,.5f,1);
            if (i >= buffer->gapLeft && i < buffer->gapRight)
            {
                bucketColor = rgba(1.0f,.4f,.4f,1);
            }
            
            // Make it a different color if the cursor is in this bucket
            if (gap_buffer_user_to_gap_coords(buffer, point) == i)
            {
                bucketColor = rgba(.4f,.4f,1.0f,1);
            }
            
            draw_rect(editor.white, bucketPos, bucketSize, bucketColor, 0);
            
            // If there is contents in it, draw
            char *c = buffer->array + i;
            if (c && 
                *c != 10 && // carriage return
                *c != 13)   // new line
            {
                draw_char(*c, bucketPos, bucketSize.x, rgba(0,0,0,1), 0);
            }
        }
        
        // Draw the left and right positions of the gap 
        if (i == buffer->gapLeft || i == (buffer->gapRight))
        {
            // Get a string for the number using _itoa_s
            char number[32] = {0};
            _itoa_s(i, number, 32, 10);
            
            // Claculate the char size and position
            Vector2 charSize = bucketSize;
            Vector2 charPos = bucketPos;
            
            charPos.x -= strlen(number) * 0.5f*charSize.x;
            
            draw_string(number, v2(charPos.x, charPos.y - 16), charSize.x, rgba(1,1,0,1), 1);
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
        // If there is contents in it, draw
        s32 gapI = gap_buffer_user_to_gap_coords(buffer, i);
        
        char *c = 0;
        if (gapI < buffer->arraySize)
        {
            c = buffer->array + gapI;
        }
        
        if (c && *c == 13) // Carriage return 
        {
            // ignore
        }
        else
        {
            bucketPos.x += bucketSize.x;
            if (((bucketPos.x + bucketSize.x) > engine.backBufferSize.x) ||
                (c && *c == 10))
            {
                bucketPos.x = 0;
                bucketPos.y -= bucketSize.y;
            }
        }
        
    }
    
    return bucketPos;
}

function void
gap_buffer_draw(GapBuffer *buffer, s32 point, 
                Vector2 origin, Vector2 bucketSize)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    
    Vector2 bucketPos = v2(origin.x, origin.y);
    
    // Draw the string without the gap
    for (s32 i = 0; i <= bufferSize; ++i)
    {
        // If there is contents in it, draw
        s32 gapI = gap_buffer_user_to_gap_coords(buffer, i);
        
        char *c = 0;
        if (gapI < buffer->arraySize)
        {
            c = buffer->array + gapI;
            
            if (c && 
                *c != 10 && // carriage return
                *c != 13)   // new line
            {
                // Draw the bucket of the gap buffer
                Color bucketColor = rgba(.1f,.1f,.1f,1);
                if (i == point)
                {
                    bucketColor = rgba(.4f,.4f,1.0f,1);
                }
                else if (*c == ' ' || *c == '.')
                {
                    bucketColor = rgba(.2f,.2f,.2f,1);
                }
                
                draw_rect(editor.white, bucketPos, bucketSize, bucketColor, 0);
                
                if (c)
                {
                    draw_char(*c, bucketPos, bucketSize.x, rgba(.9f,.9f,.7f,1), 0);
                }
            }
        }
        
        // Draw the point position
        if (point == i)
        {
            draw_rect(editor.white, v2(bucketPos.x - 0.5f*bucketSize.x, bucketPos.y-2), 
                      v2(bucketSize.x, 2), rgba(1,0,0,1), 1);
        }
        
        
        if (c && *c != 10) // carriage return
        {
            bucketPos.x += bucketSize.x;
            if (((bucketPos.x + bucketSize.x) > engine.backBufferSize.x) ||
                (c && *c == 13))
            {
                bucketPos.x = 0;
                bucketPos.y -= bucketSize.y;
            }
        }
    }
}