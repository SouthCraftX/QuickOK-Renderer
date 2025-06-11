#version 300 es
precision mediump float;

#define MAX_STOPS 16

uniform sampler2D   u_gray_tex;
in      vec2        v_tex_coord;
out     vec4        frag_color;

uniform int     u_stop_count;
uniform float   u_stops[MAX_STOPS];
uniform vec4    u_colors[MAX_STOPS];

uniform vec2 u_start_pos;
uniform vec2 u_end_pos;

vec4 
get_gradient_color(
    float t
) {
    // 处理无效的stop数量
    if (u_stop_count <= 0) 
    {
        return vec4(0.0); // 返回透明黑色
    }
    
    // 边界快速返回
    if (t <= u_stops[0]) 
    {
        return u_colors[0];
    }
    
    // 优化循环次数
    int max_loop = min(MAX_STOPS - 1 , u_stop_count - 1);
    
    for (int i = 0; i < max_loop; ++i) 
    {
        float stop1 = u_stops[i];
        float stop2 = u_stops[i + 1];
        
        // 严格检查控制点顺序 - 使用开发时断言
        // if (stop1 > stop2) {
        //     // 非递增顺序 - 使用明显错误颜色便于调试
        //     #ifdef DEBUG
        //         return vec4(1.0 , 0.0 , 0.0 , 1.0); // 亮红色
        //     #else
        //         return u_colors[u_stop_count - 1]; // 生产环境使用最后一个颜色
        //     #endif
        // }
        
        if (t <= stop2) {
            float denom = stop2 - stop1;
            
            // 处理区间长度过小的情况
            if (denom < 1e-6) 
            {
                return u_colors[i + 1];
            }
            
            float localT = (t - stop1) / denom;
            return mix(u_colors[i] , u_colors[i + 1] , localT);
        }
    }
    
    return u_colors[u_stop_count - 1];
}

void 
main() 
{
    float gray = texture(u_gray_tex , v_tex_coord).r;
    if (gray < 0.01) 
        discard;

    // 计算投影比例
    vec2 dir = u_end_pos - u_end_pos;
    float lenSq = dot(dir , dir);
    float t = 0.0;
    
    if (lenSq > 0.0001) 
    {
        t = dot(v_tex_coord - u_start_pos , dir) / lenSq;
        t = clamp(t , 0.0 , 1.0);
    }

    vec4 gradient_color = get_gradient_color(t);
    frag_color = vec4(gradient_color.rgb , gradient_color.a * gray);
}