#pragma once

function void
base_draw_char(char c, float x, float y, float size,
               float r, float g, float b, float a,
               float layer)
{
    float uvWidth = editor.font.uv.max.x - editor.font.uv.min.x;
    float uvHeight = editor.font.uv.max.y - editor.font.uv.min.y;
    
    float charUVwidth = (uvWidth / 16);
    float charUVheight = (uvHeight / 16);
    
    int charX = ((int)c) % 16;
    int charY = ((int)c) / 16;
    
    base_draw_rect(x, y,
                   size, size,
                   editor.font.uv.min.x + charX*charUVwidth, editor.font.uv.min.y + charY*charUVheight,
                   editor.font.uv.min.x + (charX+1)*charUVwidth, editor.font.uv.min.y + (charY+1)*charUVheight,
                   r,g,b,a,
                   layer);
}

function void
draw_char(char c, Vector2 pos, float size, Color col, float layer)
{
    base_draw_char(c, 
                   pos.x, pos.y,
                   size,
                   col.r, col.g, col.b, col.a,
                   layer);
}

function void
draw_string(char *string, Vector2 pos, float size, Color col, float layer)
{
    char *at = string;
    while (*at)
    {
        draw_char(*at++, pos, size, col, layer);
        pos.x += size;
    }
}
