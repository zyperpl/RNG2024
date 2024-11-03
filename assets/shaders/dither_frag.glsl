#version 330

uniform sampler2D texture2;
uniform sampler2D texture0;
uniform sampler2D texture1;
uniform vec2 lightPos[32];
uniform vec2 cameraPos;
uniform vec2 resolution;
uniform float lightSize[32];
uniform float lightIntensity[32];

in vec2 fragTexCoord;
layout(location = 0) out vec4 fragColor;

float Bayer2(vec2 a)
{
    return fract((a.x * 0.5) + ((a.y * a.y) * 0.75));
}

vec3 GetDitheredPalette(inout float x, float y, vec2 pixel)
{
    float ts = float(textureSize(texture2, 0).y);
    float idx = clamp(y, 0.0, 1.0) * (ts - 1.0);
    float nidx = idx + 1.0;
    vec3 c1 = vec3(0.0);
    vec3 c2 = vec3(0.0);
    x /= float(textureSize(texture2, 0).x);
    c1 = texture(texture2, vec2(x, idx / ts)).xyz;
    c2 = texture(texture2, vec2(x, nidx / ts)).xyz;
    vec2 param = pixel * 0.5;
    vec2 param_1 = pixel;
    float dith = (Bayer2(param) * 0.25) + Bayer2(param_1);
    float mixAmt = float(fract(idx) > dith);
    return mix(c1, c2, vec3(mixAmt));
}

void main()
{
    vec2 uv = fragTexCoord;
    vec4 t = texture(texture0, uv);
    if (t.w == 0.0)
    {
        discard;
    }
    vec4 n = texture(texture1, uv);
    vec3 normal = normalize((n.xyz * 2.0) - vec3(1.0));
    float totalLight = 0.0;
    for (int i = 0; i < 32; i++)
    {
        vec2 lp = vec2(lightPos[i].x - cameraPos.x, (resolution.y - lightPos[i].y) + cameraPos.y);
        vec2 lightDir = normalize(lp - gl_FragCoord.xy);
        float lightDist = length(lp - gl_FragCoord.xy) / 40.0;
        float intensity = clamp(pow(lightDist, -4.0) * lightSize[i], 0.0, 10.0) * lightIntensity[i];
        vec3 light = normalize(vec3(lightDir, 1.0));
        float diffuse = min(max(dot(normal, light), 0.0), 0.89999997615814208984375);
        totalLight += (diffuse * intensity);
    }
    float param = (t.z * 255.0) / 30.0;
    float param_1 = pow(t.x, 3.0) * totalLight;
    vec2 param_2 = gl_FragCoord.xy + vec2(cameraPos.x, -cameraPos.y);
    vec3 _263 = GetDitheredPalette(param, param_1, param_2);
    vec3 color = _263;
    fragColor = vec4(color, 1.0);
    fragColor.w = 1.0;
}

