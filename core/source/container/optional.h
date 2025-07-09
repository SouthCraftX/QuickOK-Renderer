#pragma once
#define __QOR_OPTIONAL_SRC__

#define _OPTIONAL(type , var_name) \
    struct \
    { \
        qo_bool_t has_value; \
        type data; \
    } var_name;
