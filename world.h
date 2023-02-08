#pragma once

function void world_save(World *world)
{
    // TODO: Save the world state to a file
}

function void world_load(World *world)
{
    // TODO: Load the world state from a file
}

function void world_init(World *world)
{
    char *scratchBufferName = "Scratch";
    if (!buffer_create(scratchBufferName))
    {
        exit(1);
    }
    else
    {
        buffer_set_current(scratchBufferName);
        
        char fullPath[260];
        build_absolute_path(fullPath, 260, "test.txt");
        buffer_set_fileName(fullPath);
        buffer_set_name("test.txt");
        buffer_read();
    }
    
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
