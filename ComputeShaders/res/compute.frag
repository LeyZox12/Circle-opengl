#version 460 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
struct Point{
vec4 pos;
vec4 oldPos;
vec4 vel;
vec4 extra;
};
layout(std430, binding=0) buffer pts{
Point p[];
};
layout(std430, binding=1) buffer rectangles{
vec4 rects[];
};
void main(){
   uint i = gl_GlobalInvocationID.x;
   uint j = gl_GlobalInvocationID.y;
   if(j == i){
       float px = p[i].pos.x;
       float py = p[i].pos.y;
       p[i].vel.y -= 9.8;
       float xdelta = p[i].pos.x * 2.0 - p[i].oldPos.x + p[i].vel.x * p[i].extra.y * p[i].extra.y;
       float ydelta = p[i].pos.y * 2.0 - p[i].oldPos.y + p[i].vel.y * p[i].extra.y * p[i].extra.y;
       p[i].pos.x = xdelta;
       p[i].pos.y = ydelta;
       p[i].oldPos.x = px;
       p[i].oldPos.y = py;
       p[i].vel.y *= 0.98;
   }
   else
   {
       float r1 = p[i].extra.x;
       float r2 = p[j].extra.x;
       float rc = p[i].extra.x + p[j].extra.x;
       vec2 p1 = vec2(p[i].pos.x, p[i].pos.y);
       vec2 p2 = vec2(p[j].pos.x, p[j].pos.y);
       vec2 dir = p1 - p2;
       float dist = distance(p1, p2);
       if(dist > 0.0 && dist < rc)
       {
           dir /= dist;
           float difference = rc - dist;
           float ratio1 = r2 / r1;
           float ratio2 = r1 / r2;
           vec2 fixed1;
           vec2 fixed2;
           if(ratio1 < ratio2)
           {
               fixed1 = p1 - (rc - dist) * ratio1 * -dir;
               fixed2 = p2 - (rc - dist) * (1.f - ratio1) * dir;
           }
           else if(ratio2 < ratio1)
           {
               fixed1 = p1 - (rc - dist) * (1.f - ratio2) * -dir;
               fixed2 = p2 - (rc - dist) * ratio2 * dir;
           }
           else
           {
               fixed1 = p1 - difference * 0.1f * -dir;
               fixed2 = p2 - difference * 0.1f * dir;
           }
           p[i].pos = vec4(fixed1.x, fixed1.y, 0.0, 0.0);
           p[j].pos = vec4(fixed2.x, fixed2.y, 0.0, 0.0);
       }
       for(int r = 0; r < 3; r++)
       {
           if(p[i].pos.x + p[i].extra.x > rects[r].x && p[i].pos.x - p[i].extra.x < rects[r].x + rects[r].z && p[i].pos.y - p[i].extra.x < rects[r].y)
           {
               float distances[4] = {abs(rects[r].x - (p[i].pos.x+p[i].extra.x)), abs((p[i].pos.x-p[i].extra.x) - (rects[r].x + rects[r].z)),
                                     abs(rects[r].y - (p[i].pos.y+p[i].extra.x)), abs((p[i].pos.y-p[i].extra.x) - (rects[r].y + rects[r].w))};
               float minV = min(distances[0], min(distances[1], min(distances[2], distances[3]))); 
               if(minV == distances[2]) p[i].pos.y = rects[r].y + p[i].extra.x;
           }
       }
   }
}
