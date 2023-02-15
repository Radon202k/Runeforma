#pragma once

function void
gap_buffer_free(GapBuffer *buffer)
{
    free(buffer->storage);
    buffer->storage = 0;
    buffer->storageLength = 0;
    buffer->left = 0;
    buffer->right = 0;
}

function void
gap_buffer_init(GapBuffer *buffer, s32 *point,
                wchar_t *data, s32 dataLength)
{
    // Set the storage length  
    buffer->storageLength = dataLength*2;
    
    // Allocate the storage array
    buffer->storage = alloc_array(buffer->storageLength, wchar_t);
    
    // Clear the memory to zero
    clear_array(buffer->storage, buffer->storageLength, wchar_t);
    
    // Copy the bytes
    copy_array(buffer->storage, data, dataLength, wchar_t);
    
    // Set the gap left to be at the end of the buffer 
    buffer->left = dataLength;
    
    // And the gap right to be at the end of the storage array
    buffer->right = buffer->storageLength;
    
    // Set the point to be at the end of the buffer
    *point = dataLength;
}

function s32
gap_buffer_gap_length(GapBuffer *buffer)
{
    return buffer->right-buffer->left;
}

function s32
gap_buffer_current_length(GapBuffer *buffer)
{
    assert(buffer->right >= buffer->left);
    s32 gapLength = gap_buffer_gap_length(buffer);
    s32 result = buffer->storageLength-gapLength;
    return result;
}

function s32
gap_buffer_user_to_gap_coords(GapBuffer *buffer, s32 userPoint)
{
    s32 bufferSize = gap_buffer_current_length(buffer);
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
                     wchar_t *dest, s32 maxLength,
                     s32 point, s32 mark)
{
    s32 GS = buffer->left;
    s32 GE = buffer->right;
    
    assert(point < mark);
    
    // If the gap is inside the range
    if (point <= GS && GS <= mark)
    {
        // Copy the first range between the point and the gap start
        copy_array(dest, buffer->storage + point, min(maxLength, (GS-point)), wchar_t);
        
        // Account for the fact that the mark is after the gap
        mark = gap_buffer_user_to_gap_coords(buffer, mark);
        
        // Copy the second range between the gap end and the mark
        copy_array(dest+GS-point, buffer->storage+GE, (mark-GE), wchar_t);
        
        return mark-GE;
    }
    // Else if gap is after the range
    else if (mark < GS)
    {
        // Copy from point to mark (gap is after so it doesn't matter)
        copy_array(dest, buffer->storage+point, (mark-point), char);
        
        return mark-point;
    }
    // Else if gap is before the range
    else if (GS < point)
    {
        // Account for the gap size
        point = gap_buffer_user_to_gap_coords(buffer, point);
        mark = gap_buffer_user_to_gap_coords(buffer, mark);
        
        // Copy from point to mark (gap is before so it doesn't matter)
        copy_array(dest, buffer->storage+point, (mark-point), char);
        
        return mark-point;
    }
    
    assert(!"not");
    return 0;
}

function void
gap_buffer_move_gap_to_point(GapBuffer *buffer, s32 point)
{
    s32 bufferLength = gap_buffer_current_length(buffer);
    assert(point >= 0 && point <= bufferLength);
    
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
gap_buffer_resize(GapBuffer *buffer, u32 newArrayLength)
{
    s32 oldArrayLength = buffer->storageLength;
    
    // Alloc new array
    wchar_t *newArray = alloc_array(newArrayLength, wchar_t);
    
    // Clear it to zero for easy memory visualization while developing
    // TODO: Optimize
    clear_array(newArray, newArrayLength, wchar_t);
    
    // Copy the contents
    copy_array(newArray, buffer->storage, oldArrayLength, wchar_t);
    
    // Free old array
    free(buffer->storage);
    
    // Set the new pointer
    buffer->storage = newArray;
    
    // Update the arrayLength buffer variable
    buffer->storageLength = newArrayLength;
    
    // Set the gap to be from the end of last size until the end of new size
    buffer->left = oldArrayLength;
    buffer->right = newArrayLength;
}

function void
gap_buffer_insert_char(GapBuffer *buffer, wchar_t c, s32 point)
{
    s32 bufferSize = gap_buffer_current_length(buffer);
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
    s32 bufferSize = gap_buffer_current_length(buffer);
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
gap_buffer_insert_string(GapBuffer *buffer, wchar_t *s, s32 point)
{
    s32 len = string_length(s);
    
    s32 gapLength = gap_buffer_gap_length(buffer);
    
    s32 storageLength = buffer->storageLength;
    
    if (len >= gapLength)
    {
        s32 diff = len-gapLength;
        s32 newLength = 2*storageLength + diff;
        gap_buffer_resize(buffer, newLength);
    }
    
    gap_buffer_move_gap_to_point(buffer, point);
    buffer->left = buffer->left + len;
    // buffer->right = buffer->right + len;
    
    copy_array(buffer->storage + point, s, len, wchar_t);
    
    return len;
}

// Delete a range between point and mark (point < mark is required)
function void
gap_buffer_delete_range(GapBuffer *buffer, s32 point, s32 mark)
{
    assert(point < mark);
    
    s32 GS = buffer->left; // Gap Start
    
    // Move the gap to the point
    gap_buffer_move_gap_to_point(buffer, point);
    
    // account for the gap size
    mark = gap_buffer_user_to_gap_coords(buffer, mark);
    
    // Save gap end
    s32 oldGE = buffer->right;
    
    // Update gap buffer end
    buffer->right = mark;
    
    // get the gap end after the gap has moved
    s32 GE = buffer->right;
    
    // clear from Old Gap End to Mark
    clear_array(buffer->storage+oldGE, mark-oldGE, wchar_t);
}