#version 450

// layout(location = ...) 用于从CPU传递顶点数据，但对于全屏绘制，我们可以在内部生成
layout(location = 0) out vec2 out_uv;

void main() {
    // 生成一个覆盖屏幕的三角形，无需顶点缓冲
    // gl_VertexIndex 是内置变量，表示当前处理的顶点索引 (0, 1, 2)
    // 这个技巧可以不使用VBO/IBO就画出一个全屏三角形
    out_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(out_uv * 2.0 - 1.0, 0.0, 1.0);
    
    // Y轴翻转，以匹配Vulkan的坐标系
    gl_Position.y = -gl_Position.y; 
}