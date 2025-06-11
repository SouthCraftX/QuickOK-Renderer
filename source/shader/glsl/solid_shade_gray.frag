#version 300 es
precision mediump float;

in vec2 v_tex_coord;
out vec4 frag_color;

uniform sampler2D u_gray_tex;      // 灰度纹理（R通道）
uniform vec4 u_color;        // 起始颜色

void main() {
    float gray = texture(u_gray_tex, v_tex_coord).r;
    vec4 baseColor = u_color;
    frag_color = vec4(baseColor.rgb, baseColor.a * gray);
}
