#pragma once

// Get a buffer pointer from a buffer name
function Buffer *
buffer_get(char *bufferName)
{
    Buffer *result = 0;
    
    // For each buffer in the chain
    Buffer *buffer = editor.world.bufferChain;
    while (buffer)
    {
        // If the names are equal
        if (strcmp(bufferName, buffer->bufferName) == 0)
        {
            // Found the buffer
            result = buffer;
            break;
        }
        
        // Advance the chain
        buffer = buffer->nextChainEntry;
    }
    
    return result;
}

// Takes a name and creates an empty buffer with that name. Note that
// no two buffers may have the same name.
function bool
buffer_create(char *bufferName)
{
    // Look for other buffer with the same name
    Buffer *other = buffer_get(bufferName);
    
    // If found name conflict
    if (other != 0)
    {
        // Failed because buffer names are required to be unique
        return false;
    }
    // Otherwise
    else
    {
        // Allocate a new buffer
        Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
        
        // Clear it to zero
        memset(buffer, 0, sizeof(Buffer));
        
        // Scratch data
        char *data = "Runeforma text editor written in C.";
        u32 dataSize = (u32)strlen(data);
        
        // Create a scratch gap buffer for it
        gap_buffer_init(&buffer->gapBuffer, &buffer->point,
                        data, dataSize);
        
        // Set its name 
        strcpy_s(buffer->bufferName, 512, bufferName);
        
        // Push to the head of the list
        buffer->nextChainEntry = editor.world.bufferChain;
        editor.world.bufferChain = buffer;
        
        return true;
    }
}

// Removes all characters and marks from the specified buffer
function bool
buffer_clear(char *bufferName)
{
    // Look to see if there is a buffer with that name
    Buffer *buffer = buffer_get(bufferName);
    
    // If found a buffer with that name
    if (buffer)
    {
        // TODO
    }
    else
    {
        return false;
    }
}

// Deletes the specified buffer. If it is the current one, the next
// buffer in the chain becomes the current one. If no buffers are
// left, the initial "scratch" buffer is re-created.
function bool 
buffer_delete(char *bufferName)
{
    // Look to see if there is a buffer with that name
    Buffer *buffer = buffer_get(bufferName);
    
    // If found a buffer with that name
    if (buffer)
    {
        // If this is the current buffer, grab the pointer to the next
        Buffer *nextInChain = 0;
        bool wasCurrent = false;
        if (buffer == editor.world.currentBuffer)
        {
            wasCurrent = true;
            nextInChain = buffer->nextChainEntry;
        }
        
        // If this was the current one
        if (wasCurrent)
        {
            // If there was a next buffer
            if (nextInChain != 0)
            {
                // Set the next as the current one
                editor.world.currentBuffer = nextInChain;
            }
            // Otherwise
            else 
            {
                // Look at the head to see if there is a buffer left
                Buffer *head = editor.world.bufferChain;
                
                // If there is something in the chain
                if (head != 0)
                {
                    // Set head as current
                    editor.world.currentBuffer = head;
                }
                else
                {
                    // Re-create the scratch buffer
                    buffer_create("Stratch");
                    
                    // Set it as current
                    editor.world.currentBuffer = buffer_get("Scratch");
                }
            }
        }
        
        // Free the memory
        gap_buffer_free(&buffer->gapBuffer);
        free(buffer);
    }
    // Otherwise
    else
    {
        // No buffer with that name to delete
        return false;
    }
}

// Sets the current buffer to the one specified
function bool
buffer_set_current(char *bufferName)
{
    // Look to see if there is a buffer with that name
    Buffer *buffer = buffer_get(bufferName);
    
    // If found it
    if (buffer)
    {
        // Set the buffer as current one;
        editor.world.currentBuffer = buffer;
        
        return true;
    }
    // Otherwise
    else
    {
        return false;
    }
}

// Sets the current buffer to the next one in the chain, and it returns
// the name of the "new" buffer. This mechanism allows for iterating
// through all buffers looking for one which meets an arbitrary test.
function char *
buffer_set_next(void)
{
    editor.world.currentBuffer = editor.world.currentBuffer->nextChainEntry;
    return editor.world.currentBuffer->bufferName;
}

// Changes the name of the current buffer
function bool 
buffer_set_name(char *bufferName)
{
    Buffer *buffer = editor.world.currentBuffer;
    // Clear it to zero for clarity in memory
    memset(buffer->bufferName, 0, array_count(buffer->bufferName));
    // Get length of new name
    u32 length = (u32)strlen(bufferName);
    // Copy new name into the bufferName
    memcpy(buffer->bufferName, bufferName, length);
    return true;
}

// Returns the name of the current buffer
function char *
buffer_get_name()
{
    Buffer *buffer = editor.world.currentBuffer;
    return buffer->bufferName;
}

// Clears the buffer and reads the currently named file into the buffer
// making any required conversions between the external and internal
// representations. The modified flag is set if the file was not empty
function bool
buffer_read(void)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    File file = read_file(buffer->fileName);
    if (file.exists)
    {
        assert(file.size < 40000);
        
        buffer->currentLine = 1;
        
        // Mark *markList;
        
        u32 charCount = 0;
        u32 lineCount = 0;
        s32 finalLength = 0;
        
        // Build a temp array to convert the data
        char *tempData = (char *)malloc(file.size);
        
        char *atSrc = (char *)file.bytes;
        char *atDst = tempData;
        
        // For every byte
        for (u32 byte = 0; byte < file.size; byte++)
        {
            // If it is a carriage return and the next is a new line
            if (*atSrc == '\r' && *(atSrc + 1) == '\n')
            {
                // Copy a new line only (effectively removing the carriage return)
                *atDst = '\n';
                
                // Advance the dest pointer
                atDst++;
                
                // Increment the length to account for the new line char
                finalLength++;
                
                // Count as a line
                lineCount++;
            }
            // Else if it is a carriage return but the next is not a new line
            else if (*atSrc == '\r' && *(atSrc + 1) != '\n')
            {
                // Ignore the character altogether, effectively "removing"
                // the useless carriage return that is alone by some reason
            }
            // Else if it is a null character
            else if (*atSrc == 0)
            {
                // Note that this for loop is using the file.size to stop
                // So if we found a null character while the for is still
                // running, it means that this null character is in the
                // middle of the file, so we will just ignore it.
            }
            // Else, it must be a valid character
            else
            {
                // Copy the character
                *atDst = *atSrc;
                
                // Advance the dest pointer
                atDst++;
                
                // Increment the final length to account for the new char
                finalLength++;
                
                // If it is a new line
                if (*atSrc == '\n')
                {
                    // Count the line
                    lineCount++;
                }
                // Otherwise
                else
                {
                    // Count as character
                    charCount++;
                    
                    // TODO: We might want a flag to allow counting new line chars as
                    // characteres as well.
                }
            }
            
            // Advance the source pointer
            atSrc++;
        }
        
        // End the data with a null
        tempData[finalLength] = 0;
        
        // Register number of chars
        buffer->numChars = charCount;
        buffer->numLines = lineCount+1;
        
        // Free the gap buffer
        gap_buffer_free(&buffer->gapBuffer);
        
        // Init the internal gap buffer
        gap_buffer_init(&buffer->gapBuffer, &buffer->point,
                        tempData, finalLength);
        
        // Free the temp array
        free(tempData);
        
        return true;
    }
    
    return false;
}

// Writes the buffer to the currently named file, making any required
// conversions between the internal and external representations. The
// modified flag is cleared and the file time is updated to the 
// current time.
function bool
buffer_write(void)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    u32 dataSize = gap_buffer_current_size(&buffer->gapBuffer);
    
    char *data = (char *)malloc(dataSize+1);
    
    // Copy the first part, from beginning of array to gap start 
    memcpy(data, buffer->gapBuffer.storage, buffer->gapBuffer.left);
    
    if (buffer->gapBuffer.storageSize - buffer->gapBuffer.right > 0)
    {
        // Copy second part, from gap end to end of array
        memcpy(data + buffer->gapBuffer.left, 
               buffer->gapBuffer.storage + buffer->gapBuffer.left, 
               buffer->gapBuffer.storageSize - buffer->gapBuffer.right);
    }
    
    // Set last character as null
    data[dataSize] = 0;
    
    return write_file(buffer->fileName, data, dataSize+1);
}

// Sets the point to the specified location
function bool
buffer_point_set(s32 loc)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    // If the location is a valid position [0,bufferSize]
    if (loc >= 0 && loc <= gap_buffer_current_size(&buffer->gapBuffer))
    {
        // Set the point and return true
        buffer->point = loc;
        return true;
    }
    
    return false;
}

// Moves the point forward (if count is positive) or backward (if negative)
// by abs(count) characters
function bool
buffer_point_move(s32 count)
{
    Buffer *buffer = editor.world.currentBuffer;
    s32 currentLocation = buffer->point;
    
    s32 newLocation = currentLocation + count;
    s32 currentGapBufferSize = gap_buffer_current_size(&buffer->gapBuffer);
    
    // If new location is negative
    if (newLocation < 0)
    {
        return false;
    }
    // Else if it is greater than the current gap buffer size
    else if (newLocation > currentGapBufferSize)
    {
        return false;
    }
    // Else, it must be valid
    else
    {
        // Set the location and return true
        buffer->point = newLocation;
        return true;
    }
}

// Returns the current location
function s32
buffer_point_get(void)
{
    Buffer *buffer = editor.world.currentBuffer;
    return buffer->point;
}

// Returns the number of the line that the point is on.
// Note that lines start from one (not zero).
function s32
buffer_point_get_line(void)
{
    Buffer *buffer = editor.world.currentBuffer;
    return buffer->currentLine;
}

// Returns the location of the start of the buffer
function s32 buffer_start(void)
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
    s32 gapI = gap_buffer_user_to_gap_coords(&buffer->gapBuffer, loc);
    char result = buffer->gapBuffer.storage[gapI];
    return result;
}

function char buffer_get_char()
{
    Buffer *buffer = editor.world.currentBuffer;
    return buffer_get_char_at(buffer->point);
}

// Returns the name of the current buffer
function char *
buffer_get_fileName(void)
{
    Buffer *buffer = editor.world.currentBuffer;
    return buffer->fileName;
}

function void
buffer_set_fileName(char *fileName)
{
    u32 length = (u32)strlen(fileName);
    Buffer *buffer = editor.world.currentBuffer;
    memset(buffer->fileName, 0, array_count(buffer->fileName));
    memcpy(buffer->fileName, fileName, length);
}

function bool buffer_search_forward(char *string)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    // If the gap is after the point
    if (buffer->point <= buffer->gapBuffer.left)
    {
        s32 n = buffer->gapBuffer.left-buffer->point+1, m = (s32)strlen(string);
        char *arr = buffer->gapBuffer.storage;
        
        s32 newLineLoc = string_search_naive_first_forward(arr, n, string, m,
                                                           buffer->point+1, buffer->gapBuffer.left);
        if (newLineLoc > -1)
        {
            buffer_point_set(newLineLoc);
            return true;
        }
        
        // Then search from gapRight+1 to end
        n = buffer->gapBuffer.storageSize - buffer->gapBuffer.right;
        
        newLineLoc = string_search_naive_first_forward(arr, n, string, m,
                                                       buffer->gapBuffer.right+1, buffer->gapBuffer.storageSize);
        
        if (newLineLoc > -1)
        {
            // Since newLineLoc is after the gap, we need to transform it to user coords
            // before setting the point.
            newLineLoc -= gap_buffer_current_gap_size(&buffer->gapBuffer);
            
            buffer_point_set(newLineLoc);
            return true;
        }
    }
    
    // If the gap is before the point
    else if (buffer->point > buffer->gapBuffer.left)
    {
        s32 gapI = gap_buffer_user_to_gap_coords(&buffer->gapBuffer, buffer->point);
        
        s32 n = buffer->gapBuffer.storageSize-gapI+1, m = (s32)strlen(string);
        char *arr = buffer->gapBuffer.storage;
        
        // Search from point to end
        s32 newLineLoc = string_search_naive_first_forward(arr, n, string, m,
                                                           gapI+1, buffer->gapBuffer.storageSize);
        
        if (newLineLoc > -1)
        {
            // Since newLineLoc is after the gap, we need to transform it to user coords
            // before setting the point.
            newLineLoc -= gap_buffer_current_gap_size(&buffer->gapBuffer);
            
            buffer_point_set(newLineLoc);
            return true;
        }
    }
    
    return false;
}

function bool buffer_search_backward(char *string)
{
    Buffer *buffer = editor.world.currentBuffer;
    char *arr = buffer->gapBuffer.storage;
    
    // If the gap is after the point (or at the point)
    if (buffer->point <= buffer->gapBuffer.left)
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
    else if (buffer->point > buffer->gapBuffer.left)
    {
        // Since this is after the gap, must account for the gap size in the index
        s32 gapI = gap_buffer_user_to_gap_coords(&buffer->gapBuffer, buffer->point);
        
        // First search from gapI-1 to gapRight
        s32 n = gapI-1-buffer->gapBuffer.right, m = (s32)strlen(string);
        
        s32 newLineLoc = string_search_naive_first_backward(arr, n, string, m,
                                                            buffer->gapBuffer.right, gapI-1);
        if (newLineLoc > -1)
        {
            // Again, since it's after the gap, must convert back
            newLineLoc -= gap_buffer_current_gap_size(&buffer->gapBuffer);
            
            buffer_point_set(newLineLoc);
            return true;
        }
        
        // Then search from gapLeft-1 to start
        n = buffer->gapBuffer.left-1, m = (s32)strlen(string);
        
        newLineLoc = string_search_naive_first_backward(arr, n, string, m,
                                                        0, buffer->gapBuffer.left-1);
        if (newLineLoc > -1)
        {
            buffer_point_set(newLineLoc);
            return true;
        }
    }
    
    return false;
}