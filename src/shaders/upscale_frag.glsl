// https://www.shadertoy.com/view/tsdcRM
/*Copyright 2020 Ethan Alexander Shulman
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and
this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED
"AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#version 460

layout(location = 0) in vec2 fragTexCoord;
layout(binding = 0) uniform sampler2D tex;

layout(location = 0) out vec4 outColor;

// upscaling multiplier amount
#define UPSCALE 4.0

// image mipmap level, for base upscaling
#define ML 0

// equality threshold of 2 colors before forming lines
#define THRESHOLD 0.01

// line thickness
float LINE_THICKNESS;

float w = 256.0 * 4.0;
float h = 144.0 * 4.0;

// anti aliasing scaling, smaller value make lines more blurry
#define AA_SCALE (UPSCALE * 1.8)

// draw diagonal line connecting 2 pixels if within threshold
bool diag(inout vec4 sum, vec2 uv, vec2 p1, vec2 p2)
{
  vec4 v1 = texelFetch(tex, ivec2(uv + vec2(p1.x, p1.y)), ML);
  vec4 v2 = texelFetch(tex, ivec2(uv + vec2(p2.x, p2.y)), ML);
  if (length(v1 - v2) < THRESHOLD)
  {
    vec2 dir = p2 - p1, lp = uv - (floor(uv + p1) + .5);
    dir     = normalize(vec2(dir.y, -dir.x));
    float l = clamp((LINE_THICKNESS - dot(lp, dir)) * AA_SCALE, 0., 1.);
    sum     = mix(sum, v1, l);
    return true;
  }
  return false;
}

void main()
{
  vec2 uv = fragTexCoord * vec2(w, h);
  vec2 ip = uv / UPSCALE;

  vec4 s = texelFetch(tex, ivec2(ip.x, ip.y), ML);

  LINE_THICKNESS = 0.5;
  if (diag(s, ip, vec2(-1, 0), vec2(0, 1)))
  {
    LINE_THICKNESS = 0.323;
    diag(s, ip, vec2(-1, 0), vec2(1, 1));
    diag(s, ip, vec2(-1, -1), vec2(0, 1));
  }
  LINE_THICKNESS = 0.4;
  if (diag(s, ip, vec2(0, 1), vec2(1, 0)))
  {
    LINE_THICKNESS = 0.323;
    diag(s, ip, vec2(0, 1), vec2(1, -1));
    diag(s, ip, vec2(-1, 1), vec2(1, 0));
  }
  LINE_THICKNESS = 0.3;
  if (diag(s, ip, vec2(1, 0), vec2(0, -1)))
  {
    LINE_THICKNESS = 0.323;
    diag(s, ip, vec2(1, 0), vec2(-1, -1));
    diag(s, ip, vec2(1, 1), vec2(0, -1));
  }
  LINE_THICKNESS = 0.3;
  if (diag(s, ip, vec2(0, -1), vec2(-1, 0)))
  {
    LINE_THICKNESS = 0.323;
    diag(s, ip, vec2(0, -1), vec2(-1, 1));
    diag(s, ip, vec2(1, -1), vec2(-1, 0));
  }

  outColor   = s;
}
