#pragma once

function void
base_debug_draw_char(SpriteGroup *group, Sprite font,
                     wchar_t c, float x, float y, float size,
                     float r, float g, float b, float a,
                     float layer)
{
    float uvWidth = font.uv.max.x - font.uv.min.x;
    float uvHeight = font.uv.max.y - font.uv.min.y;
    
    float charUVwidth = (uvWidth / 16);
    float charUVheight = (uvHeight / 16);
    
    int charX = ((int)c) % 16;
    int charY = ((int)c) / 16;
    
    base_draw_rect(group,
                   x, y,
                   size, size,
                   font.uv.min.x + charX*charUVwidth, font.uv.min.y + charY*charUVheight,
                   font.uv.min.x + (charX+1)*charUVwidth, font.uv.min.y + (charY+1)*charUVheight,
                   r,g,b,a,
                   layer);
}

function void
debug_draw_char(SpriteGroup *group, Sprite font,
                wchar_t c, Vector2 pos, float size, Color col, float layer)
{
    base_debug_draw_char(group, font,
                         c, 
                         pos.x, pos.y,
                         size,
                         col.r, col.g, col.b, col.a,
                         layer);
}

function void
debug_draw_string(SpriteGroup *group, Sprite font, 
                  wchar_t *string, Vector2 pos, float size, Color col, float layer)
{
    wchar_t *at = string;
    while (*at)
    {
        debug_draw_char(group, font, *at++, pos, size, col, layer);
        pos.x += size;
    }
}

function void
draw_label_int(SpriteGroup *group, TruetypeFont *font,
               s32 value, Vector2 pos, float size, Color col, float layer,
               bool centered)
{
    // Get a string for the number using _itoa_s
    wchar_t number[32] = {0};
    _itow_s(value, number, 32, 10);
    
    // Claculate the char size and position
    Vector2 charPos = pos;
    
    if (centered)
    {
        charPos.x -= string_length(number) * 0.5f*size;
    }
    
    draw_string(group, font, number, 
                v2(charPos.x, charPos.y - 16.0f), col, layer);
}

function void
draw_label_v2i(SpriteGroup *group, TruetypeFont *font,
               Vector2i value, Vector2 pos, float size, Color col, float layer,
               bool centered)
{
    // Get a string for the number using _itoa_s
    wchar_t xs[15] = {0};
    _itow_s(value.x, xs, 16, 10);
    
    wchar_t ys[15] = {0};
    _itow_s(value.y, ys, 16, 10);
    
    wchar_t number[32] = {0};
    string_append(number, 32, xs);
    string_append(number, 32, L", ");
    string_append(number, 32, ys);
    
    // Calculate the char size and position
    Vector2 charPos = pos;
    
    if (centered)
    {
        charPos.x -= string_length(number)*0.5f*size;
    }
    
    draw_string(group, font, 
                number, 
                v2(charPos.x, charPos.y - 16.0f), col, layer);
    
}
