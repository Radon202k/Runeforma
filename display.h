#pragma once

function void
display_init(void)
{
    // Load fonts
    wchar_t fontFileName[260];
    build_absolute_path(fontFileName, 260, L"fonts/Inconsolata-Regular.ttf");
    wchar_t *fontName = L"Inconsolata";
    editor.font32 = font_create_from_file(fontFileName, fontName, 24);
    editor.font16 = font_create_from_file(fontFileName, fontName, 16);
    
    // Create sprites
    wchar_t whitePath[260];
    build_absolute_path(whitePath, 260, L"images/white.png");
    wchar_t debugFontPath[260];
    build_absolute_path(debugFontPath, 260, L"images/debugFont.png");
    editor.debugFont = sprite_create_from_file(debugFontPath);
    editor.white = sprite_create_from_file(whitePath);
    assert(editor.debugFont.exists);
    assert(editor.white.exists);
}

function void
display_render_frame(void)
{
    SpriteGroup *layer1 = sprite_group_get_layer(1);
    SpriteGroup *layer2 = sprite_group_get_layer(2);
    SpriteGroup *layer3 = sprite_group_get_layer(3);
    
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    
    float winWidth = engine.backBufferSize.x;
    float winHeight = engine.backBufferSize.y;
    
    Vector2 origin = world->origin;
    Vector2 bucketSize = world->bucketSize;
    
    // Draw buffer name
    float bufferNameWidth = string_length(buffer->bufferName)*bucketSize.x;
    draw_string(layer3, &editor.font32, 
                buffer->bufferName, 
                v2(engine.backBufferSize.x-bufferNameWidth-10, 
                   1.0f*winHeight-1.2f*bucketSize.y), 
                rgba(.5f,.5f,.5f,1), 11);
    
    if (editor.showGap)
    {
        gap_buffer_draw_with_gap(&buffer->gapBuffer, buffer->point, buffer->mark,
                                 origin, bucketSize);
    }
    else
    {
        float contentH = gap_buffer_content_height(&buffer->gapBuffer, bucketSize.y);
        
        // Draw the text
        gap_buffer_draw(&buffer->gapBuffer, buffer->firstLineCharP, 
                        buffer->point, buffer->mark,
                        origin, bucketSize);
    }
    
    
    // draw dt label
    draw_label_float(layer1, &editor.font32,
                     engine.dt, 
                     v2(engine.backBufferSize.x-100,32), 
                     1, rgba(1,1,0,1), 1, false);
    
    
    // Draw last line label
    draw_label_int(layer1, &editor.font32,
                   buffer->lastLineCharP, 
                   v2(engine.backBufferSize.x-100,2*32), 
                   1, rgba(1,0,0,1), 1, false);
    
    // Draw first line label
    draw_label_int(layer1, &editor.font32,
                   buffer->firstLineCharP, 
                   v2(engine.backBufferSize.x-100,3*32), 
                   1, rgba(1,1,0,1), 1, false);
    
}
