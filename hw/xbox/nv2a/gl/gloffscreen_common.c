/*
 *  Offscreen OpenGL abstraction layer - Common utilities
 *
 *  Copyright (c) 2010 Intel
 *  Written by:
 *    Gordon Williams <gordon.williams@collabora.co.uk>
 *    Ian Molton <ian.molton@collabora.co.uk>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "gloffscreen.h"

static inline const char *gl_error_to_str(GLint gl_error)
{
    switch(gl_error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_INVALID_INDEX: return "GL_INVALID_INDEX";
        default: return "Unknown";
    }
}

bool glo_requestpixels(GLenum gl_format, GLenum gl_type, GLuint pbo_id,
                    unsigned int bytes_per_pixel, unsigned int stride,
                    unsigned int width, unsigned int height) {
    bool result = true;

    /* TODO: weird strides */
    assert(stride % bytes_per_pixel == 0);

    /* Save guest processes GL state before we ReadPixels() */
    int rl, pa;
    glGetIntegerv(GL_PACK_ROW_LENGTH, &rl);
    glGetIntegerv(GL_PACK_ALIGNMENT, &pa);
    glPixelStorei(GL_PACK_ROW_LENGTH, stride / bytes_per_pixel);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id);
    glReadPixels(0, 0, width, height, gl_format, gl_type, NULL);
    GLint gl_error = glGetError();
    if(gl_error != GL_NO_ERROR) {
        fprintf(stderr, "glReadPixels: %s\n", gl_error_to_str(gl_error));
        result = false;
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    /* Restore GL state */
    glPixelStorei(GL_PACK_ROW_LENGTH, rl);
    glPixelStorei(GL_PACK_ALIGNMENT, pa);

    return result;
}

void glo_readpixels(GLenum gl_format, GLenum gl_type, GLuint pbo_id,
                    unsigned int bytes_per_pixel, unsigned int stride,
                    unsigned int width, unsigned int height, bool vflip,
                    void *data)
{
    /* TODO: weird strides */
    assert(stride % bytes_per_pixel == 0);

    /* Save guest processes GL state before we ReadPixels() */
    int rl, pa;
    glGetIntegerv(GL_PACK_ROW_LENGTH, &rl);
    glGetIntegerv(GL_PACK_ALIGNMENT, &pa);
    glPixelStorei(GL_PACK_ROW_LENGTH, stride / bytes_per_pixel);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id);

    void *ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    GLint gl_error = glGetError();
    if(gl_error != GL_NO_ERROR) {
        fprintf(stderr, "glMapBuffer: %s\n", gl_error_to_str(gl_error));
    } else {
        if(ptr != NULL) {
            memcpy(data, ptr, stride * height);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

            if (vflip) {
                GLubyte *b = (GLubyte *) data;
                GLubyte *c = &((GLubyte *) data)[stride * (height - 1)];
                GLubyte *tmp = (GLubyte *) malloc(width * bytes_per_pixel);
                    for (int irow = 0; irow < height / 2; irow++) {
                    memcpy(tmp, b, width * bytes_per_pixel);
                    memcpy(b, c, width * bytes_per_pixel);
                    memcpy(c, tmp, width * bytes_per_pixel);
                    b += stride;
                    c -= stride;
                }
                free(tmp);
            }
        } else {
            fprintf(stderr, "glMapBuffer returned NULL\n");
        }
    }

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    /* Restore GL state */
    glPixelStorei(GL_PACK_ROW_LENGTH, rl);
    glPixelStorei(GL_PACK_ALIGNMENT, pa);
}

bool glo_check_extension(const char* ext_name)
{
    return epoxy_has_gl_extension(ext_name);
}
