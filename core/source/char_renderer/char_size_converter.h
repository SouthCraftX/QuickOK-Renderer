#include "../rendering_env.h"

// #define CHAR_DEFAULT_DPI 72
// #define CHAR_DEFAULT_SCALE_FACTOR 1

struct __TypographicHaoPoundPair
{
    qo_ccstring_t  string;
    qo_fp32_t      pound;
};
typedef struct __TypographicHaoPoundPair _TypographicHaoPoundPair;

struct
{
    qo_fp32_t    factor;
    qo_uint32_t  dpi;
} g_char_scale_info;

void
set_char_scale_info(
    qo_uint32_t  dpi ,
    qo_fp32_t    factor
) {
    QO_ASSERT(dpi && factor);
    g_char_scale_info.dpi = dpi;
    g_char_scale_info.factor = factor;
}

qo_size_t
pound_to_pixel_size(
    qo_fp32_t  pound
) {
    return (qo_size_t) (pound * g_char_scale_info.factor *
        g_char_scale_info.dpi / 72);
}

qo_fp32_t
typographic_hao_to_pound(
    qo_ccstring_t  utf8_string
) {
    const _TypographicHaoPoundPair table[] = {
        {"大特" , 63.},
        {"特号" , 54.},
        {"初号" , 42.},
        {"小初" , 36.},
        {"大一" , 31.5},
        {"一号" , 26.},
        {"小一" , 24.},
        {"二号" , 22.},
        {"小二" , 18.},
        {"三号" , 16.},
        {"小三" , 15.},
        {"四号" , 14.},
        {"小四" , 12.},
        {"五号" , 10.5},
        {"小五" , 9.},
        {"六号" , 7.5},
        {"小六" , 6.5},
        {"七号" , 5.5},
        {"八号" , 5.}
    };
    const qo_size_t table_size = sizeof(table) / sizeof(_TypographicHaoPoundPair);
    for (qo_int32_t i = 0 ; i < table_size ; ++i)
    {
        if (!strcmp(utf8_string , table[i].string))
        {
            return table[i].pound;
        }
    }
    return -1.;
}
