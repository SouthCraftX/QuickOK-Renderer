#version 450
#extension GL_ARB_separate_shader_objects : enable

// 定义一个最大支持的渐变点数量。这必须是编译时常量。
// 应用程序发送的点数量可以小于这个值，但不能超过。
#define MAX_GRADIENT_POINTS 32

// 输入的UV坐标，来自顶点着色器
layout(location = 0) in vec2 in_uv;

// 输出的最终颜色
layout(location = 0) out vec4 out_color;

// 定义单个渐变点的结构体
// 必须与C++中的结构体内存布局匹配 (std140)
struct GradientPoint {
    float position; // 在 [0, 1] 范围内的位置
    vec3 color;     // 该位置的RGB颜色
};

// 使用UBO传递渐变数据
// `binding = 0` 必须与Vulkan中描述符集的绑定点匹配
layout(binding = 0, std140) uniform GradientUBO {
    GradientPoint points[MAX_GRADIENT_POINTS];
    int point_count; // 实际使用的渐变点数量
};

// 核心函数：根据输入值 t 计算渐变颜色
vec3 get_gradient_color(float t) {
    // 处理边界情况
    if (point_count == 0) {
        return vec3(0.0, 0.0, 0.0); // 没有点，返回黑色
    }
    if (point_count == 1 || t <= points[0].position) {
        return points[0].color; // 只有一个点，或t在第一个点之前
    }
    if (t >= points[point_count - 1].position) {
        return points[point_count - 1].color; // t在最后一个点之后
    }

    // 循环查找 t 所在的颜色段
    // 我们需要找到第一个位置大于 t 的点
    for (int i = 1; i < point_count; ++i) {
        if (t <= points[i].position) {
            // 我们找到了段: [points[i-1], points[i]]
            GradientPoint left = points[i-1];
            GradientPoint right = points[i];

            // 重新归一化t到这个段的 [0, 1] 范围
            float segment_length = right.position - left.position;
            if (segment_length <= 0.0) {
                // 防止除以零
                return left.color;
            }
            float t_local = (t - left.position) / segment_length;

            // 在两个颜色之间进行线性插值
            return mix(left.color, right.color, t_local);
        }
    }

    // 理论上不应该执行到这里，但作为保障返回最后一个颜色
    return points[point_count - 1].color;
}

void main() {
    // 我们使用水平UV坐标 (in_uv.x) 作为渐变的输入值
    // 这是一个从左到右的渐变
    float t = in_uv.x;

    vec3 gradient_color = get_gradient_color(t);

    out_color = vec4(gradient_color, 1.0);
}