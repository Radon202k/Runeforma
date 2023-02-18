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

// Include the other files
#include "string.h"
#include "subeditor.h"
#include "display.h"
#include "usercommands.h"
#include "animator.h"

// Init is called once by Engine2D at startup
function void init()
{
    // Init the display
    display_init();
    
    // Init the world
    world_init(&editor.world);
    
    // Animation test stuff
    animator_init_v2(&editor.test, 1, v2(1,-0.66f), v2(.3f,.99f),
                     v2(.25f*engine.backBufferSize.x, .5f*engine.backBufferSize.y),
                     v2(.75f*engine.backBufferSize.x, .5f*engine.backBufferSize.y));
}

// Update is called once per frame by Engine2D
function void update()
{
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    SpriteGroup *layer2 = sprite_group_push_layer(2);
    
    Buffer *buffer = editor.world.currentBuffer;
    s32 bufferSize = gap_buffer_current_length(&buffer->gapBuffer);
    
    if (editor.dragging && !engine.mouse.left.down)
    {
        editor.dragging = false;
    }
    
    handle_user_navigation();
    
    // Render the current frame
    display_render_frame();
    
    // Anim test stuff
    Vector2 rectPos = animator_update_v2(&editor.test);
    
    // Draw the rect at the lerped position
    draw_rect(layer1, editor.white, rectPos, 
              v2(10,10), rgba(1,0,0,1), 0);
    
    // draw anim t label 
    draw_label_float(layer1, &editor.font32,
                     editor.test.t, 
                     v2(engine.backBufferSize.x-100,2*32), 
                     1, rgba(1,1,0,1), 1, false);
    
    // Draw the animation curve
    animator_draw_bezier_curve(&editor.test);
    
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