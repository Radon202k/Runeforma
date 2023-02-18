#pragma once

function void
world_save(World *world)
{
    // TODO: Save the world state to a file
}

function void
world_load(World *world)
{
    // TODO: Load the world state from a file
}

function void
world_init(World *world)
{
    wchar_t *scratchBufferName = L"Scratch";
    if (!buffer_create(scratchBufferName))
    {
        exit(1);
    }
    else
    {
        buffer_set_current(scratchBufferName);
#if 1
        wchar_t fullPath[260] = {0};
        build_absolute_path(fullPath, 260, L"test.txt");
        buffer_set_fileName(fullPath);
        buffer_set_name(L"test.txt");
        buffer_read();
        buffer_point_set(0);
#endif
    }
    
    editor.scrollBarPoint = 1;
    
    // Define a size to draw each bucket
    world->bucketSize = v2(editor.font32.charWidth, editor.font32.lineAdvance);
    
    // Calculate origin (bottom left) based on buffer size
    world->origin = v2(0, engine.backBufferSize.y-2.0f*world->bucketSize.y);
    
    world->origin.x += 2*world->bucketSize.x;
}

function void world_fini(World *world)
{
    Buffer *buffer = world->currentBuffer;
    while (buffer)
    {
        Buffer *nextPointer = buffer->nextChainEntry;
        free(buffer);
        
        buffer = nextPointer;
    }
    
    world_save(world);
}
