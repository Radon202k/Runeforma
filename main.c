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

// Init is called once by Engine2D at startup
function void init()
{
    // Init the display
    display_init();
    
    // Init the world
    world_init(&editor.world);
    
    animator_init_v2(&editor.pointPosAnimator, 5, v2(.02f,.96f), v2(.24f,.98f),
                     v2(0,0), v2(100,0));
    
    animator_init_v2(&editor.originAnimator, 5, v2(.02f,.96f), v2(.24f,.98f),
                     v2(0,0), v2(100,0));
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
    
    Buffer *buffer = editor.world.currentBuffer;
    s32 bufferSize = gap_buffer_length(&buffer->gapBuffer);
    
    input_update();
    
    // Render the current frame
    display_render_frame();
    
#if 0
    // TODO: Update to handle variable char widths
    // Search for occurences of the word "to"
    s32 *foundPositions, foundCount;
    string_search_naive(buffer->gapBuffer.storage, 
                        buffer->gapBuffer.left, "to", 2,
                        &foundPositions, &foundCount);
    
    if (foundCount > 0)
    {
        for (s32 i = 0; i < foundCount; ++i)
        {
            s32 loc = foundPositions[i];
            
            Vector2 bucketPos = 
                gap_buffer_point_to_screen_pos(&buffer->gapBuffer, loc, 
                                               origin, bucketSize);
            
            draw_rect(layer1, editor.white, bucketPos, 
                      v2(bucketSize.x*2, bucketSize.y), rgba(1,0,0,0.5f), 1);
        }
    }
    
    
    // Has to free the allocated array
    free(foundPositions);
    
#endif
    
}