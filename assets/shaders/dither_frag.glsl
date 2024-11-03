#version 300 es
precision mediump float;
precision highp int;

uniform highp sampler2D texture2;
uniform highp sampler2D texture0;
uniform highp sampler2D texture1;
uniform highp vec2 lightPos[32];
uniform highp vec2 cameraPos;
uniform highp vec2 resolution;
uniform highp float lightSize[32];
uniform highp float lightIntensity[32];

in highp vec2 fragTexCoord;
layout(location = 0) out highp vec4 fragColor;

highp float Bayer2(highp vec2 a)
{
    return fract((a.x * 0.5) + ((a.y * a.y) * 0.75));
}

highp vec3 GetDitheredPalette(inout highp float x, highp float y, highp vec2 pixel)
{
    highp float ts = float(textureSize(texture2, 0).y);
    highp float idx = clamp(y, 0.0, 1.0) * (ts - 1.0);
    highp float nidx = idx + 1.0;
    highp vec3 c1 = vec3(0.0);
    highp vec3 c2 = vec3(0.0);
    x /= float(textureSize(texture2, 0).x);
    c1 = texture(texture2, vec2(x, idx / ts)).xyz;
    c2 = texture(texture2, vec2(x, nidx / ts)).xyz;
    highp vec2 param = pixel * 0.5;
    highp vec2 param_1 = pixel;
    highp float dith = (Bayer2(param) * 0.25) + Bayer2(param_1);
    highp float mixAmt = float(fract(idx) > dith);
    return mix(c1, c2, vec3(mixAmt));
}

void main()
{
    highp vec2 uv = fragTexCoord;
    highp vec4 t = texture(texture0, uv);
    if (t.w == 0.0)
    {
        discard;
    }
    highp vec4 n = texture(texture1, uv);
    highp vec3 normal = normalize((n.xyz * 2.0) - vec3(1.0));
    highp float totalLight = 0.0;
    for (int i = 0; i < 32; i++)
    {
        highp vec2 lp = vec2(lightPos[i].x - cameraPos.x, (resolution.y - lightPos[i].y) + cameraPos.y);
        highp vec2 lightDir = normalize(lp - gl_FragCoord.xy);
        highp float lightDist = length(lp - gl_FragCoord.xy) / 40.0;
        highp float intensity = clamp(pow(lightDist, -4.0) * lightSize[i], 0.0, 10.0) * lightIntensity[i];
        highp vec3 light = normalize(vec3(lightDir, 1.0));
        highp float diffuse = min(max(dot(normal, light), 0.0), 0.89999997615814208984375);
        totalLight += (diffuse * intensity);
    }
    highp float param = (t.z * 255.0) / 30.0;
    highp float param_1 = pow(t.x, 3.0) * totalLight;
    highp vec2 param_2 = gl_FragCoord.xy + vec2(cameraPos.x, -cameraPos.y);
    highp vec3 _263 = GetDitheredPalette(param, param_1, param_2);
    highp vec3 color = _263;
    fragColor = vec4(color, 1.0);
    fragColor.w = 1.0;
}

