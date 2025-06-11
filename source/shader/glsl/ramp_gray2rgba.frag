#version 300 es
precision mediump float;

uniform sampler2D   u_gray_tex;
uniform sampler2D   u_color_ramp;
uniform float       u_ramp_offset;
uniform float       u_ramp_scale;

in  vec2            v_tex_coord;
out vec4            frag_color;

void 
main()
{
    float   gray = texture(u_gray_tex , v_tex_coord).r;
    float   ramp_pos = clamp(gray * u_ramp_scale + u_ramp_offset , 0.0, 1.0);
    vec4    color = texture(u_color_ramp , vec2(ramp_pos , 0.5));
}