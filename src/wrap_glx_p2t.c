/* OCaml bindings for the GLX library (as part of OCaml-Xlib).
   Copyright (C) 2008, 2009 by Florent Monnier
   printf("fmonnier@%s", "linux-nantes.org");
 
  OCaml-Xlib is FLOSS software:
 
  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
 
  The Software is provided "AS IS", without warranty of any kind, express or
  implied, including but not limited to the warranties of merchantability,
  fitness for a particular purpose and noninfringement. In no event shall
  the authors or copyright holders be liable for any claim, damages or other
  liability, whether in an action of contract, tort or otherwise, arising
  from, out of or in connection with the Software or the use or other dealings
  in the Software.
*/

#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glx.h>

#define CAML_NAME_SPACE 1

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/fail.h>

#include "wrap_xlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wrap_glx.h"

static PFNGLXBINDTEXIMAGEEXTPROC    glXBindTexImageEXT_func    = NULL;
static PFNGLXRELEASETEXIMAGEEXTPROC glXReleaseTexImageEXT_func = NULL;

static int  glx_p2t_not_available = 0;

#define no_glx_p2t() \
    do{ caml_failwith("GLX_P2T: unable to load functions for the " \
                      "extension GLX_EXT_texture_from_pixmap"); }while(0)

CAMLprim value
ml_init_glx_p2t( value unit )
{
    glXBindTexImageEXT_func = (PFNGLXBINDTEXIMAGEEXTPROC)
        glXGetProcAddress((GLubyte *) "glXBindTexImageEXT");

    glXReleaseTexImageEXT_func = (PFNGLXRELEASETEXIMAGEEXTPROC)
        glXGetProcAddress((GLubyte*) "glXReleaseTexImageEXT");

    if (!glXBindTexImageEXT_func || !glXReleaseTexImageEXT_func) {
        glx_p2t_not_available = 1;
    } else {
        glx_p2t_not_available = 0;
    }
    /*
     * Don't raise a failure here in case the functions of this
     * extenstion are not used. For example if the user code first
     * checks if this extension is available or not before calling
     * these functions (but the functions are there, so this current
     * init_glx_p2t() would have been called when the module was open.
     */
    return Val_unit;
}

static const int tex_image_buffer_table[] = {
    GLX_FRONT_LEFT_EXT,
    GLX_FRONT_RIGHT_EXT,
    GLX_BACK_LEFT_EXT,
    GLX_BACK_RIGHT_EXT,
    GLX_FRONT_EXT,
    GLX_BACK_EXT,
    GLX_AUX0_EXT,
    GLX_AUX1_EXT,
    GLX_AUX2_EXT,
    GLX_AUX3_EXT,
    GLX_AUX4_EXT,
    GLX_AUX5_EXT,
    GLX_AUX6_EXT,
    GLX_AUX7_EXT,
    GLX_AUX8_EXT,
    GLX_AUX9_EXT,
};

CAMLprim value
ml_glXBindTexImageEXT( value dpy, value drawable, value buffer, value ml_attrib_list )
{
    if (glx_p2t_not_available) no_glx_p2t();
    glXBindTexImageEXT_func(
        Display_val(dpy),
        GLXDrawable_val(drawable),
        tex_image_buffer_table[Long_val(buffer)],
        NULL // const int *attrib_list  /* TODO */
    );
    return Val_unit;
}

CAMLprim value
ml_glXReleaseTexImageEXT( value dpy, value drawable, value buffer )
{
    if (glx_p2t_not_available) no_glx_p2t();
    glXReleaseTexImageEXT_func(
        Display_val(dpy),
        GLXDrawable_val(drawable),
        tex_image_buffer_table[Long_val(buffer)]
    );
    return Val_unit;
}

CAMLprim value
ml_glXCreatePixmapEXT( value dpy, value config, value pixmap, value attrib_list )
{
    static const int GLX_TEXTURE_FORMAT_EXT_table[] = {
        GLX_TEXTURE_FORMAT_NONE_EXT,
        GLX_TEXTURE_FORMAT_RGB_EXT,
        GLX_TEXTURE_FORMAT_RGBA_EXT,
    };
    static const int GLX_TEXTURE_TARGET_EXT_table[] = {
        GLX_TEXTURE_1D_EXT,
        GLX_TEXTURE_2D_EXT,
        GLX_TEXTURE_RECTANGLE_EXT,
    };

    int attrs[30]; // TODO: is this thread safe ? or should we do a malloc ?
    int i = 0;
    while ( attrib_list != Val_emptylist )
    {
        value attrib = Field(attrib_list, 0);
        {
            switch (Tag_val(attrib))
            {
                case 0:
                    attrs[i]   = GLX_TEXTURE_FORMAT_EXT;
                    attrs[i+1] = GLX_TEXTURE_FORMAT_EXT_table[Long_val(Field(attrib,0))];
                    break;
                case 1:
                    attrs[i]   = GLX_TEXTURE_TARGET_EXT;
                    attrs[i+1] = GLX_TEXTURE_TARGET_EXT_table[Long_val(Field(attrib,0))];
                    break;
                case 2:
                    attrs[i]   = GLX_MIPMAP_TEXTURE_EXT;
                    attrs[i+1] = Bool_val(Field(attrib,0));
                    break;
                default:
                    caml_failwith("glXCreatePixmap: variant handling bug");
            }
            i += 2;
        }
        attrib_list = Field(attrib_list,1);
        if (i >= 28) break;
    }
    attrs[i] = None;

    GLXPixmap glXPixmap =
        glXCreatePixmap( Display_val(dpy), GLXFBConfig_val(config),
                         Pixmap_val(pixmap), attrs );

    return Val_GLXPixmap(glXPixmap);
}

// vim: sw=4 sts=4 ts=4 et fdm=marker
