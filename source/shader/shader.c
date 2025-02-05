#include "../../include/renderer.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>

/// @return shader handle. 0 if error
GLuint
__qo_compile_shader(
    GLenum          type ,
    qo_ccstring_t   source_code ,
    qo_size_t       length
) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader , 1 , &source_code , &length);
    glCompileShader(shader);

    GLint stat;
    glGetShaderiv(shader , GL_COMPILE_STATUS , &stat);
    if(!stat)
    {
        // TODO: log shader compile error
        return 0;
    }
    return shader;
}

GLuint
__qo_load_binary_shader(
    qo_pointer_t    binary ,
    qo_size_t       length ,
    GLenum          format
) {
    GLuint program = glCreateProgram();
    glProgramBinary(program , format , binary , length);

    GLint linked;
    glGetProgramiv(program , GL_LINK_STATUS , &linked);
    if (!linked)
    {
        // TODO: log shader link error
        return 0;
    }
    return program;
}

qo_bool_t
__qo_shader_attach(
    GLuint  program ,
    GLuint  vertex_shader ,
    GLuint  fragment_shader
) {
    glAttachShader(program , vertex_shader);
    glAttachShader(program , fragment_shader);
    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program , GL_LINK_STATUS , &linked);
    if (!linked)
    {
        // TODO: log shader link error
        return qo_false;
    }
    return qo_true;
}






