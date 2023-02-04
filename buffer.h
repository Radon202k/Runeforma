#pragma once

function Buffer *
buffer_create(char *bufferName, char *data, s32 dataSize)
{
    // Allocate a new buffer
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    
    // Clear it to zero
    memset(buffer, 0, sizeof(Buffer));
    
    // Set its name 
    strcpy_s(buffer->bufferName, 512, bufferName);
    
    // Define default values
    assert(dataSize < 40000);
    
    buffer->currentLine = 1;
    
    // Mark *markList;
    u32 arraySize = dataSize*2;
    
    // Set the array size
    buffer->contents.arraySize = arraySize;
    
    // Allocate memory for the array
    buffer->contents.array = (char *)malloc(arraySize);
    
    // Clear the memory to zero
    memset(buffer->contents.array, 0, arraySize);
    
    u32 charCount = 0;
    u32 lineCount = 0;
    
    // Copy everything but \r
    s32 finalLength = 0;
    char *atSrc = data;
    char *atDst = buffer->contents.array;
    for (s32 byte = 0; byte < dataSize; byte++)
    {
        if (*atSrc == '\r' && *(atSrc + 1) != '\n')
        {
            // Replace \r with \n
            *atDst = '\n';
            
            atDst++;
            finalLength++;
            
            lineCount++;
        }
        else if (*atSrc == '\r' && *(atSrc + 1) == '\n')
        {
            // Remove useless \r
        }
        else if (*atSrc == 0)
        {
            // Remove null in the middle of file
        }
        else
        {
            *atDst = *atSrc;
            
            atDst++;
            finalLength++;
            
            if (*atSrc == '\n')
            {
                lineCount++;
            }
            else
            {
                charCount++;
            }
        }
        
        atSrc++;
    }
    
    // Register numbero of chars
    buffer->numChars = charCount;
    buffer->numLines = lineCount+1;
    
    // End the file with a null
    buffer->contents.array[arraySize] = 0;
    
    // Init the internal gap buffer
    gap_buffer_init(&buffer->contents, &buffer->point,
                    finalLength);
    
    // Set a temp filename
    strcpy_s(buffer->fileName, 512, bufferName);
    
    return buffer;
}

function bool buffer_clear(char *bufferName)
{
}

function bool buffer_delete(char *bufferName)
{
}

function bool buffer_set_current(char *bufferName)
{
}

function bool buffer_set_next(char *bufferName)
{
}

function bool buffer_set_name(char *bufferName)
{
}

function bool buffer_get_name(char *bufferName)
{
}

function bool buffer_save(Buffer *buffer)
{
    u32 dataSize = gap_buffer_current_size(&buffer->contents);
    
    char *data = (char *)malloc(dataSize+1);
    
    // Copy the first part, from beginning of array to gap start 
    memcpy(data, buffer->contents.array, buffer->contents.gapLeft);
    
    if (buffer->contents.arraySize - buffer->contents.gapRight > 0)
    {
        // Copy second part, from gap end to end of array
        memcpy(data + buffer->contents.gapLeft, 
               buffer->contents.array + buffer->contents.gapLeft, 
               buffer->contents.arraySize - buffer->contents.gapRight);
    }
    
    // Set last character as null
    data[dataSize] = 0;
    
    return write_file(buffer->fileName, data, dataSize+1);
}

function bool buffer_point_set(s32 loc)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    if (loc >= 0 && loc <= gap_buffer_current_size(&buffer->contents))
    {
        buffer->point = loc;
        return true;
    }
    
    return false;
}

function bool buffer_point_move(s32 count)
{
}

function s32 buffer_point_get(Buffer *buffer)
{
}

function s32 buffer_point_get_line(Buffer *buffer)
{
}

function s32 buffer_start(Buffer *buffer)
{
}

function s32 buffer_end(Buffer *buffer)
{
}

function bool buffer_compare_location(s32 loc1, s32 loc2)
{
    return false;
}

function s32 buffer_location_to_count(s32 loc)
{
}

function s32 buffer_count_to_location()
{
}

// TODO: Marks

function char buffer_get_char_at(s32 loc)
{
    Buffer *buffer = editor.world.currentBuffer;
    s32 gapI = gap_buffer_user_to_gap_coords(&buffer->contents, loc);
    char result = buffer->contents.array[gapI];
    return result;
}

function char buffer_get_char()
{
    Buffer *buffer = editor.world.currentBuffer;
    return buffer_get_char_at(buffer->point);
}

function bool buffer_search_forward(char *string)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    // If the gap is after the point
    if (buffer->point <= buffer->contents.gapLeft)
    {
        s32 n = buffer->contents.gapLeft-buffer->point+1, m = (s32)strlen(string);
        char *arr = buffer->contents.array;
        
        s32 newLineLoc = string_search_naive_first_forward(arr, n, string, m,
                                                           buffer->point+1, buffer->contents.gapLeft);
        if (newLineLoc > -1)
        {
            buffer_point_set(newLineLoc);
            return true;
        }
        
        // Then search from gapRight+1 to end
        n = buffer->contents.arraySize - buffer->contents.gapRight;
        
        newLineLoc = string_search_naive_first_forward(arr, n, string, m,
                                                       buffer->contents.gapRight+1, buffer->contents.arraySize);
        
        if (newLineLoc > -1)
        {
            // Since newLineLoc is after the gap, we need to transform it to user coords
            // before setting the point.
            newLineLoc -= gap_buffer_current_gap_size(&buffer->contents);
            
            buffer_point_set(newLineLoc);
            return true;
        }
    }
    
    // If the gap is before the point
    else if (buffer->point > buffer->contents.gapLeft)
    {
        s32 gapI = gap_buffer_user_to_gap_coords(&buffer->contents, buffer->point);
        
        s32 n = buffer->contents.arraySize-gapI+1, m = (s32)strlen(string);
        char *arr = buffer->contents.array;
        
        // Search from point to end
        s32 newLineLoc = string_search_naive_first_forward(arr, n, string, m,
                                                           gapI+1, buffer->contents.arraySize);
        
        if (newLineLoc > -1)
        {
            // Since newLineLoc is after the gap, we need to transform it to user coords
            // before setting the point.
            newLineLoc -= gap_buffer_current_gap_size(&buffer->contents);
            
            buffer_point_set(newLineLoc);
            return true;
        }
    }
    
    return false;
}

function bool buffer_search_backward(char *string)
{
    Buffer *buffer = editor.world.currentBuffer;
    char *arr = buffer->contents.array;
    
    // If the gap is after the point (or at the point)
    if (buffer->point <= buffer->contents.gapLeft)
    {
        // Search from point-1 to start
        s32 n = buffer->point-1, m = (s32)strlen(string);
        
        s32 newLineLoc = string_search_naive_first_backward(arr, n, string, m,
                                                            0, buffer->point-1);
        if (newLineLoc > -1)
        {
            buffer_point_set(newLineLoc);
            return true;
        }
    }
    // If the gap is before the point
    else if (buffer->point > buffer->contents.gapLeft)
    {
        // Since this is after the gap, must account for the gap size in the index
        s32 gapI = gap_buffer_user_to_gap_coords(&buffer->contents, buffer->point);
        
        // First search from gapI-1 to gapRight
        s32 n = gapI-1-buffer->contents.gapRight, m = (s32)strlen(string);
        
        s32 newLineLoc = string_search_naive_first_backward(arr, n, string, m,
                                                            buffer->contents.gapRight, gapI-1);
        if (newLineLoc > -1)
        {
            // Again, since it's after the gap, must convert back
            newLineLoc -= gap_buffer_current_gap_size(&buffer->contents);
            
            buffer_point_set(newLineLoc);
            return true;
        }
        
        // Then search from gapLeft-1 to start
        n = buffer->contents.gapLeft-1, m = (s32)strlen(string);
        
        newLineLoc = string_search_naive_first_backward(arr, n, string, m,
                                                        0, buffer->contents.gapLeft-1);
        if (newLineLoc > -1)
        {
            buffer_point_set(newLineLoc);
            return true;
        }
    }
    
    return false;
}