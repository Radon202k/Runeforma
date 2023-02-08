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

function void
gap_buffer_move_gap_to_point(GapBuffer *buffer, s32 *point)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    assert(*point >= 0 && *point <= bufferSize);
    
    // If the gap is in the right position
    if (buffer->left == *point)
    {
        // Do nothing
    }
    // If the gap is before the point
    else if (buffer->left < *point)
    {
        // The gap must be moved to the point. 
        // The characters **after the gap** but **before the point** must be moved
        
        // Calculate how many characters we will shift
        s32 charShiftCount = *point - buffer->left;
        
        // The size of the gap
        s32 gapSize = buffer->right - buffer->left;
        
        // Begin at the start of the gap, go to the last one before the point
        for (s32 i = buffer->left; i < *point; ++i)
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
    else if (buffer->left > *point)
    {
        // The gap must be moved to the point. 
        // The characters **after the point** but **before the gap** must be moved
        
        // The amount of chars to shift
        s32 charShiftCount = buffer->left - *point;
        
        // Size of gap
        s32 gapSize = buffer->right - buffer->left;
        
        // Start at the character just before the end of the gap
        // Continue until i - gapSize is lower than the point
        for (s32 i = buffer->right-1; (i - gapSize) >= *point; --i)
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
gap_buffer_insert_char(GapBuffer *buffer, char c, s32 *point)
{
    s32 bufferSize = gap_buffer_current_size(buffer);
    assert(*point >= 0 && *point <= bufferSize);
    
    // If there is no space left in the gap
    if (bufferSize == buffer->storageSize)
    {
        s32 oldArraySize = buffer->storageSize;
        s32 newArraySize = 2*oldArraySize;
        buffer->storageSize = newArraySize;
        
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
    
    gap_buffer_move_gap_to_point(buffer, point);
    
    // Increase the gap start pointer (reduce the gap size)
    buffer->left++;
    
    // Insert the character in the point position
    buffer->storage[*point] = c;
    
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
    buffer->storage[*point] = 0;
    
    // Decrease the gap start pointer (reduce the gap size)
    buffer->left--;
}

function void
test_gap_buffer_insert_char()
{
    // Test case 1: inserting at the beginning of the buffer
    GapBuffer buffer1 = {
        .left = 0,
        .right = 5,
        .storageSize = 10,
        .storage = "#####67890"
    };
    
    s32 point = 0;
    gap_buffer_insert_char(&buffer1, 'A', &point);
    assert(buffer1.left == 1);
    assert(buffer1.right == 5);
    assert(strcmp(buffer1.storage, "A####67890") == 0);
    assert(point == 1);
    
    // Test case 2: inserting at the end of the buffer
    GapBuffer buffer2 = {
        .left = 5,
        .right = 10,
        .storageSize = 10,
        .storage = "12345#####"
    };
    point = 5;
    gap_buffer_insert_char(&buffer2, 'Z', &point);
    assert(buffer2.left == 6);
    assert(buffer2.right == 10);
    assert(strcmp(buffer2.storage, "12345Z####") == 0);
    assert(point == 6);
    
    // Add more test cases as needed
}
