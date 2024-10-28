#version 460

layout(binding = 0) uniform sampler2D texture0; // base
layout(binding = 1) uniform sampler2D texture1; // normal
layout(binding = 2) uniform sampler2D texture2; // palette

layout(location = 0) in vec2 fragTexCoord;

layout(location = 30) uniform vec2 cameraPos;
layout(location = 31) uniform vec2 resolution;

#define MAX_LIGHTS 16
layout(location = 32) uniform vec2 lightPos[MAX_LIGHTS];
layout(location = 48) uniform float lightStrength[MAX_LIGHTS];

layout(location = 0) out vec4 fragColor;

float Bayer2(vec2 a)
{
  a = floor(a);
  return fract(a.x / 2. + a.y * a.y * .75);
}

#define Bayer4(a)  (Bayer2(.5 * (a)) * .25 + Bayer2(a))
#define Bayer8(a)  (Bayer4(.5 * (a)) * .25 + Bayer2(a))
#define Bayer16(a) (Bayer8(.5 * (a)) * .25 + Bayer2(a))
#define Bayer32(a) (Bayer16(.5 * (a)) * .25 + Bayer2(a))
#define Bayer64(a) (Bayer32(.5 * (a)) * .25 + Bayer2(a))

vec3 GetDitheredPalette(float x, float y, vec2 pixel)
{
  float ts   = textureSize(texture2, 0).y;
  float idx  = clamp(y, 0.0, 1.0) * (ts - 1.0);
  float nidx = idx + 1.0;

  vec3 c1 = vec3(0);
  vec3 c2 = vec3(0);

  x /= textureSize(texture2, 0).x;

  c1 = texture(texture2, vec2(x, idx / ts)).rgb;
  c2 = texture(texture2, vec2(x, nidx / ts)).rgb;

  float dith   = Bayer64(pixel);
  float mixAmt = float(fract(idx) > dith);

  return mix(c1, c2, mixAmt);
}

void main()
{
  vec2 uv = fragTexCoord;
  vec4 t  = texture(texture0, uv);

  if (t.a == 0.0)
    discard;

  vec4 n      = texture(texture1, uv);
  vec3 normal = normalize(n.xyz * 2.0 - 1.0);

  float totalLight = 0.0;

  for (int i = 0; i < MAX_LIGHTS; i++)
  {
    vec2 lp              = vec2(lightPos[i].x - cameraPos.x, resolution.y - lightPos[i].y - cameraPos.y);
    vec2 lightDir        = normalize(lp - gl_FragCoord.xy);
    float lightDist      = length(lp - gl_FragCoord.xy) / 40.0;
    float lightIntensity = pow(lightDist, -2.0) * lightStrength[i];

    vec3 light    = normalize(vec3(lightDir, 1.0));
    float diffuse = min(max(dot(normal, light), 0.0), 0.9);
    totalLight += diffuse * lightIntensity;
  }

  vec3 color = GetDitheredPalette(t.b * 255.0 / 30.0, pow(t.r, 3.0) * totalLight, gl_FragCoord.xy + cameraPos);

  fragColor = vec4(color, 1.0);

  fragColor.a = 1.0;
}
