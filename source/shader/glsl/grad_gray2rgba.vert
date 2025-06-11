#version 300 es
precision mediump float;

in vec4 a_position;  // 顶点位置（x, y, z, w）
in vec2 a_tex_coord;  // 纹理坐标

out vec2 v_tex_coord; // 传递给片段着色器的纹理坐标

void main() {
    // 直接将顶点位置转换为裁剪空间坐标
    gl_Position = a_position;
    
    // 传递纹理坐标到片段着色器
    v_tex_coord = a_tex_coord;
}