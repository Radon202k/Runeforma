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
    Buffer *buffer = 0;
    
    File testFile = read_file("test.txt");
    
    if (!testFile.exists)
    {
        buffer = buffer_create("Scratch", "Runeforma scratch buffer", (s32)strlen("Runeforma scratch buffer")+1);
    }
    else
    {
        buffer = buffer_create(testFile.name, (char *)testFile.data, testFile.size);
    }
    
    world->bufferChain = buffer;
    world->currentBuffer = world->bufferChain;
    
    
    int y = 0;
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
