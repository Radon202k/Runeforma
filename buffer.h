#pragma once

// Get a buffer pointer from a buffer name
function Buffer *
buffer_get(wchar_t *bufferName)
{
    Buffer *result = 0;
    
    // For each buffer in the chain
    Buffer *buffer = editor.world.bufferChain;
    while (buffer)
    {
        // If the names are equal
        if (string_equal(bufferName, buffer->bufferName))
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
buffer_create(wchar_t *bufferName)
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
        Buffer *buffer = alloc_type(Buffer);
        
        // Scratch data
        wchar_t *data = L"Runeforma text editor written in C.";
        u32 dataLength = string_length(data);
        
        // Create a scratch gap buffer for it
        gap_buffer_init(&buffer->gapBuffer, &buffer->point,
                        data, dataLength);
        
        // Set its name 
        string_copy(buffer->bufferName, 512, bufferName);
        
        // Push to the head of the list
        buffer->nextChainEntry = editor.world.bufferChain;
        editor.world.bufferChain = buffer;
        
        return true;
    }
}

// Removes all characters and marks from the specified buffer
function bool
buffer_clear(wchar_t *bufferName)
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
buffer_delete(wchar_t *bufferName)
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
                    buffer_create(L"Stratch");
                    
                    // Set it as current
                    editor.world.currentBuffer = buffer_get(L"Scratch");
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
buffer_set_current(wchar_t *bufferName)
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
function wchar_t *
buffer_set_next(void)
{
    editor.world.currentBuffer = editor.world.currentBuffer->nextChainEntry;
    return editor.world.currentBuffer->bufferName;
}

// Changes the name of the current buffer
function bool 
buffer_set_name(wchar_t *bufferName)
{
    Buffer *buffer = editor.world.currentBuffer;
    // Clear current name to zero for clarity in memory
    clear_array(buffer->bufferName, array_count(buffer->bufferName), wchar_t);
    // Get length of new name
    u32 length = string_length(bufferName);
    // Copy new name into the bufferName
    copy_array(buffer->bufferName, bufferName, length, wchar_t);
    return true;
}

// Returns the name of the current buffer
function wchar_t *
buffer_get_name()
{
    Buffer *buffer = editor.world.currentBuffer;
    return buffer->bufferName;
}

function bool
isUtf16(u8 *bytes, u32 size)
{
    if (bytes[0] == 255 && bytes[1] == 254) return true;
    else if (bytes[0] == 254 && bytes[1] == 255) return true;
    else return false;
}

// Clears the buffer and reads the currently named file into the buffer
// making any required conversions between the external and internal
// representations. The modified flag is set if the file was not empty
function bool
buffer_read(void)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    wchar_t *text = 0;
    s32 textLength = 0;
    
    File file = read_file(buffer->fileName);
    if (file.exists)
    {
        assert(file.size < 40000);
        
        bool fileUsesUtf16 = isUtf16(file.bytes, file.size);
        
        // If the file uses UTF-16
        if (fileUsesUtf16)
        {
            // The length of the text is equal to the file size divided by 2
            // Minus 1 to remove the null character at the end
            textLength = (file.size/sizeof(wchar_t))-1;
            
            // Allocate the array
            text = alloc_array(textLength, wchar_t);
            
            // Copy the text into it (skip BOM at first wchar_t (2 bytes))
            copy_array(text, file.bytes+2, textLength, wchar_t);
        }
        // Else, assumes the file uses UTF-8 or ASCII
        else
        {
            // Get the text length
            textLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (LPCCH)file.bytes, -1, NULL, 0);
            
            // Alloc the array
            text = alloc_array(textLength, wchar_t);
            
            // Convert the text to UTF-16 and copy into text
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (LPCCH)file.bytes, -1, text, textLength);
        }
        
        // Process the file
        buffer->numLines = 1;
        s32 lastLineCharP = 0;
        for (s32 charIndex = 0;
             charIndex < textLength;
             ++charIndex)
        {
            if (text[charIndex] == 10)
            {
                buffer->numLines++;
                lastLineCharP = charIndex;
            }
            else
            {
                buffer->numChars++;
            }
        }
        
        buffer->lastLineCharP = lastLineCharP;
        buffer->currentLine = 1;
    }
    
    // Free the gap buffer
    gap_buffer_free(&buffer->gapBuffer);
    
    // Init the internal gap buffer
    gap_buffer_init(&buffer->gapBuffer, &buffer->point,
                    text, textLength);
    
    return true;
}

// Writes the buffer to the currently named file, making any required
// conversions between the internal and external representations. The
// modified flag is cleared and the file time is updated to the 
// current time.
function bool
buffer_write(void)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    u32 dataLength = gap_buffer_length(&buffer->gapBuffer);
    wchar_t *data = alloc_array(dataLength+1, wchar_t);
    
    gap_buffer_get_range(&buffer->gapBuffer, 
                         data, dataLength+1,
                         0, dataLength);
    
    // Set last character as null
    data[dataLength] = 0;
    
    return write_file(buffer->fileName, data, dataLength+1);
}

// Sets the point to the specified location
function bool
buffer_point_set(s32 loc)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    // If the location is a valid position [0,bufferSize]
    if (loc >= 0 && loc <= gap_buffer_length(&buffer->gapBuffer))
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
    s32 currentGapBufferSize = gap_buffer_length(&buffer->gapBuffer);
    
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

function wchar_t 
buffer_get_char(s32 loc)
{
    Buffer *buffer = editor.world.currentBuffer;
    wchar_t result = gap_buffer_get_char(&buffer->gapBuffer, loc);
    return result;
}

// Returns the name of the current buffer
function wchar_t *
buffer_get_fileName(void)
{
    Buffer *buffer = editor.world.currentBuffer;
    return buffer->fileName;
}

function void
buffer_set_fileName(wchar_t *fileName)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    // Clear current filename to zero
    clear_array(buffer->fileName, array_count(buffer->fileName), wchar_t);
    
    // Copy new filename
    u32 length = string_length(fileName);
    copy_array(buffer->fileName, fileName, length, wchar_t);
}

function bool 
buffer_search_forward(wchar_t **stringArray, s32 arrayN)
{
    Buffer *buffer = editor.world.currentBuffer;
    
    for (s32 k = 0;
         k < arrayN;
         ++k)
    {
        // If the gap is after the point
        if (buffer->point <= buffer->gapBuffer.left)
        {
            s32 n = buffer->gapBuffer.left-buffer->point+1, m = string_length(stringArray[k]);
            wchar_t *arr = buffer->gapBuffer.storage;
            
            s32 newLineLoc = string_search_naive_first_forward(arr, n, stringArray[k], m,
                                                               buffer->point+1, buffer->gapBuffer.left);
            if (newLineLoc > -1)
            {
                buffer_point_set(newLineLoc);
                return true;
            }
            
            // Then search from gapRight+1 to end
            n = buffer->gapBuffer.storageLength-buffer->gapBuffer.right;
            
            newLineLoc = string_search_naive_first_forward(arr, n, stringArray[k], m,
                                                           buffer->gapBuffer.right+1, 
                                                           buffer->gapBuffer.storageLength);
            
            if (newLineLoc > -1)
            {
                // Since newLineLoc is after the gap, we need to transform it to user coords
                // before setting the point.
                newLineLoc -= gap_buffer_gap_length(&buffer->gapBuffer);
                
                buffer_point_set(newLineLoc);
                return true;
            }
        }
        
        // If the gap is before the point
        else if (buffer->point > buffer->gapBuffer.left)
        {
            s32 gapI = gap_buffer_user_to_gap_coords(&buffer->gapBuffer, buffer->point);
            
            s32 n = buffer->gapBuffer.storageLength-gapI+1, m = string_length(stringArray[k]);
            wchar_t *arr = buffer->gapBuffer.storage;
            
            // Search from point to end
            s32 newLineLoc = string_search_naive_first_forward(arr, n, stringArray[k], m,
                                                               gapI+1, buffer->gapBuffer.storageLength);
            
            if (newLineLoc > -1)
            {
                // Since newLineLoc is after the gap, we need to transform it to user coords
                // before setting the point.
                newLineLoc -= gap_buffer_gap_length(&buffer->gapBuffer);
                
                buffer_point_set(newLineLoc);
                return true;
            }
        }
    }
    
    return false;
}

function bool 
buffer_search_backward(wchar_t **stringArray, s32 arrayN)
{
    Buffer *buffer = editor.world.currentBuffer;
    wchar_t *arr = buffer->gapBuffer.storage;
    
    for (s32 k = 0;
         k < arrayN;
         ++k)
    {
        bool found = false;
        s32 foundCharLoc = -1;
        
        // If the gap is after the point (or at the point)
        if (buffer->point <= buffer->gapBuffer.left)
        {
            // Search from point-1 to start
            s32 n = buffer->point-1, m = string_length(stringArray[k]);
            
            s32 foundLoc = string_search_naive_first_backward(arr, n, stringArray[k], m,
                                                              0, buffer->point-1);
            if (foundLoc > -1)
            {
                foundCharLoc = foundLoc;
                found = true;
            }
        }
        // If the gap is before the point
        else if (buffer->point > buffer->gapBuffer.left)
        {
            // Since this is after the gap, must account for the gap size in the index
            s32 gapI = gap_buffer_user_to_gap_coords(&buffer->gapBuffer, buffer->point);
            
            // First search from gapI-1 to gapRight
            s32 n = gapI-1-buffer->gapBuffer.right, m = string_length(stringArray[k]);
            
            s32 foundLoc = string_search_naive_first_backward(arr, n, stringArray[k], m,
                                                              buffer->gapBuffer.right, gapI-1);
            if (foundLoc > -1)
            {
                // Again, since it's after the gap, must convert back
                foundLoc -= gap_buffer_gap_length(&buffer->gapBuffer);
                
                foundCharLoc = foundLoc;
                found = true;
            }
            
            // Then search from gapLeft-1 to start
            n = buffer->gapBuffer.left-1, m = string_length(stringArray[k]);
            
            foundLoc = string_search_naive_first_backward(arr, n, stringArray[k], m,
                                                          0, buffer->gapBuffer.left-1);
            if (foundLoc > -1)
            {
                foundCharLoc = foundLoc;
                found = true;
            }
        }
        
        //
        if (found)
        {
            buffer_point_set(foundCharLoc);
            return true;
        }
    }
    
    return false;
}