#include "opengl_helpers.hpp"
#include "opengl_includes.hpp"

#include <assert.h>
#include <stdio.h>
#include <cstdlib>

const char* gl_error_enum_string(GLenum e)
{
    switch(e)
    {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
	    return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
	    return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
	    return "GL_INVALID_OPERATION";
	case GL_OUT_OF_MEMORY:
	    return "GL_OUT_OF_MEMORY";
	default:
	    return "UNKNOWN ENUM";
    }
    return nullptr;
}


void gl_log_error()
{
    GLenum error_code = glGetError();
    while (GL_NO_ERROR != error_code)
    {
        printf("OpenGL error %d: %s\n", error_code, gl_error_enum_string(error_code));
        error_code = glGetError();
    }
}

void glad_empty_callback(const char* name, void* funcptr, int len_args, ...) {}

void glad_pre_callback(const char* name, void* funcptr, int len_args, ...)
{
    GLenum error_code = glad_glGetError();
    if (GL_NO_ERROR == error_code)
        return;
    while (GL_NO_ERROR != error_code)
    {
        printf("Error before %s: %d %s\n", name, error_code, gl_error_enum_string(error_code));
        error_code = glad_glGetError();
    }
    exit(1);
}

void glad_post_callback(const char* name, void* funcptr, int len_args, ...)
{
    GLenum error_code = glad_glGetError();
    if (GL_NO_ERROR == error_code)
        return;
    while (GL_NO_ERROR != error_code)
    {
        printf("Error after %s: %d %s\n", name, error_code, gl_error_enum_string(error_code));
        error_code = glad_glGetError();
    }
    exit(1);
}

void glad_setup_callback(bool pre, bool post)
{
    if(pre)
        glad_set_pre_callback(glad_pre_callback);
    else
        glad_set_pre_callback(glad_empty_callback);

    if(post)
        glad_set_post_callback(glad_post_callback);
    else
        glad_set_post_callback(glad_empty_callback);
}
   
