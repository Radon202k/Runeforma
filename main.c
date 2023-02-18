// Include STB_Image library 
#define STB_IMAGE_IMPLEMENTATION
#include "w:\libs\stb_image.h"

// Define Engine2D configuration
#define SPRITE_ATLAS_SIZE 1024
#define BACKBUFFER_WIDTH 800
#define BACKBUFFER_HEIGHT 600
#define WINDOW_TITLE L"Runeforma"
#define CLEAR_COLOR rgba(.099f,.099f,.099f,1)

// Include Engine2D
#include "engine2d.h"

// This file defines all data structures
#include "datastructures.h"

global Editor editor;

// Include the other files
#include "render.h"
#include "animator.h"
#include "string.h"
#include "subeditor.h"
#include "display.h"
#include "usercommands.h"
#include "input.h"

function void
tokenizer_parse_string(wchar_t *input, TokenNode **head, int *count)
{
    TokenNode *current = 0;
    int length = string_length(input);
    
    // Allocate space for a temp buffer
    wchar_t *buffer = alloc_array((length+1),wchar_t);
    int bufferIndex = 0;
    
    bool inComment = false;
    
    // For each char
    for (int i = 0;
         i < length;
         i++) 
    {
        // If can find a double //
        if (input[i] == '/' && i+1 < length && input[i+1] == '/')
        {
            // Flag as inside a comment
            inComment = true;
            i++;
        } 
        // If hit a line break inside a comment
        else if (input[i] == '\n' && inComment) 
        {
            // Flag as end of the comment
            inComment = false;
        }
        
        // If is not a white space or is inside a comment
        if (!iswspace(input[i]) || inComment) 
        {
            // Add to the temp buffer
            buffer[bufferIndex++] = input[i];
        }
        
        // If it is a white space or the the end of the buffer
        if ((iswspace(input[i]) || i == length-1) && bufferIndex > 0) 
        {
            // End the temp buffer with null terminator
            buffer[bufferIndex] = L'\0';
            
            // Alloc a new token
            Token *token = alloc_type(Token);
            
            // Alloc the array of wchar_t
            token->value = alloc_array(bufferIndex+1, wchar_t);
            
            // Copy the string
            string_copy(token->value, bufferIndex+1, buffer);
            
            // Set the length
            token->length = bufferIndex;
            
            // Reset the temp buffer index
            bufferIndex = 0;
            
            // If inside a comment
            if (inComment)
            {
                // Flag the token as a comment token
                token->type = CTokenType_Comment;
            }
            // If is alphabetic character
            else if (iswalpha(token->value[0]))
            {
                // Flag the token as identifier
                token->type = CTokenType_Identifier;
                
                // If token is a keyword
                if (wcscmp(token->value, L"if") == 0 ||
                    wcscmp(token->value, L"else") == 0 ||
                    wcscmp(token->value, L"for") == 0 ||
                    wcscmp(token->value, L"while") == 0 ||
                    wcscmp(token->value, L"return") == 0 ||
                    wcscmp(token->value, L"void") == 0 ||
                    wcscmp(token->value, L"int") == 0 ||
                    wcscmp(token->value, L"double") == 0 ||
                    wcscmp(token->value, L"char") == 0) 
                {
                    // Flag token as keyword
                    token->type = CTokenType_Keyword;
                }
            }
            // If token is digit
            else if (iswdigit(token->value[0]))
            {
                // Flag as literal
                token->type = CTokenType_Literal;
            }
            // Else
            else
            {
                // Flag as operator
                token->type = CTokenType_Operator;
                
                // If token is a punctuation mark
                if (wcscmp(token->value, L";") == 0 ||
                    wcscmp(token->value, L",") == 0 ||
                    wcscmp(token->value, L"{") == 0 ||
                    wcscmp(token->value, L"}") == 0 ||
                    wcscmp(token->value, L"(") == 0 ||
                    wcscmp(token->value, L")") == 0)
                {
                    // Flag as punctuation
                    token->type = CTokenType_Punctuation;
                }
            }
            
            // Alloc the token node
            TokenNode *node = alloc_type(TokenNode);
            node->token = token;
            node->next = 0;
            
            // If there is no head in the list
            if (*head == 0)
            {
                // Set us as the head
                *head = node;
                // Set current as us
                current = node;
            } 
            // If there was a head already
            else 
            {
                // Put us in the end of the list
                current->next = node;
                
                // Update the end of the list as us
                current = node;
            }
            
            // Update the count
            (*count)++;
        }
    }
    
    // Free the temp buffer
    free(buffer);
}

// Init is called once by Engine2D at startup
function void init()
{
    // Init the display
    display_init();
    
    // Init the world
    world_init(&editor.world);
    
    // Cursor animators
    animator_init_v2(&editor.pointPosAnimator, 5, v2(.02f,.96f), v2(.24f,.98f),
                     v2(0,0),v2(0,0));
    
    animator_init_color(&editor.pointColAnimator, 1, v2(.02f,.96f), v2(.24f,.98f),
                        rgba(1,0,0,.2f), rgba(1,0,0,1), false, true);
    animator_play(&editor.pointColAnimator);
    
    // Origin animator
    animator_init_v2(&editor.originAnimator, 5, v2(.02f,.96f), v2(.24f,.98f),
                     v2(0,0),v2(0,0));
}

// Resized is called once by Engine2D after the window has been resized
function void resized()
{
    World *world = &editor.world;
    world->origin = v2(0, engine.backBufferSize.y-2.0f*world->bucketSize.y);
    world->origin.x += 2*world->bucketSize.x;
}

// Update is called once per frame by Engine2D
function void update()
{
    SpriteGroup *layer1 = sprite_group_get_layer(1);
    SpriteGroup *layer2 = sprite_group_get_layer(2);
    
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    s32 bufferSize = gap_buffer_length(&buffer->gapBuffer);
    
    // Input
    input_update();
    
    // Tokenizer
    TokenNode *tokenList = 0;
    s32 tokenCount = 0;
    wchar_t *windowContents = 0;
    buffer_get_range(buffer->firstLineCharP, buffer->lastVisibleCharP, &windowContents);
    tokenizer_parse_string(windowContents, &tokenList, &tokenCount);
    free(windowContents);
    
    // TODO: Render through tokens instead of directly using gap buffer
    
    // Render the current frame
    display_render_frame();
}