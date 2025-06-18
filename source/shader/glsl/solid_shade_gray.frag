// fragment.frag
#version 450
//#extension GL_KHR_vulkan_glsl : require
// 声明中等精度，对某些移动GPU有性能提升，且对于颜色处理完全足够
precision mediump float;

// --- 输入 ---
// 从顶点着色器接收纹理坐标
layout(location = 0) in vec2 in_tex_coord;

// --- 描述符绑定 ---
// 绑定0：单通道的灰度纹理
layout(set = 0, binding = 0) uniform sampler2D u_grayscale_texture;

// --- Push Constants ---
// 使用Push Constants来接收底色，这是最高效的方式
layout(push_constant) uniform push_constants {
    // 我们只需要一个三维向量表示RGB颜色
    vec3 base_color;
} push_consts;


// --- 输出 ---
// 输出到颜色附件0
layout(location = 0) out vec4 outColor;

void main() {
    // 1. 从单通道纹理中读取灰度值。
    //    我们只关心.r通道（或者.x），因为它是单通道纹理。
    float grayscale_value = texture(u_grayscale_texture, in_tex_coord).r;

    // 2. 核心操作：将底色与灰度值相乘。
    //    这会将灰度值作为亮度来调制底色。
    //    - 当 grayscale_value = 0.0 (黑), 结果为 vec3(0.0) (黑).
    //    - 当 grayscale_value = 1.0 (白), 结果为 base_color (原始底色).
    //    - 当 grayscale_value = 0.5 (灰), 结果为 base_color * 0.5 (半亮度的底色).
    vec3 final_rgb = push_consts.base_color * grayscale_value;

    // 3. 输出最终颜色。Alpha通道通常设为1.0（不透明）。
    outColor = vec4(final_rgb, 1.0);
}