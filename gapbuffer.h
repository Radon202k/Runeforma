#pragma once

function void
gap_buffer_free(GapBuffer *buffer)
{
    free(buffer->storage);
    buffer->storage = 0;
    buffer->storageSize = 0;
    buffer->left = 0;
    buffer->right = 0;
}

function void
gap_buffer_init(GapBuffer *buffer, s32 *point,
                char *data, s32 dataSize)
{
    // Set the storage size  
    buffer->storageSize = dataSize*2;
    
    // Allocate the storage array
    buffer->storage = (char *)malloc(buffer->storageSize);
    
    // Clear the memory to zero
    memset(buffer->storage, 0, buffer->storageSize);
    
    // Copy the bytes
    memcpy(buffer->storage, data, dataSize);
    
    // Set the gap left to be at the end of the buffer 
    buffer->left = dataSize;
    
    // And the gap right to be at the end of the storage array
    buffer->right = buffer->storageSize;
    
    // Set the point to be at the end of the buffer
    *point = dataSize;
}

function s32
gap_buffer_current_gap_size(GapBuffer *buffer)
{
    s32 gapSize = (buffer->right - buffer->left);
    return gapSize;
}

function s32
gap_buffer_current_size(GapBuffer *buffer)
{
    assert(buffer->right >= buffer->left);
    s32 gapSize = gap_buffer_current_gap_size(buffer);
    s32 result = buffer->storageSize - gapSize;
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
    if (userPoint >= buffer->left)
    {
        result += (buffer->right - buffer->left);
    }
    
    return result;
}

function s32
gap_buffer_get_range(GapBuffer *buffer, 
                     char *dest, s32 maxSize,
                     s32 point, s32 mark)
{
    s32 GS = buffer->left;
    s32 GE = buffer->right;
    
    assert(point < mark);
    
    // If the gap is inside the range
    if (point <= GS && GS <= mark)
    {
        // Copy the first range between the point and the gap start
        memcpy(dest, buffer->storage + point, min(maxSize, (GS-point)));
        
        // Account for the fact that the mark is after the gap
        mark = gap_buffer_user_to_gap_coords(buffer, mark);
        
        // Copy the second range between the gap end and the mark
        memcpy(dest+GS-point, buffer->storage+GE, mark-GE);
        
        return mark-GE;
    }
    // Else if gap is after the range
    else if (mark < GS)
    {
        // Copy from point to mark (gap is after so it doesn't matter)
        memcpy(dest, buffer->storage+point, mark-point);
        
        return mark-point;
    }
    // Else if gap is before the range
    else if (GS < point)
    {
        // Account for the gap size
        point = gap_buffer_user_to_gap_coords(buffer, point);
        mark = gap_buffer_user_to_gap_coords(buffer, mark);
        
        // Copy from point to mark (gap is before so it doesn't matter)
        memcpy(dest, buffer->storage+point, mark-point);
        
        return mark-point;
    }
    
    assert(!"not");
    return 0;
}

function void
gap_buffer_move_gap_to_point(GapBuffer *buffer, s32 point)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    assert(point >= 0 && point <= bufferSize);
    
    // If the gap is in the right position
    if (buffer->left == point)
    {
        // Do nothing
    }
    // If the gap is before the point
    else if (buffer->left < point)
    {
        // The gap must be moved to the point. 
        // The characters **after the gap** but **before the point** must be moved
        
        // Calculate how many characters we will shift
        s32 charShiftCount = point - buffer->left;
        
        // The size of the gap
        s32 gapSize = buffer->right - buffer->left;
        
        // Begin at the start of the gap, go to the last one before the point
        for (s32 i = buffer->left; i < point; ++i)
        {
            // Shift the character
            buffer->storage[i] = buffer->storage[i + gapSize];
            // Clear the other one for clarity while developing
            // TODO: Optimize
            buffer->storage[i + gapSize] = 0;
        }
        
        // Move the gap "pointers"
        buffer->left += charShiftCount;
        buffer->right += charShiftCount;
    }
    // If the gap is after the point
    else if (buffer->left > point)
    {
        // The gap must be moved to the point. 
        // The characters **after the point** but **before the gap** must be moved
        
        // The amount of chars to shift
        s32 charShiftCount = buffer->left - point;
        
        // Size of gap
        s32 gapSize = buffer->right - buffer->left;
        
        // Start at the character just before the end of the gap
        // Continue until i - gapSize is lower than the point
        for (s32 i = buffer->right-1; (i - gapSize) >= point; --i)
        {
            // Shift the character
            buffer->storage[i] = buffer->storage[i - gapSize];
            buffer->storage[i - gapSize] = 0;
        }
        
        // Move the gap "pointers"
        buffer->left -= charShiftCount;
        buffer->right -= charShiftCount;
    }
}

function void
gap_buffer_resize(GapBuffer *buffer, u32 newArraySize)
{
    s32 oldArraySize = buffer->storageSize;
    
    // Alloc new array
    char *newArray = (char *)malloc(newArraySize);
    
    // Clear it to zero for easy memory visualization while developing
    // TODO: Optimize
    memset(newArray, 0, newArraySize);
    
    // Copy the contents
    memcpy(newArray, buffer->storage, oldArraySize);
    
    // Free old array
    free(buffer->storage);
    
    // Set the new pointer
    buffer->storage = newArray;
    
    // Update the arraySize buffer variable
    buffer->storageSize = newArraySize;
    
    // Set the gap to be from the end of last size until the end of new size
    buffer->left = oldArraySize;
    buffer->right = newArraySize;
}

function void
gap_buffer_insert_char(GapBuffer *buffer, char c, s32 point)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    assert(point >= 0 && point <= bufferSize);
    
    // Move gap
    gap_buffer_move_gap_to_point(buffer, point);
    
    // Increase the gap start pointer (reduce the gap size)
    buffer->left++;
    
    // Insert the character in the point position
    buffer->storage[point] = c;
    
    // If there is no space left in the gap
    if (buffer->left == buffer->right)
    {
        s32 newArraySize = 2*bufferSize;
        gap_buffer_resize(buffer, newArraySize);
    }
}

function void
gap_buffer_delete_char(GapBuffer *buffer, s32 point)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    assert(point > 0 && point <= bufferSize);
    
    // Move gap
    gap_buffer_move_gap_to_point(buffer, point);
    
    // Decrease the gap start pointer (reduce the gap size)
    buffer->left--;
    
    // Clear the position for clarity while developing
    // TODO: Optimize
    buffer->storage[point] = 0;
}

function s32
gap_buffer_insert_string(GapBuffer *buffer, char *s, s32 point)
{
    s32 len = (s32)strlen(s);
    s32 gapSize = gap_buffer_current_gap_size(buffer);
    s32 storageSize = buffer->storageSize;
    if (len > gapSize)
    {
        s32 diff = gapSize - len;
        s32 newSize = 2*storageSize + diff;
        gap_buffer_resize(buffer, newSize);
    }
    
    gap_buffer_move_gap_to_point(buffer, point);
    buffer->left = buffer->left + len;
    memcpy(buffer->storage + point, s, len);
    return len;
}

// Delete a range between point and mark (point < mark is required)
function void
gap_buffer_delete_range(GapBuffer *buffer, s32 point, s32 mark)
{
    assert(point < mark);
    
    s32 GS = buffer->left; // Gap Start
    
    gap_buffer_move_gap_to_point(buffer, point);
    
    // account for the gap size
    mark = gap_buffer_user_to_gap_coords(buffer, mark);
    
    // get the gap end after the gap has moved
    s32 GE = buffer->right;
    
    // clear from Gap End to Mark
    memset(buffer->storage + GE, 0, mark - GE);
    
    // set GE to mark
    buffer->right = mark;
}