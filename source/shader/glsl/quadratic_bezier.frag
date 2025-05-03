#version 300 es
precision mediump float;

out vec4 frag_color;
in vec2 v_tex_coords;

uniform sampler2D bezier_segments;
uniform int segment_count;

float
cross2(vec2 a, vec2 b)
{
    return a.x * b.y - a.y * b.x;
}

float
point_to_line_distance(vec2 p, vec2 a, vec2 b)
{
    vec2 ab = b - a;
    float t = clamp(dot(p - a, ab) / dot(ab, ab), 0.0, 1.0);
    vec2 proj = a + t * ab;
    return length(p - proj);
}

// 逐步逼近 Bézier 曲线
float quadratic_bezier_distance(vec2 p, vec2 p0, vec2 p1, vec2 p2)
{
    float area = abs(cross2(p1 - p0, p2 - p0));
    const float epsilon = 1e-3;
    if (area < epsilon) 
        return point_to_line_distance(p, p0, p2);

    vec2 lp = p - p0;  // 局部坐标
    vec2 v1 = p1 - p0;
    vec2 v2 = p2 - p0;
    
    float min_dist = min(length(lp), length(lp - v2));  // 端点距离
    vec2 p_mid = 0.25 * p0 + 0.5 * p1 + 0.25 * p2; // 中间点
    min_dist = min(min_dist, length(p - p_mid));

    // 细分变量
    const int MAX_SUBDIV = 10;
    vec2 m0, m1, b_mid;
    
    for (int i = 0; i < MAX_SUBDIV; i++) {
        m0 = 0.5 * (p0 + p1);
        m1 = 0.5 * (p1 + p2);
        b_mid = 0.5 * (m0 + m1);  // Bézier 中点

        float d_mid = length(p - b_mid);
        min_dist = min(min_dist, d_mid);

        // 选择靠近 `p` 的子曲线继续细分
        if (length(p - m0) < length(p - m1)) {
            p2 = b_mid;
            p1 = m0;
        } else {
            p0 = b_mid;
            p1 = m1;
        }
        
        // 提前终止
        if (d_mid < 1e-3) break;
    }

    return min_dist;
}

void main() 
{
    vec2 p = gl_FragCoord.xy;
    float min_distance = 1e6;

    const float margin = 1.0;
    int max_segments = min(segment_count, 64);

    for (int i = 0; i < max_segments; i++)
    {
        int baseIndex = i * 3;
        vec2 p0 = texelFetch(bezier_segments, ivec2(baseIndex, 0), 0).xy;
        vec2 p1 = texelFetch(bezier_segments, ivec2(baseIndex + 1, 0), 0).xy;
        vec2 p2 = texelFetch(bezier_segments, ivec2(baseIndex + 2, 0), 0).xy;

        vec2 seg_min = min(p0, min(p1, p2)) - vec2(margin);
        vec2 seg_max = max(p0, max(p1, p2)) + vec2(margin);

        if (p.x < seg_min.x || p.x > seg_max.x || p.y < seg_min.y || p.y > seg_max.y)
            continue;

        float d = quadratic_bezier_distance(p, p0, p1, p2);
        min_distance = min(min_distance, d);
    }

    float width = max(fwidth(min_distance) * 2.0, 0.01);
    float alpha = smoothstep(1.0 - width * 0.8, 1.0 + width, min_distance);
    frag_color = vec4(1.0, 1.0, 1.0, alpha);
}
