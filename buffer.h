#pragma once

function Buffer *buffer_create(char *bufferName, char *data)
{
    // Allocate a new buffer
    Buffer *buffer = (Buffer *)malloc(sizeof(Buffer));
    
    // Clear it to zero
    memset(buffer, 0, sizeof(Buffer));
    
    // Set its name 
    strcpy_s(buffer->bufferName, 512, bufferName);
    
    // Define default values
    u32 length = (u32)strlen(data);
    
    assert(length < 40000);
    
    buffer->currentLine = 1;
    buffer->numChars = length;
    buffer->numLines = 1;
    
    // Mark *markList;
    
    // Init the internal gap buffer
    gap_buffer_init(&buffer->contents, length*2, data, &buffer->point);
    
    // Set a temp filename
    strcpy_s(buffer->fileName, 512, bufferName);
    
    return buffer;
}

function bool buffer_save(Buffer *buffer)
{
    u32 dataSize = gap_buffer_current_size(&buffer->contents);
    
    char *data = (char *)malloc(dataSize + 1);
    
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
    
    return write_file(buffer->fileName,
                      data,
                      dataSize+1);
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

function bool buffer_point_set(Buffer *buffer, s32 loc)
{
    buffer->point = loc;
}

function bool buffer_point_move(Buffer *buffer, s32 count)
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

/* This function uses the naive brute force algorithm to search for needles in the string
Text represent the string, n is its length
Pat represent the pattern/needle, m is its length
*/
function void 
string_search_naive(char *text, s32 n, char *pat, s32 m,
                    s32 **foundPositions, s32 *foundCount)
{
    (*foundPositions) = (s32 *)malloc(16*sizeof(s32));
    memset((*foundPositions), 0, 16*sizeof(s32));
    
    *foundCount = 0;
    
    s32 lim = n-m+1;
    s32 i=0, j=0, k=0;
    for (i = 0; i <= lim; i++)
    {
        k = i;
        
        for (j = 0; (j <= m) && text[k] && (text[k] == pat[j]); j++)
        {
            k++;
        }
        
        if (j == m)
        {
            // Register position where found occurence
            (*foundPositions)[(*foundCount)] = i-j+1;
            
            // Increment occurences found
            (*foundCount) = (*foundCount) + 1;
        }
    }
}

function bool buffer_search_forward(Buffer *buffer, char *string)
{
    // Start at the point
    s32 gapP = gap_buffer_user_to_gap_coords(&buffer->contents, buffer->point);
    
    // If the gap is after the point
    if (buffer->point < buffer->contents.gapLeft)
    {
        // Search from point to gapLeft
        char *at = buffer->contents.array + gapP;
        while (at != buffer->contents.array + buffer->contents.gapLeft)
        {
            
            
            at++;
        }
        
        // Then search from gapRight to end
        
    }
    // If the gap is before the point
    else if (buffer->point > buffer->contents.gapLeft)
    {
        // Search from point to end
    }
}