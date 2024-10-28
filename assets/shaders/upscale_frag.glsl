#version 300 es
precision mediump float;
precision highp int;

uniform highp sampler2D tex;

in highp vec2 fragTexCoord;
layout(location = 0) out highp vec4 outColor;
highp float w;
highp float h;
highp float LINE_THICKNESS;

bool diag(inout highp vec4 sum, highp vec2 uv, highp vec2 p1, highp vec2 p2)
{
    highp vec4 v1 = texelFetch(tex, ivec2(uv + vec2(p1.x, p1.y)), 0);
    highp vec4 v2 = texelFetch(tex, ivec2(uv + vec2(p2.x, p2.y)), 0);
    if (length(v1 - v2) < 0.00999999977648258209228515625)
    {
        highp vec2 dir = p2 - p1;
        highp vec2 lp = uv - (floor(uv + p1) + vec2(0.5));
        dir = normalize(vec2(dir.y, -dir.x));
        highp float l = clamp((LINE_THICKNESS - dot(lp, dir)) * 7.19999980926513671875, 0.0, 1.0);
        sum = mix(sum, v1, vec4(l));
        return true;
    }
    return false;
}

void main()
{
    w = 1024.0;
    h = 576.0;
    highp vec2 uv = fragTexCoord * vec2(w, h);
    highp vec2 ip = uv / vec2(4.0);
    highp vec4 s = texelFetch(tex, ivec2(int(ip.x), int(ip.y)), 0);
    LINE_THICKNESS = 0.5;
    highp vec4 param = s;
    highp vec2 param_1 = ip;
    highp vec2 param_2 = vec2(-1.0, 0.0);
    highp vec2 param_3 = vec2(0.0, 1.0);
    bool _143 = diag(param, param_1, param_2, param_3);
    s = param;
    if (_143)
    {
        LINE_THICKNESS = 0.323000013828277587890625;
        highp vec4 param_4 = s;
        highp vec2 param_5 = ip;
        highp vec2 param_6 = vec2(-1.0, 0.0);
        highp vec2 param_7 = vec2(1.0);
        bool _155 = diag(param_4, param_5, param_6, param_7);
        s = param_4;
        highp vec4 param_8 = s;
        highp vec2 param_9 = ip;
        highp vec2 param_10 = vec2(-1.0);
        highp vec2 param_11 = vec2(0.0, 1.0);
        bool _164 = diag(param_8, param_9, param_10, param_11);
        s = param_8;
    }
    LINE_THICKNESS = 0.4000000059604644775390625;
    highp vec4 param_12 = s;
    highp vec2 param_13 = ip;
    highp vec2 param_14 = vec2(0.0, 1.0);
    highp vec2 param_15 = vec2(1.0, 0.0);
    bool _174 = diag(param_12, param_13, param_14, param_15);
    s = param_12;
    if (_174)
    {
        LINE_THICKNESS = 0.323000013828277587890625;
        highp vec4 param_16 = s;
        highp vec2 param_17 = ip;
        highp vec2 param_18 = vec2(0.0, 1.0);
        highp vec2 param_19 = vec2(1.0, -1.0);
        bool _185 = diag(param_16, param_17, param_18, param_19);
        s = param_16;
        highp vec4 param_20 = s;
        highp vec2 param_21 = ip;
        highp vec2 param_22 = vec2(-1.0, 1.0);
        highp vec2 param_23 = vec2(1.0, 0.0);
        bool _194 = diag(param_20, param_21, param_22, param_23);
        s = param_20;
    }
    LINE_THICKNESS = 0.300000011920928955078125;
    highp vec4 param_24 = s;
    highp vec2 param_25 = ip;
    highp vec2 param_26 = vec2(1.0, 0.0);
    highp vec2 param_27 = vec2(0.0, -1.0);
    bool _204 = diag(param_24, param_25, param_26, param_27);
    s = param_24;
    if (_204)
    {
        LINE_THICKNESS = 0.323000013828277587890625;
        highp vec4 param_28 = s;
        highp vec2 param_29 = ip;
        highp vec2 param_30 = vec2(1.0, 0.0);
        highp vec2 param_31 = vec2(-1.0);
        bool _214 = diag(param_28, param_29, param_30, param_31);
        s = param_28;
        highp vec4 param_32 = s;
        highp vec2 param_33 = ip;
        highp vec2 param_34 = vec2(1.0);
        highp vec2 param_35 = vec2(0.0, -1.0);
        bool _222 = diag(param_32, param_33, param_34, param_35);
        s = param_32;
    }
    LINE_THICKNESS = 0.300000011920928955078125;
    highp vec4 param_36 = s;
    highp vec2 param_37 = ip;
    highp vec2 param_38 = vec2(0.0, -1.0);
    highp vec2 param_39 = vec2(-1.0, 0.0);
    bool _230 = diag(param_36, param_37, param_38, param_39);
    s = param_36;
    if (_230)
    {
        LINE_THICKNESS = 0.323000013828277587890625;
        highp vec4 param_40 = s;
        highp vec2 param_41 = ip;
        highp vec2 param_42 = vec2(0.0, -1.0);
        highp vec2 param_43 = vec2(-1.0, 1.0);
        bool _240 = diag(param_40, param_41, param_42, param_43);
        s = param_40;
        highp vec4 param_44 = s;
        highp vec2 param_45 = ip;
        highp vec2 param_46 = vec2(1.0, -1.0);
        highp vec2 param_47 = vec2(-1.0, 0.0);
        bool _248 = diag(param_44, param_45, param_46, param_47);
        s = param_44;
    }
    outColor = s;
}

