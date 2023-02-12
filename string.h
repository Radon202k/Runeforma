#pragma once

function s32
string_search_naive_first_forward(wchar_t *text, s32 n, wchar_t *pattern, s32 m,
                                  s32 l, s32 r)
{
    if ((r-l) <= n && m <= n)
    {
        s32 i=0, j=0, k=0;
        
        // From left to right
        for (i = l; i <= r; i++)
        {
            k = i;
            
            // Test every character for equality
            for (j = 0; (j < m) && (text[k] == pattern[j]); j++)
            {
                k++;
            }
            
            // If the number of equal characters (j) is equal to the length of pattern (m)
            if (j == m)
            {
                // It means it matches at position (i)
                return i;
            }
        }
    }
    
    return -1;
}

function s32
string_search_naive_first_backward(wchar_t *text, s32 n, wchar_t *pattern, s32 m,
                                   s32 l, s32 r)
{
    if ((r-l) <= n && m <= n)
    {
        s32 i=0, j=0, k=0;
        
        // From right to left
        for (i = r; i >= l; i--)
        {
            k = i;
            
            // Test every character for equality
            for (j = 0; (j < m) && (text[k] == pattern[j]); j++)
            {
                k++;
            }
            
            // If the number of equal characters (j) is equal to the length of pattern (m)
            if (j == m)
            {
                // It means it matches at position (i)
                return i;
            }
        }
    }
    
    return -1;
}

// Naive algorithm to search for pattern in text
function void 
string_search_naive(char *text, s32 n, char *pattern, s32 m,
                    s32 **foundPositions, s32 *foundCount)
{
    (*foundPositions) = alloc_array(16, s32);
    
    *foundCount = 0;
    
    s32 lim = n-m+1;
    s32 i=0, j=0, k=0;
    for (i = 0; i <= lim; i++)
    {
        k = i;
        
        for (j = 0; (j <= m) && text[k] && (text[k] == pattern[j]); j++)
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
