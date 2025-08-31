#version 460 core

layout(local_size_x = 8, local_size_y = 1, local_size_z = 1) in;
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
    if(i>=p.length()) return;
    float px = p[i].pos.x;
    float py = p[i].pos.y;
    p[i].vel.y -= 9.8;
    float dt = p[i].extra.y;
    float xdelta = p[i].pos.x*2.0 - p[i].oldPos.x + p[i].vel.x * dt * dt;
    float ydelta = p[i].pos.y*2.0 - p[i].oldPos.y + p[i].vel.y * dt * dt;
    p[i].pos.x = xdelta;
    p[i].pos.y = ydelta;
    p[i].oldPos.x = px;
    p[i].oldPos.y = py;
    p[i].vel.y *= 0.98;
}
