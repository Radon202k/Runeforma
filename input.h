#pragma once

function void
input_update(void)
{
    World *world = &editor.world;
    Buffer *buffer = world->currentBuffer;
    s32 bufferLength = gap_buffer_length(&buffer->gapBuffer);
    
    // Scroll bar input
    if (engine.mouse.wheel != 0)
    {
        editor.scrollBarPoint += engine.dt*engine.mouse.wheel;
    }
    
    // [Up Arrow]
    if (engine.key.up.pressed)
    {
        if (engine.key.control.down)
        {
            if (buffer->firstLineCharP > 0)
            {
                command_first_line_go_to_previous();
            }
        }
        else
        {
            command_go_to_previous_line();
        }
    }
    
    // [Down Arrow]
    if (engine.key.down.pressed)
    {
        if (engine.key.control.down)
        {
            if (buffer->firstLineCharP-1 != buffer->lastLineCharP)
            {
                command_first_line_go_to_next();
            }
        }
        else
        {
            command_go_to_next_line();
        }
    }
    
    // [Left Arrow]
    if (engine.key.left.pressed)
    {
        if (engine.key.control.down)
        {
            command_go_to_previous_space_or_linebreak();
        } 
        else
        {
            command_go_to_previous_char();
        }
    }
    
    // [Right Arrow]
    if (engine.key.right.pressed)
    {
        if (engine.key.control.down)
        {
            command_go_to_next_space_or_linebreak();
        }
        else
        {
            command_go_to_next_char();
        }
    }
    
    // [Control] key is down
    if (engine.key.control.down)
    {
        // [C] key was pressed
        if (engine.key.c.pressed)
        {
            command_copy_range();
        }
        // [V] key was pressed
        else if (engine.key.v.pressed)
        {
            command_paste_range();
        }
        // [D] key was pressed
        if (engine.key.d.pressed)
        {
            command_delete_range();
        }
        
        // [Space] was pressed
        if (engine.key.space.pressed)
        {
            buffer->mark = buffer->point;
        }
        
        // [S] key was pressed
        if (engine.key.s.pressed)
        {
            buffer_write();
        }
    }
    // If control is not down
    else
    {
        // If char was inputed
        if (engine.inputCharEntered)
        { 
            command_insert_char();
        }
        
        // If backspace key was pressed
        if (engine.key.backspace.pressed)
        {
            command_backspace();
        }
    }
    
    if (engine.key.alt.down)
    {
        if (engine.key.up.pressed)
        {
            /*
            wchar_t sasukePath[260];
            build_absolute_path(sasukePath, 260, L"images/sasuke.png");
            
            editor.sasuke = sprite_create_from_file(sasukePath);
            assert(editor.sasuke.exists);
            
            u8 *bytes = atlas_get_bytes();
            atlas_update(bytes);
            
            editor.sasukeUploaded = true;
        */
        }
    }
    
    if (engine.key.alt.down)
    {
        if (engine.key.f1.pressed)
        {
            editor.showGap = !editor.showGap;
        }
    }
}