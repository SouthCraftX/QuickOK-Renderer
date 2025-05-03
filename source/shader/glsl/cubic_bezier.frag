#version 300 es
precision mediump float;  // 使用 mediump 以提高性能（尤其在移动设备上）

uniform sampler2D   bezier_tex;  // 存储 Bézier 片段数据
uniform int         segment_count;      // Bézier 片段数量
out     vec4        frag_color;

// 计算三次 Bézier 曲线
vec2 cubic_bezier(
    vec2 P0 , 
    vec2 P1 , 
    vec2 P2 , 
    vec2 P3 , 
    float t
) {
    float u = 1.0 - t;
    return u * u * u * P0 + 3.0 * u * u * t * P1 + 3.0 * u * t * t * P2 + t * t * t * P3;
}

// AABB 判断，判断点是否在指定矩形范围内
bool is_inside_aabb(
    vec2 P , 
    vec2 min_bbox , 
    vec2 max_bbox
) {
    return P.x >= min_bbox.x && P.x <= max_bbox.x &&
           P.y >= min_bbox.y && P.y <= max_bbox.y;
}

// 二分法计算 Bézier 曲线的最小距离
float binary_search_bezier_distance(
    vec2 P , 
    vec2 P0 , 
    vec2 P1 , 
    vec2 P2 , 
    vec2 P3) 
{
    float t_min = 0.0 , t_max = 1.0;
    float min_dist = 1e6;
    float prev_dist = 1e6;

    for (int i = 0; i < 10; i++) {
        float t_mid = (t_min + t_max) * 0.5;
        vec2 B = cubic_bezier(P0 , P1 , P2 , P3 , t_mid);
        float dist = length(B - P);
        min_dist = min(min_dist , dist);

        // 如果距离变化很小，提前结束
        if (abs(dist - prev_dist) < 0.001) break;
        prev_dist = dist;

        // 判断曲线的方向，选择更新的 t 范围
        if (dot(B - P , P1 - P0) < 0.0) t_max = t_mid;
        else t_min = t_mid;
    }
    return min_dist;
}

void main() {
    vec2 P = gl_FragCoord.xy;  // 获取当前片段的像素坐标
    float min_dist = 1e6;

    // 遍历所有 Bézier 片段
    for (int i = 0; i < segment_count; i++) 
    {
        // 从纹理中获取每个 Bézier 片段的 4 个控制点
        vec4 P0 = texelFetch(bezier_tex , ivec2(0 , i), 0);  // P0.x , P0.y
        vec4 P1 = texelFetch(bezier_tex , ivec2(1 , i), 0);  // P1.x , P1.y
        vec4 P2 = texelFetch(bezier_tex , ivec2(2 , i), 0);  // P2.x , P2.y
        vec4 P3 = texelFetch(bezier_tex , ivec2(3 , i), 0);  // P3.x , P3.y

        // 使用 AABB 剔除，减少不必要的计算
        if (!is_inside_aabb(P , min(P0.xy , P3.xy), max(P0.xy , P3.xy))) continue;

        // 计算当前 Bézier 曲线的最小距离
        float dist = binary_search_bezier_distance(P , P0.xy , P1.xy , P2.xy , P3.xy);
        min_dist = min(min_dist , dist);
    }

    // 使用 fwidth() 自适应计算抗锯齿范围
    float alpha = smoothstep(0.5 * fwidth(min_dist), 1.5 * fwidth(min_dist), min_dist);

    // 设置片段颜色（通过 alpha 实现抗锯齿效果）
    frag_color = vec4(1.0 , 1.0 , 1.0 , alpha);
}
