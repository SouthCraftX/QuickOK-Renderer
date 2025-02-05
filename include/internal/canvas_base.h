// for debug
#include "../renderer.h"

#define __QO_CANVAS_EXTRA_CONTEXT_PTRS  4

struct __QO_CanvasMethods
{
    void
    (* destructor)(
        QO_Canvas * self
    );

    qo_stat_t
    (* render_textline)(
        QO_Canvas *         self ,
        qo_ccstring_t       textline ,
        qo_uint32_t         x ,
        qo_uint32_t         y ,
        QO_FontEngine *     p_font_engine
    );

    void
    (* clean)(
        QO_Canvas *                     self ,
        QO_CanvasCleaningMethod    method
    );

    qo_stat_t
    (* draw_rectangle)(
        QO_Canvas *         self ,
        qo_uint32_t         x ,
        qo_uint32_t         y ,
        qo_uint32_t         width ,
        qo_uint32_t         height ,
        QO_Color            color
    );
};
typedef struct __QO_CanvasMethods _QO_CanvasMethods ;

struct _QO_Canvas 
{
    qo_uint32_t         width;
    qo_uint32_t         height;
    QO_Colorspace       colorspace;
    QO_Color            default_background_color;
    _QO_CanvasMethods * methods;
    QO_RenderingEnv *   p_bound_env;
    qo_pointer_t        extra_context[__QO_CANVAS_EXTRA_CONTEXT_PTRS];
};

QO_GLOBAL_UNIQUE QO_FORCE_INLINE
void
qo_canvas_destroy(
    QO_Canvas * p_canvas
) { 
    p_canvas->methods->destructor(p_canvas);
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE
void
qo_canvas_clean(
    QO_Canvas *             p_canvas ,
    QO_CanvasCleaningMethod method
) {
    p_canvas->methods->clean(p_canvas , method);
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE
qo_stat_t
qo_canvas_render_textline(
    QO_Canvas *         p_canvas ,
    qo_ccstring_t       textline ,
    qo_uint32_t         x ,
    qo_uint32_t         y ,
    QO_FontEngine *     p_font_engine
) {
    return p_canvas->methods->render_textline(p_canvas , textline , x , y , p_font_engine);
}

QO_GLOBAL_UNIQUE QO_FORCE_INLINE
qo_stat_t
qo_canvas_draw_rectangle(
    QO_Canvas *         p_canvas ,
    qo_uint32_t         x ,
    qo_uint32_t         y ,
    qo_uint32_t         width ,
    qo_uint32_t         height ,
    QO_Color            color
) {
    return p_canvas->methods->draw_rectangle(p_canvas , x , y , width , height , color);
}