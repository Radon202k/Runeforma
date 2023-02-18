#pragma once

function void
animator_base_update(Animator *anim)
{
    // Advance the animation t 
    anim->t += anim->tVel*engine.dt;
    
    // Clamp and reverse the velocity at boundaries [0,1]
    if (anim->t > 1)
    {
        anim->t = 1;
        anim->tVel *= -1;
    }
    else if (anim->t < 0)
    {
        anim->t = 0;
        anim->tVel *= -1;
    }
}

function Vector2
animator_evaluate_bezier_curve(Animator *anim, bool *inverted)
{
    // Assert the t value is in the valid range [0,1]
    assert(anim->t >= 0 && anim->t <= 1);
    
    float t = anim->t;
    
    // If the velocity is negative
    if (anim->tVel < 0)
    {
        // Invert the animation to make it go from b to a
        t = 1 - t;
        *inverted = true;
    }
    
    // Evaluate the bezier curve
    Vector2 bezierCurve = v2_bezier(anim->c0.x,anim->c0.y,anim->c1.x,anim->c1.y,t);
    
    return bezierCurve;
}

function void
animator_init_v2(Animator *anim, float velocity,
                 Vector2 c0, Vector2 c1,
                 Vector2 a, Vector2 b)
{
    anim->tVel = velocity;
    
    // Control points positions
    anim->c0 = c0;
    anim->c1 = c1;
    
    anim->a = alloc_type(Vector2);
    *((Vector2 *)(anim->a)) = a;
    
    anim->b = alloc_type(Vector2);
    *((Vector2 *)(anim->b)) = b;
}

function Vector2
animator_update_v2(Animator *anim)
{
    animator_base_update(anim);
    
    // Get the positions a,b
    Vector2 *a = (Vector2 *)anim->a;
    Vector2 *b = (Vector2 *)anim->b;
    
    bool inverted = false;
    Vector2 bezier = animator_evaluate_bezier_curve(anim, &inverted);
    if (inverted)
    {
        b = (Vector2 *)anim->a;
        a = (Vector2 *)anim->b;
    }
    
    // Lerp the position
    Vector2 lerpedPos = v2_lerp(*a, bezier.y, *b);
    return lerpedPos;
}

function void
animator_update_and_render_cp(float px, float py, Vector2 *cp, Vector2 cpSize,
                              float windowX, float windowY,
                              float windowW, float windowH)
{
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    
    float cx = windowX+cp->x*windowW;
    float cy = windowY+cp->y*windowH;
    Vector2 controlSize = cpSize;
    
    if (fabs(engine.mouse.pos.x-cx)<100.0f &&
        fabs(engine.mouse.pos.y-cy)<100.0f)
    {
        controlSize = v2_mul(1.5f,controlSize);
        
        if (engine.mouse.left.down && !editor.dragging)
        {
            editor.dragging = true;
            editor.dragLastP = engine.mouse.pos;
            editor.draggingAddress = cp;
        }
    }
    
    if (editor.dragging && editor.draggingAddress == cp)
    {
        Vector2 deltaP = v2_sub(engine.mouse.pos,editor.dragLastP);
        editor.dragLastP = engine.mouse.pos;
        *cp = v2_add(*cp,v2_mul(1.0f/windowW,deltaP));
    }
    
    // Draw the control point
    draw_rect(layer1, editor.white, 
              v2(cx-.5f*controlSize.x,cy-.5f*controlSize.y),
              controlSize,
              rgba(1,1,0,.8f), 1);
    
    // Draw the line from p0 to c0
    base_draw_line(px, py, cx, cy, 1,1,0,1, 2);
}

function void
animator_draw_bezier_curve(Animator *anim)
{
    SpriteGroup *layer1 = sprite_group_push_layer(1);
    SpriteGroup *layer2 = sprite_group_push_layer(2);
    
    float windowW = 200;
    float windowH = 200;
    float windowX = .5f*engine.backBufferSize.x-.5f*windowW;
    float windowY = .5f*engine.backBufferSize.y-.5f*windowH;
    
    // Draw the window
    draw_rect(layer1, editor.white, 
              v2(windowX,windowY),
              v2(windowW,windowH),
              rgba(0.05f,0.05f,0.05f,.8f), 0);
    
    s32 lineCount = 10;
    float spacingX = windowW/lineCount;
    float spacingY = windowH/lineCount;
    
    Color lineCol = rgba(.2f,.2f,.2f,.8f);
    
    // Draw vertical lines
    float lineW = 1;
    float lineH = windowH;
    for (s32 i = 0;
         i <= lineCount;
         ++i)
    {
        float lineX = windowX+i*spacingX;
        float lineY = windowY;
        draw_rect(layer1, editor.white, 
                  v2(lineX,lineY),
                  v2(lineW,lineH),
                  lineCol, 0);
        
    }
    
    // Draw horizontal lines
    lineW = windowW+1;
    lineH = 1;
    for (s32 i = 0;
         i <= lineCount;
         ++i)
    {
        float lineX = windowX;
        float lineY = windowY+i*spacingY;
        draw_rect(layer1, editor.white, 
                  v2(lineX,lineY),
                  v2(lineW,lineH),
                  lineCol, 0);
        
    }
    
    float pW = 10;
    float pH = 10;
    
    // Draw p0 [0,0]
    float p0x = windowX+0*windowW;
    float p0y = windowY+0*windowH;
    draw_rect(layer1, editor.white, 
              v2(p0x-.5f*pW,p0y-.5f*pH),
              v2(pW,pH),
              rgba(1,1,1,.8f), 1);
    
    // Draw p1 [1,1]
    float p1x = windowX+1*windowW;
    float p1y = windowY+1*windowH;
    draw_rect(layer1, editor.white, 
              v2(p1x-.5f*pW,p1y-.5f*pH),
              v2(pW,pH),
              rgba(1,1,1,.8f), 1);
    
    
    Vector2 cpSize = v2(pW,pH);
    
    // Draw c0 (first control point)
    animator_update_and_render_cp(p0x,p0y,&anim->c0,cpSize,
                                  windowX,windowY,
                                  windowW,windowH);
    
    // Draw c1 (second control point)
    animator_update_and_render_cp(p1x,p1y,&anim->c1,cpSize,
                                  windowX,windowY,
                                  windowW,windowH);
    
    // Draw the actual curve
    Vector2 lastPoint = v2(0,0);
    s32 segmentCount = 42;
    float drawT = 0;
    float drawTDelta = 1.0f/(segmentCount-1); 
    for (s32 i = 0;
         i < segmentCount;
         ++i)
    {
        Vector2 point = v2_bezier(anim->c0.x,anim->c0.y,
                                  anim->c1.x,anim->c1.y,drawT);
        if (i > 0)
        {
            base_draw_line(windowX+lastPoint.x*windowW,windowY+lastPoint.y*windowH,
                           windowX+point.x*windowW,windowY+point.y*windowH,
                           1,0,0,1,
                           3);
        }
        
        lastPoint = point;
        drawT += drawTDelta;
    }
    
    Color pointCol = rgba(1,1,0,.8f);
    // Evaluate the bezier curve
    bool inverted = false;
    Vector2 bezier = animator_evaluate_bezier_curve(anim, &inverted);
    if (inverted)
    {
        pointCol = rgba(1,0,1,.8f);
    }
    
    // Draw T in the curve
    float tx = windowX+bezier.x*windowW;
    float ty = windowY+bezier.y*windowH;
    draw_rect(layer1, editor.white, 
              v2(tx-.5f*pW,ty-.5f*pH),
              v2(pW,pH),
              pointCol, 1);
}