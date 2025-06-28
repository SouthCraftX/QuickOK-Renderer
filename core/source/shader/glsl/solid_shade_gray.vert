// vertex.vert
#version 450

// 顶点输入
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_tex_coord;

// 输出到片段着色器
layout(location = 0) out vec2 out_tex_coord;

void main() {
    // 将顶点坐标直接作为裁剪空间坐标（适用于全屏四边形）
    gl_Position = vec4(in_position, 0.0, 1.0);
    
    // 传递纹理坐标
    out_tex_coord = in_tex_coord;
}