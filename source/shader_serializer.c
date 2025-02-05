#include "../include/shader_serializer.h"
#include <stdio.h>

struct _QO_GLESSerializer 
{
    FILE *      p_file;
    qo_size_t   shader_count;   
    qo_size_t   program_count;
}