#version 460 core
#extension GL_NV_shader_atomic_float : require
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
struct Point{
vec4 pos;
vec4 oldPos;
vec4 vel;
vec4 extra;
};
layout(std430, binding = 0) buffer pts{
Point p[];
};

layout(std430, binding = 1) buffer rectangles{
vec4 rects[];
};

void main()
{
    uint i = gl_GlobalInvocationID.x;
    uint j = gl_GlobalInvocationID.y;
    if(i>=p.length() || j>=p.length()) return;
    else if(j==i)
    {
        float px = p[i].pos.x;
        float py = p[i].pos.y;
        p[i].vel.y -= 9.8;
        float dt = p[i].extra.y;
        float xdelta = p[i].pos.x - p[i].oldPos.x + p[i].vel.x * dt * dt;
        float ydelta = p[i].pos.y - p[i].oldPos.y + p[i].vel.y * dt * dt;
        atomicAdd(p[i].pos.x, xdelta);
        atomicAdd(p[i].pos.y, ydelta);
        p[i].oldPos.x = px;
        p[i].oldPos.y = py;
        p[i].vel.y *= 0.98;
    }
    else if(j > i)
    for(int _ = 0; _ < 10; _++)
    {
        float r1 = p[i].extra.x;
        float r2 = p[j].extra.x;
        float rc = r1 + r2;
        vec2 p1 = p[i].pos.xy;
        vec2 p2 = p[j].pos.xy;
        vec2 dir = p1 - p2;
        float dist = distance(p1, p2);
        if(dist > 0.0 && dist < rc)
        {
            dir /= dist;
            float difference = rc - dist;
            vec2 fixed1 = -difference * 0.1f * -dir;
            vec2 fixed2 = - difference * 0.1f * dir;
            atomicAdd(p[i].pos.x,fixed1.x);
            atomicAdd(p[i].pos.y,fixed1.y);
            
            atomicAdd(p[j].pos.x,fixed2.x);
            atomicAdd(p[j].pos.y,fixed2.y);
        }
        for(int r = 0; r < 3; r++)
        {
            if(p[i].pos.x + p[i].extra.x > rects[r].x && p[i].pos.x - p[i].extra.x < rects[r].x + rects[r].z && p[i].pos.y - p[i].extra.x < rects[r].y)
            {
                float distances[4] =
                {
                    abs(rects[r].x - (p[i].pos.x + r1)),
                    abs((p[i].pos.x - r1) - (rects[r].x + rects[r].z)),
                    abs(rects[r].y - (p[i].pos.y + r1)),
                    abs((p[i].pos.y - r1) - (rects[r].y + rects[r].w))
                };
                float minV = min(distances[0], min(distances[1], min(distances[2], distances[3])));
                if(minV == distances[2]) p[i].pos.y = rects[r].y + p[i].extra.x;
                else if(minV == distances[0]) p[i].pos.x = rects[r].x - p[i].extra.x;
                else if(minV == distances[1]) p[i].pos.x = rects[r].x + rects[r].z + p[i].extra.x;
            }
        }
    }
}
