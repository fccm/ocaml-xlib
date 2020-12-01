/* OCaml bindings for the GLX library (as part of OCaml-Xlib).
   Copyright (C) 2008, 2009 by Florent Monnier
   printf("monnier.florent@%s", "gmail.com");
  
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wrap_xlib.h"
#include "wrap_glx.h"

custom_ops(XVisualInfo);  // caml alloc

CAMLprim value
ml_glXQueryExtension( value dpy )
{
    int error_base, event_base;
    if (!glXQueryExtension( Display_val(dpy), &error_base, &event_base )) {
        do_warn2("glXQueryExtension: error_base: %d;  event_base: %d\n",
                         error_base, event_base);
        caml_failwith("GLX extension not supported by the X server");
    }
    return Val_unit;
}

CAMLprim value
ml_glXQueryVersion( value dpy )
{
    CAMLparam1(dpy);
    CAMLlocal1(tpl);
    int major, minor;
    if (!glXQueryVersion( Display_val(dpy), &major, &minor ))
        caml_failwith("glXQueryVersion");
    tpl = caml_alloc(2, 0);
    Store_field( tpl, 0, Val_int(major) );
    Store_field( tpl, 1, Val_int(minor) );
    CAMLreturn(tpl);
}

CAMLprim value
ml_glXQueryExtensionsString( value dpy, value screen )
{
    return caml_copy_string( glXQueryExtensionsString( Display_val(dpy), ScreenNB_val(screen) ) );
}

static const int name_table[] = {
    GLX_VENDOR,
    GLX_VERSION,
    GLX_EXTENSIONS
};

CAMLprim value
ml_glXQueryServerString( value dpy, value screen, value name )
{
    return caml_copy_string( glXQueryServerString( Display_val(dpy), ScreenNB_val(screen),
                                                   name_table[Long_val(name)] ) );
}

CAMLprim value
ml_glXGetClientString( value dpy, value name )
{
    const char *str = glXGetClientString( Display_val(dpy), name_table[Long_val(name)] );
    return caml_copy_string(str);
}

CAMLprim value
ml_glXCreateWindow( value dpy, value config, value win )
{
    int attrib_list[] = { None };  // the man says: currently unused
    GLXWindow glx_win = glXCreateWindow( Display_val(dpy), GLXFBConfig_val(config),
                                         Window_val(win), attrib_list );
    return Val_GLXWindow(glx_win);
}

CAMLprim value
ml_glXDestroyWindow( value dpy, value win )
{
    glXDestroyWindow( Display_val(dpy), GLXWindow_val(win) );
    return Val_unit;
}

CAMLprim value
ml_glXSwapBuffers( value dpy, value drawable )
{
    glXSwapBuffers( Display_val(dpy), GLXDrawable_val(drawable) );
    return Val_unit;
}

CAMLprim value
ml_glXUseXFont( value font, value first, value count, value list )
{
    glXUseXFont( Font_val(font), Int_val(first), Int_val(count), Int_val(list) );
    return Val_unit;
}

CAMLprim value
ml_glXWaitGL(value unit)
{
    glXWaitGL();
    return Val_unit;
}

CAMLprim value
ml_glXWaitX(value unit)
{
    glXWaitX();
    return Val_unit;
}

static inline int
visual_attrib( value v, int *p1, int *p2 )
{
    if (Is_long(v))
    {
        switch (Int_val(v))
        {
            case 0: *p1 = GLX_USE_GL; break;
            case 1: *p1 = GLX_RGBA; break;
            case 2: *p1 = GLX_DOUBLEBUFFER; break;
            case 3: *p1 = GLX_STEREO; break;
            default: caml_failwith("visual_attrib handling bug");
        }
        return 1;
    }
    else // (Is_block(v))
    {
        switch (Tag_val(v))
        {
            case 0:  *p1 = GLX_BUFFER_SIZE;      *p2 = UInt_val(Field(v,0)); break;
            case 1:  *p1 = GLX_LEVEL;            *p2 = Int_val(Field(v,0)); break;
            case 2:  *p1 = GLX_AUX_BUFFERS;      *p2 = UInt_val(Field(v,0)); break;
            case 3:  *p1 = GLX_RED_SIZE;         *p2 = UInt_val(Field(v,0)); break;
            case 4:  *p1 = GLX_GREEN_SIZE;       *p2 = UInt_val(Field(v,0)); break;
            case 5:  *p1 = GLX_BLUE_SIZE;        *p2 = UInt_val(Field(v,0)); break;
            case 6:  *p1 = GLX_ALPHA_SIZE;       *p2 = UInt_val(Field(v,0)); break;
            case 7:  *p1 = GLX_DEPTH_SIZE;       *p2 = UInt_val(Field(v,0)); break;
            case 8:  *p1 = GLX_STENCIL_SIZE;     *p2 = UInt_val(Field(v,0)); break;
            case 9:  *p1 = GLX_ACCUM_RED_SIZE;   *p2 = UInt_val(Field(v,0)); break;
            case 10: *p1 = GLX_ACCUM_GREEN_SIZE; *p2 = UInt_val(Field(v,0)); break;
            case 11: *p1 = GLX_ACCUM_BLUE_SIZE;  *p2 = UInt_val(Field(v,0)); break;
            case 12: *p1 = GLX_ACCUM_ALPHA_SIZE; *p2 = Int_val(Field(v,0)); break;
            default: caml_failwith("visual_attrib handling bug");
        }
        return 2;
    }
}


CAMLprim value
ml_glXChooseVisual( value dpy, value screen, value ml_attribList )
{
    CAMLparam3(dpy, screen, ml_attribList);
    CAMLlocal1(visual_info);
#define MAX_ATTRIBS 31
    int attribList[MAX_ATTRIBS];
    XVisualInfo *visinfo = NULL;

    int i = 0;
    while ( ml_attribList != Val_emptylist )
    {
        int n, p1, p2;
        value attrib = Field(ml_attribList, 0);
        n = visual_attrib( attrib, &p1, &p2 );
        if (n == 1) {
            if (i > MAX_ATTRIBS - 2)  // i max = 29
                caml_invalid_argument("glXChooseVisual: Visual.attrib list: too much elements");

            attribList[i] = p1;
            ++i;
        } else {
            if (i > MAX_ATTRIBS - 3)  // i max = 28
                caml_invalid_argument("glXChooseVisual: Visual.attrib list: too much elements");
            attribList[i] = p1;
            attribList[i+1] = p2;
            i += 2;
        }
        ml_attribList = Field(ml_attribList, 1);
    }
    attribList[i] = None;  // i max = 30

    visinfo = glXChooseVisual( Display_val(dpy), ScreenNB_val(screen), attribList );
    if (!visinfo) caml_failwith("glXChooseVisual");
    alloc_XVisualInfo(visual_info);
    memcpy(XVisualInfo_val(visual_info), visinfo, sizeof(XVisualInfo));
    XFree(visinfo);
    CAMLreturn(visual_info);
}
#undef MAX_ATTRIBS

static inline void
fbconfig_attrib( value v, int *p1, int *p2 )
{
    if (Is_long(v))
        caml_failwith("visual_attrib handling bug");

    // (Is_block(v))
    {
        switch (Tag_val(v))
        {
#define attrib_case(n, attrib_name, attrib_param_conv) \
          case n:  *p1 = attrib_name;  *p2 = attrib_param_conv(Field(v,0));  break;

          attrib_case(0,  GLX_FBCONFIG_ID,      GLXFBConfigID_val )
          attrib_case(1,  GLX_BUFFER_SIZE,      UInt_val )
          attrib_case(2,  GLX_LEVEL,            Int_val  )
          attrib_case(3,  GLX_DOUBLEBUFFER,     Bool_val )
          attrib_case(4,  GLX_STEREO,           Bool_val )
          attrib_case(5,  GLX_AUX_BUFFERS,      UInt_val )
          attrib_case(6,  GLX_RED_SIZE,         UInt_val )
          attrib_case(7,  GLX_GREEN_SIZE,       UInt_val )
          attrib_case(8,  GLX_BLUE_SIZE,        UInt_val )
          attrib_case(9,  GLX_ALPHA_SIZE,       UInt_val )
          attrib_case(10, GLX_DEPTH_SIZE,       UInt_val )
          attrib_case(11, GLX_STENCIL_SIZE,     UInt_val )
          attrib_case(12, GLX_ACCUM_RED_SIZE,   UInt_val )
          attrib_case(13, GLX_ACCUM_GREEN_SIZE, UInt_val )
          attrib_case(14, GLX_ACCUM_BLUE_SIZE,  UInt_val )
          attrib_case(15, GLX_ACCUM_ALPHA_SIZE, Int_val  )
          //attrib_case(16, GLX_RENDER_TYPE,      GLX.opengl_rendering_modes list )  TODO
          //attrib_case(17, GLX_DRAWABLE_TYPE,    GLX.glx_drawable_types list     )  TODO
          attrib_case(18, GLX_X_RENDERABLE,     Bool_val         )
          //attrib_case(19, GLX_X_VISUAL_TYPE,    GLX.x_visual_type     )  TODO
          //attrib_case(20, GLX_CONFIG_CAVEAT,    GLX.config_caveat     )  TODO
          //attrib_case(21, GLX_TRANSPARENT_TYPE, GLX.transparent_type  )  TODO
          attrib_case(22, GLX_TRANSPARENT_INDEX_VALUE,  Int_val )
          attrib_case(23, GLX_TRANSPARENT_RED_VALUE,    Int_val )
          attrib_case(24, GLX_TRANSPARENT_GREEN_VALUE,  Int_val )
          attrib_case(25, GLX_TRANSPARENT_BLUE_VALUE,   Int_val )
          attrib_case(26, GLX_TRANSPARENT_ALPHA_VALUE,  Int_val )
#undef attrib_case
            default: caml_failwith("fbconfig_attrib handling bug");
        }
    }
}


CAMLprim value
ml_glXChooseFBConfig( value dpy, value screen, value ml_attribList )
{
    CAMLparam3(dpy, screen, ml_attribList);
    CAMLlocal1(ml_configs);
#define MAX_ATTRIBS 55
    int attribList[MAX_ATTRIBS];
    GLXFBConfig *configs;
    int i, nitems;

    i = 0;
    while ( ml_attribList != Val_emptylist )
    {
        value attrib = Field(ml_attribList, 0);
        if (i > MAX_ATTRIBS - 3)  // i max = 52
            caml_invalid_argument("glXChooseFBConfig: "
                                  "FBConfig.attrib list: too much elements");
        fbconfig_attrib( attrib, &(attribList[i]), &(attribList[i+1]) );
        i += 2;
        ml_attribList = Field(ml_attribList, 1);
    }
    attribList[i] = None;  // i max = 54

    configs = glXChooseFBConfig( Display_val(dpy), ScreenNB_val(screen), attribList, &nitems );
    ml_configs = caml_alloc(nitems, 0);
    for (i=0; i < nitems; ++i)
    {
        Store_field( ml_configs, i, Val_GLXFBConfig(configs[i]) );
    }
    CAMLreturn(ml_configs);
}
#undef MAX_ATTRIBS

CAMLprim value
ml_glXGetFBConfigs( value dpy, value screen )
{
    CAMLparam2(dpy, screen);
    CAMLlocal1(ml_configs);
    GLXFBConfig *configs;
    int i, nelements;
    configs = glXGetFBConfigs( Display_val(dpy), ScreenNB_val(screen), &nelements );
    ml_configs = caml_alloc(nelements, 0);
    for (i=0; i < nelements; ++i)
    {
        Store_field( ml_configs, i, Val_GLXFBConfig(configs[i]) );
    }
    CAMLreturn(ml_configs);
}

CAMLprim value
ml_XFree_glXFBConfig( value configs )
{
    CAMLparam1(configs);
    CAMLlocal1(head);
    GLXFBConfig config_head;
    if (configs == Val_emptylist)
        caml_invalid_argument("xFree_glXFBConfig");
    head = Field(configs, 0);
    config_head = GLXFBConfig_val(head);
    XFree( &config_head );
    CAMLreturn(Val_unit);
}

CAMLprim value
ml_glXGetVisualFromFBConfig( value dpy, value config )
{
    CAMLparam2(dpy, config);
    CAMLlocal1(visual_info);
    XVisualInfo *visinfo = NULL;
    visinfo = glXGetVisualFromFBConfig( Display_val(dpy), GLXFBConfig_val(config) );
    if (!visinfo) caml_failwith("glXGetVisualFromFBConfig");
    alloc_XVisualInfo(visual_info);
    memcpy(XVisualInfo_val(visual_info), visinfo, sizeof(XVisualInfo));
    XFree(visinfo); /* XXX */
    CAMLreturn(visual_info);
}

CAMLprim value
ml_glXCreatePixmap( value dpy, value config, value pixmap, value attrib_list )
{
    const int attrs[] = { None };

    if ( attrib_list != Val_emptylist ) {
        caml_invalid_argument(
            "GLX.glXCreatePixmap: the attribs parameter should be an empty list,"
            " in GLX 1.4 this parameter is ignored."
            " but there are additional parameters with extensions,"
            " for instance see GLX_P2T.glXCreatePixmapEXT");
    }

    GLXPixmap glXPixmap =
        glXCreatePixmap( Display_val(dpy), GLXFBConfig_val(config),
                         Pixmap_val(pixmap), attrs /* NULL */ );
    return Val_GLXPixmap(glXPixmap);
}

CAMLprim value
ml_glXDestroyPixmap( value dpy, value pixmap )
{
    glXDestroyPixmap( Display_val(dpy), GLXPixmap_val(pixmap) );
    return Val_unit;
}

CAMLprim value
ml_glXCreateContext( value dpy, value vis, value share_list, value direct )
{
    GLXContext shareList =
        ( (share_list == Val_int(0)) ?
          NULL :                                 // None
          GLXContext_val( Field(share_list,0) )  // Some v
        );

    GLXContext ctx = glXCreateContext( Display_val(dpy), XVisualInfo_val(vis),
                                       shareList, Bool_val(direct) );

    if (!ctx) caml_failwith("glXCreateContext");

    return Val_GLXContext(ctx);
}


CAMLprim value
ml_glXCreateNewContext( value dpy, value config, value render_type, value share_list, value direct )
{
    GLXContext shareList =
        ( (share_list == Val_int(0)) ?
          NULL :                                 // None
          GLXContext_val( Field(share_list,0) )  // Some v
        );

    GLXContext ctx = glXCreateNewContext(
         Display_val(dpy),
         GLXFBConfig_val(config),
         ( (render_type == Val_int(0)) ? GLX_RGBA_TYPE : GLX_COLOR_INDEX_TYPE ),
         shareList,
         Bool_val(direct) );

    if (!ctx) caml_failwith("glXCreateNewContext");

    return Val_GLXContext(ctx);
}


static const unsigned long conv_attrib_bit_table[] = {
    GL_ACCUM_BUFFER_BIT,
    GL_COLOR_BUFFER_BIT,
    GL_CURRENT_BIT,
    GL_DEPTH_BUFFER_BIT,
    GL_ENABLE_BIT,
    GL_EVAL_BIT,
    GL_FOG_BIT,
    GL_HINT_BIT,
    GL_LIGHTING_BIT,
    GL_LINE_BIT,
    GL_LIST_BIT,
    GL_MULTISAMPLE_BIT,
    GL_PIXEL_MODE_BIT,
    GL_POINT_BIT,
    GL_POLYGON_BIT,
    GL_POLYGON_STIPPLE_BIT,
    GL_SCISSOR_BIT,
    GL_STENCIL_BUFFER_BIT,
    GL_TEXTURE_BIT,
    GL_TRANSFORM_BIT,
    GL_VIEWPORT_BIT,
};

CAMLprim value
ml_glXCopyContext( value dpy, value src, value mask_list )
{
    GLXContext dst = NULL;
    unsigned long attrib_bit = 0;
    while ( mask_list != Val_emptylist )
    {
        value head = Field(mask_list, 0);
        attrib_bit |= conv_attrib_bit_table[Long_val(head)];
        mask_list = Field(mask_list, 1);
    }
    glXCopyContext( Display_val(dpy), GLXContext_val(src), dst, attrib_bit );
    if (!dst) caml_failwith("glXCopyContext");
    return Val_GLXContext(dst);
}

CAMLprim value
ml_glXDestroyContext( value dpy, value ctx )
{
    glXDestroyContext( Display_val(dpy), GLXContext_val(ctx) );
    return Val_unit;
}

CAMLprim value
ml_glXMakeContextCurrent( value dpy, value draw, value read, value ctx )
{
    if (!glXMakeContextCurrent( Display_val(dpy), GLXDrawable_val(draw), GLXDrawable_val(read), GLXContext_val(ctx) ))
        caml_failwith("glXMakeContextCurrent");
    return Val_unit;
}

CAMLprim value
ml_glXMakeContextCurrent_release( value dpy )
{
    if (!glXMakeContextCurrent( Display_val(dpy), None, None, NULL ))
        caml_failwith("glXMakeContextCurrent_release");
    return Val_unit;
}

CAMLprim value
ml_glXMakeCurrent( value dpy, value drawable, value ctx )
{
    if (!glXMakeCurrent( Display_val(dpy), GLXDrawable_val(drawable), GLXContext_val(ctx) ))
        caml_failwith("glXMakeCurrent");
    return Val_unit;
}

CAMLprim value
ml_glXMakeCurrent_none( value dpy )
{
    if (!glXMakeCurrent( Display_val(dpy), None, NULL ))
        caml_failwith("glXMakeCurrentNone");
    return Val_unit;
}

CAMLprim value
ml_glXIsDirect( value dpy, value ctx )
{
    Bool b = glXIsDirect( Display_val(dpy), GLXContext_val(ctx) );
    return Val_bool(b);
}

CAMLprim value
ml_glXCreatePbuffer( value dpy, value config, value attrib_list )
{
    int attrs[20]; // TODO: is this thread safe ? or should we do a malloc ?
    int i = 0;
    while ( attrib_list != Val_emptylist )
    {
        value attrib = Field(attrib_list, 0);
        //if (Is_long(attrib)) {
        //    caml_failwith("variant handling bug");
        //} else
        //if (Is_block(attrib))
        {
            switch (Tag_val(attrib))
            {
                case 0: attrs[i++] = GLX_PBUFFER_WIDTH; break;
                case 1: attrs[i++] = GLX_PBUFFER_HEIGHT; break;
                case 2: attrs[i++] = GLX_LARGEST_PBUFFER; break;
                case 3: attrs[i++] = GLX_PRESERVED_CONTENTS; break;
                default: caml_failwith("variant handling bug");
            }
            attrs[i++] = Int_val(Field(attrib,0));
        }
        attrib_list = Field(attrib_list,1);
        if (i >= 19) break;
    }
    attrs[i] = None;

    GLXPbuffer pbuf = glXCreatePbuffer( Display_val(dpy), GLXFBConfig_val(config), attrs );
    return Val_GLXPbuffer(pbuf);
}

CAMLprim value
ml_glXDestroyPbuffer( value dpy, value pbuf )
{
    glXDestroyPbuffer( Display_val(dpy), GLXPbuffer_val(pbuf) );
    return Val_unit;
}


#define push_glx_drawable_type(bit_mask,n) \
    if (c_mask & bit_mask) { \
        cons = caml_alloc(2, 0); \
        Store_field( cons, 0, Val_int(n) ); \
        Store_field( cons, 1, li ); \
        li = cons; \
    }

static value
Val_GLX_RENDER_TYPE( int c_mask )
{
    CAMLparam0();
    CAMLlocal2(li, cons);
    li = Val_emptylist;

    push_glx_drawable_type( GLX_RGBA_BIT,        0 )
    push_glx_drawable_type( GLX_COLOR_INDEX_BIT, 1 )

    CAMLreturn(li);
}

static value
Val_GLX_DRAWABLE_TYPE( int c_mask )
{
    CAMLparam0();
    CAMLlocal2(li, cons);
    li = Val_emptylist;

    push_glx_drawable_type( GLX_WINDOW_BIT,  0 )
    push_glx_drawable_type( GLX_PIXMAP_BIT,  1 )
    push_glx_drawable_type( GLX_PBUFFER_BIT, 2 )

    CAMLreturn(li);
}

static value
Val_x_visual_type( int v )
{
    switch (v)
    {
        case GLX_TRUE_COLOR  : return Val_some( Val_int(0) );
        case GLX_DIRECT_COLOR: return Val_some( Val_int(1) );
        case GLX_PSEUDO_COLOR: return Val_some( Val_int(2) );
        case GLX_STATIC_COLOR: return Val_some( Val_int(3) );
        case GLX_GRAY_SCALE  : return Val_some( Val_int(4) );
        case GLX_STATIC_GRAY : return Val_some( Val_int(5) );
        case GLX_NONE        : return Val_none;
    }
    caml_failwith("glXGetFBConfigAttrib with GLX_X_VISUAL_TYPE");
    return Val_int(0);
}

static value
Val_config_caveat( int c_mask )
{
    CAMLparam0();
    CAMLlocal2(li, cons);
    li = Val_emptylist;

    push_glx_drawable_type( GLX_NONE,                  0 )
    push_glx_drawable_type( GLX_SLOW_CONFIG,           1 )
    push_glx_drawable_type( GLX_NON_CONFORMANT_CONFIG, 2 )

    CAMLreturn(li);
}

static value
Val_transparent_type( int c_mask )
{
    CAMLparam0();
    CAMLlocal2(li, cons);
    li = Val_emptylist;

    push_glx_drawable_type( GLX_NONE,              0 )
    push_glx_drawable_type( GLX_TRANSPARENT_RGB,   1 )
    push_glx_drawable_type( GLX_TRANSPARENT_INDEX, 2 )

    CAMLreturn(li);
}

static value
Val_GLX_BIND_TO_TEXTURE_TARGETS_EXT( int c_mask )
{
    CAMLparam0();
    CAMLlocal2(li, cons);
    li = Val_emptylist;

    push_glx_drawable_type( GLX_TEXTURE_1D_BIT_EXT,        0 )
    push_glx_drawable_type( GLX_TEXTURE_2D_BIT_EXT,        1 )
    push_glx_drawable_type( GLX_TEXTURE_RECTANGLE_BIT_EXT, 2 )

    CAMLreturn(li);
}
#undef push_glx_drawable_type

static value
Val_GLX_VISUAL_ID( int v )
{
    if (v == 0) return Val_none;
    else return Val_some( Val_VisualID(v) );
}


CAMLprim value
ml_glXGetFBConfigAttrib( value dpy, value config, value attribute )
{
    int rvalue;

#define case_fbconfig_attrib(n,attrib,Val_conv) \
    case n: \
      { int ret = glXGetFBConfigAttrib( Display_val(dpy), GLXFBConfig_val(config), attrib, &rvalue); \
        if (ret == Success) return Val_conv(rvalue); else return Val_emptylist; \
      } break;

    switch (Int_val(attribute))
    {
        case_fbconfig_attrib(  0, GLX_FBCONFIG_ID,             Val_GLXFBConfigID )
        case_fbconfig_attrib(  1, GLX_BUFFER_SIZE,             Val_bool )
        case_fbconfig_attrib(  2, GLX_LEVEL,                   Val_int  )
        case_fbconfig_attrib(  3, GLX_DOUBLEBUFFER,            Val_bool )
        case_fbconfig_attrib(  4, GLX_STEREO,                  Val_bool )
        case_fbconfig_attrib(  5, GLX_AUX_BUFFERS,             Val_uint )
        case_fbconfig_attrib(  6, GLX_RED_SIZE,                Val_uint )
        case_fbconfig_attrib(  7, GLX_GREEN_SIZE,              Val_uint )
        case_fbconfig_attrib(  8, GLX_BLUE_SIZE,               Val_uint )
        case_fbconfig_attrib(  9, GLX_ALPHA_SIZE,              Val_uint )
        case_fbconfig_attrib( 10, GLX_DEPTH_SIZE,              Val_uint )
        case_fbconfig_attrib( 11, GLX_STENCIL_SIZE,            Val_uint )
        case_fbconfig_attrib( 12, GLX_ACCUM_RED_SIZE,          Val_uint )
        case_fbconfig_attrib( 13, GLX_ACCUM_GREEN_SIZE,        Val_uint )
        case_fbconfig_attrib( 14, GLX_ACCUM_BLUE_SIZE,         Val_uint )
        case_fbconfig_attrib( 15, GLX_ACCUM_ALPHA_SIZE,        Val_int  )
        case_fbconfig_attrib( 16, GLX_RENDER_TYPE,             Val_GLX_RENDER_TYPE   )
        case_fbconfig_attrib( 17, GLX_DRAWABLE_TYPE,           Val_GLX_DRAWABLE_TYPE )
        case_fbconfig_attrib( 18, GLX_X_RENDERABLE,            Val_bool              )
        case_fbconfig_attrib( 19, GLX_VISUAL_ID,               Val_GLX_VISUAL_ID     )
        case_fbconfig_attrib( 20, GLX_X_VISUAL_TYPE,           Val_x_visual_type     )
        case_fbconfig_attrib( 21, GLX_CONFIG_CAVEAT,           Val_config_caveat     )
        case_fbconfig_attrib( 22, GLX_TRANSPARENT_TYPE,        Val_transparent_type  )
        case_fbconfig_attrib( 23, GLX_TRANSPARENT_INDEX_VALUE, Val_int  )
        case_fbconfig_attrib( 24, GLX_TRANSPARENT_RED_VALUE,   Val_int  )
        case_fbconfig_attrib( 25, GLX_TRANSPARENT_GREEN_VALUE, Val_int  )
        case_fbconfig_attrib( 26, GLX_TRANSPARENT_BLUE_VALUE,  Val_int  )
        case_fbconfig_attrib( 27, GLX_TRANSPARENT_ALPHA_VALUE, Val_int  )
        case_fbconfig_attrib( 28, GLX_MAX_PBUFFER_WIDTH,       Val_uint )
        case_fbconfig_attrib( 29, GLX_MAX_PBUFFER_HEIGHT,      Val_uint )
        case_fbconfig_attrib( 30, GLX_MAX_PBUFFER_PIXELS,      Val_uint )

        /* extension: GLX_EXT_texture_from_pixmap */
        case_fbconfig_attrib( 31, GLX_BIND_TO_TEXTURE_TARGETS_EXT, Val_GLX_BIND_TO_TEXTURE_TARGETS_EXT )
        case_fbconfig_attrib( 32, GLX_BIND_TO_TEXTURE_RGBA_EXT,    Val_bool )
        case_fbconfig_attrib( 33, GLX_BIND_TO_TEXTURE_RGB_EXT,     Val_bool )
        case_fbconfig_attrib( 34, GLX_Y_INVERTED_EXT,              Val_bool )
    }
#undef case_fbconfig_attrib

    caml_failwith("bug in function glXGetFBConfigAttrib");
    return Val_int(0);
}

/* {{{

  | GLX_FBCONFIG_ID of GLX.glXFBConfigID
  | GLX_BUFFER_SIZE of GLX.uint
  | GLX_LEVEL of int
  | GLX_DOUBLEBUFFER of bool
  | GLX_STEREO of bool
  | GLX_AUX_BUFFERS of GLX.uint
  | GLX_RED_SIZE of GLX.uint
  | GLX_GREEN_SIZE of GLX.uint
  | GLX_BLUE_SIZE of GLX.uint
  | GLX_ALPHA_SIZE of GLX.uint
  | GLX_DEPTH_SIZE of GLX.uint
  | GLX_STENCIL_SIZE of GLX.uint
  | GLX_ACCUM_RED_SIZE of GLX.uint
  | GLX_ACCUM_GREEN_SIZE of GLX.uint
  | GLX_ACCUM_BLUE_SIZE of GLX.uint
  | GLX_ACCUM_ALPHA_SIZE of int
  | GLX_RENDER_TYPE of GLX.opengl_rendering_modes list
  | GLX_DRAWABLE_TYPE of GLX.glx_drawable_types list
  | GLX_X_RENDERABLE of bool

GLX_VISUAL_ID ***
 XID of the corresponding visual, or zero if there is no associated visual
 (i.e., if GLX_X_RENDERABLE is False or GLX_DRAWABLE_TYPE does not have the GLX_WINDOW_BIT bit set).

  | GLX_X_VISUAL_TYPE of GLX.x_visual_type
  | GLX_CONFIG_CAVEAT of GLX.config_caveat
  | GLX_TRANSPARENT_TYPE of GLX.transparent_type
  | GLX_TRANSPARENT_INDEX_VALUE of int
  | GLX_TRANSPARENT_RED_VALUE of int
  | GLX_TRANSPARENT_GREEN_VALUE of int
  | GLX_TRANSPARENT_BLUE_VALUE of int
  | GLX_TRANSPARENT_ALPHA_VALUE of int

  (* TODO *)
  | GLX_MAX_PBUFFER_WIDTH of uint
  | GLX_MAX_PBUFFER_HEIGHT of uint
  | GLX_MAX_PBUFFER_PIXELS of uint

}}} */


/* {{{ funcs to be wrapped 

#ifdef GLX_VERSION_1_1
#endif

#ifdef GLX_VERSION_1_2
#endif

#ifdef GLX_VERSION_1_3
#endif

#ifdef GLX_VERSION_1_4
#endif



// Tokens for glXChooseVisual and glXGetConfig:
#define GLX_USE_GL		1
#define GLX_BUFFER_SIZE		2
#define GLX_LEVEL		3
#define GLX_RGBA		4
#define GLX_DOUBLEBUFFER	5
#define GLX_STEREO		6
#define GLX_AUX_BUFFERS		7
#define GLX_RED_SIZE		8
#define GLX_GREEN_SIZE		9
#define GLX_BLUE_SIZE		10
#define GLX_ALPHA_SIZE		11
#define GLX_DEPTH_SIZE		12
#define GLX_STENCIL_SIZE	13
#define GLX_ACCUM_RED_SIZE	14
#define GLX_ACCUM_GREEN_SIZE	15
#define GLX_ACCUM_BLUE_SIZE	16
#define GLX_ACCUM_ALPHA_SIZE	17


// Error codes returned by glXGetConfig:
#define GLX_BAD_SCREEN		1
#define GLX_BAD_ATTRIBUTE	2
#define GLX_NO_EXTENSION	3
#define GLX_BAD_VISUAL		4
#define GLX_BAD_CONTEXT		5
#define GLX_BAD_VALUE       	6
#define GLX_BAD_ENUM		7


// GLX 1.1 and later:
#define GLX_VENDOR		1
#define GLX_VERSION		2
#define GLX_EXTENSIONS 		3


// GLX 1.3 and later:
#define GLX_CONFIG_CAVEAT		0x20
#define GLX_DONT_CARE			0xFFFFFFFF
#define GLX_X_VISUAL_TYPE		0x22
#define GLX_TRANSPARENT_TYPE		0x23
#define GLX_TRANSPARENT_INDEX_VALUE	0x24
#define GLX_TRANSPARENT_RED_VALUE	0x25
#define GLX_TRANSPARENT_GREEN_VALUE	0x26
#define GLX_TRANSPARENT_BLUE_VALUE	0x27
#define GLX_TRANSPARENT_ALPHA_VALUE	0x28
#define GLX_WINDOW_BIT			0x00000001
#define GLX_PIXMAP_BIT			0x00000002
#define GLX_PBUFFER_BIT			0x00000004
#define GLX_AUX_BUFFERS_BIT		0x00000010
#define GLX_FRONT_LEFT_BUFFER_BIT	0x00000001
#define GLX_FRONT_RIGHT_BUFFER_BIT	0x00000002
#define GLX_BACK_LEFT_BUFFER_BIT	0x00000004
#define GLX_BACK_RIGHT_BUFFER_BIT	0x00000008
#define GLX_DEPTH_BUFFER_BIT		0x00000020
#define GLX_STENCIL_BUFFER_BIT		0x00000040
#define GLX_ACCUM_BUFFER_BIT		0x00000080
#define GLX_NONE			0x8000
#define GLX_SLOW_CONFIG			0x8001
#define GLX_TRUE_COLOR			0x8002
#define GLX_DIRECT_COLOR		0x8003
#define GLX_PSEUDO_COLOR		0x8004
#define GLX_STATIC_COLOR		0x8005
#define GLX_GRAY_SCALE			0x8006
#define GLX_STATIC_GRAY			0x8007
#define GLX_TRANSPARENT_RGB		0x8008
#define GLX_TRANSPARENT_INDEX		0x8009
#define GLX_VISUAL_ID			0x800B
#define GLX_SCREEN			0x800C
#define GLX_NON_CONFORMANT_CONFIG	0x800D
#define GLX_DRAWABLE_TYPE		0x8010
#define GLX_RENDER_TYPE			0x8011
#define GLX_X_RENDERABLE		0x8012
#define GLX_FBCONFIG_ID			0x8013
#define GLX_RGBA_TYPE			0x8014
#define GLX_COLOR_INDEX_TYPE		0x8015
#define GLX_MAX_PBUFFER_WIDTH		0x8016
#define GLX_MAX_PBUFFER_HEIGHT		0x8017
#define GLX_MAX_PBUFFER_PIXELS		0x8018
#define GLX_PRESERVED_CONTENTS		0x801B
#define GLX_LARGEST_PBUFFER		0x801C
#define GLX_WIDTH			0x801D
#define GLX_HEIGHT			0x801E
#define GLX_EVENT_MASK			0x801F
#define GLX_DAMAGED			0x8020
#define GLX_SAVED			0x8021
#define GLX_WINDOW			0x8022
#define GLX_PBUFFER			0x8023
#define GLX_PBUFFER_HEIGHT              0x8040
#define GLX_PBUFFER_WIDTH               0x8041
#define GLX_RGBA_BIT			0x00000001
#define GLX_COLOR_INDEX_BIT		0x00000002
#define GLX_PBUFFER_CLOBBER_MASK	0x08000000


// GLX 1.4 and later:
#define GLX_SAMPLE_BUFFERS              0x186a0
#define GLX_SAMPLES                     0x186a1




GLXPixmap glXCreateGLXPixmap( Display *dpy, XVisualInfo *visual, Pixmap pixmap );

void glXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap );


int glXGetConfig( Display *dpy, XVisualInfo *visual, int attrib, int *value );

GLXContext glXGetCurrentContext( void );

GLXDrawable glXGetCurrentDrawable( void );


void glXUseXFont( Font font, int first, int count, int list );



// GLX 1.1 and later
const char *glXQueryExtensionsString( Display *dpy, int screen );

const char *glXQueryServerString( Display *dpy, int screen, int name );



// GLX 1.2 and later
Display *glXGetCurrentDisplay( void );


// GLX 1.3 and later
#ifdef GLX_VERSION_1_3




void glXQueryDrawable( Display *dpy, GLXDrawable draw, int attribute, unsigned int *value );

GLXContext glXCreateNewContext( Display *dpy, GLXFBConfig config,
                                       int renderType, GLXContext shareList,
                                       Bool direct );

Bool glXMakeContextCurrent( Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx );

GLXDrawable glXGetCurrentReadDrawable( void );

int glXQueryContext( Display *dpy, GLXContext ctx, int attribute, int *value );

void glXSelectEvent( Display *dpy, GLXDrawable drawable, unsigned long mask );

void glXGetSelectedEvent( Display *dpy, GLXDrawable drawable, unsigned long *mask );

#endif


// GLX 1.4 and later
void (*glXGetProcAddress(const GLubyte *procname))( void );

}}} */
// {{{ 
#if 0

#ifndef GLX_GLXEXT_LEGACY

#include <GL/glxext.h>

#else



// ARB 2. GLX_ARB_get_proc_address
#ifndef GLX_ARB_get_proc_address
#define GLX_ARB_get_proc_address 1

typedef void (*__GLXextFuncPtr)(void);
__GLXextFuncPtr glXGetProcAddressARB (const GLubyte *);

#endif // GLX_ARB_get_proc_address



#endif // GLX_GLXEXT_LEGACY


///
/// The following aren't in glxext.h yet.
///


// ???. GLX_NV_vertex_array_range
#ifndef GLX_NV_vertex_array_range
#define GLX_NV_vertex_array_range

void *glXAllocateMemoryNV(GLsizei size, GLfloat readfreq, GLfloat writefreq, GLfloat priority);
void glXFreeMemoryNV(GLvoid *pointer);
typedef void * ( * PFNGLXALLOCATEMEMORYNVPROC) (GLsizei size, GLfloat readfreq, GLfloat writefreq, GLfloat priority);
typedef void ( * PFNGLXFREEMEMORYNVPROC) (GLvoid *pointer);

#endif // GLX_NV_vertex_array_range


// ???. GLX_MESA_allocate_memory
#ifndef GLX_MESA_allocate_memory
#define GLX_MESA_allocate_memory 1

void *glXAllocateMemoryMESA(Display *dpy, int scrn, size_t size, float readfreq, float writefreq, float priority);
void glXFreeMemoryMESA(Display *dpy, int scrn, void *pointer);
GLuint glXGetMemoryOffsetMESA(Display *dpy, int scrn, const void *pointer);
typedef void * ( * PFNGLXALLOCATEMEMORYMESAPROC) (Display *dpy, int scrn, size_t size, float readfreq, float writefreq, float priority);
typedef void ( * PFNGLXFREEMEMORYMESAPROC) (Display *dpy, int scrn, void *pointer);
typedef GLuint (* PFNGLXGETMEMORYOFFSETMESAPROC) (Display *dpy, int scrn, const void *pointer);

#endif // GLX_MESA_allocate_memory


//
// ARB ?. GLX_ARB_render_texture
// XXX This was never finalized!
//
#ifndef GLX_ARB_render_texture
#define GLX_ARB_render_texture 1

Bool glXBindTexImageARB(Display *dpy, GLXPbuffer pbuffer, int buffer);
Bool glXReleaseTexImageARB(Display *dpy, GLXPbuffer pbuffer, int buffer);
Bool glXDrawableAttribARB(Display *dpy, GLXDrawable draw, const int *attribList);

#endif // GLX_ARB_render_texture


/*
 * Remove this when glxext.h is updated.
 */
#ifndef GLX_NV_float_buffer
#define GLX_NV_float_buffer 1

#define GLX_FLOAT_COMPONENTS_NV         0x20B0

#endif /* GLX_NV_float_buffer */



/* GLX_MESA_swap_frame_usage */
#ifndef GLX_MESA_swap_frame_usage
#define GLX_MESA_swap_frame_usage 1

int glXGetFrameUsageMESA(Display *dpy, GLXDrawable drawable, float *usage);
int glXBeginFrameTrackingMESA(Display *dpy, GLXDrawable drawable);
int glXEndFrameTrackingMESA(Display *dpy, GLXDrawable drawable);
int glXQueryFrameTrackingMESA(Display *dpy, GLXDrawable drawable, int64_t *swapCount, int64_t *missedFrames, float *lastMissedUsage);

typedef int (*PFNGLXGETFRAMEUSAGEMESAPROC) (Display *dpy, GLXDrawable drawable, float *usage);
typedef int (*PFNGLXBEGINFRAMETRACKINGMESAPROC)(Display *dpy, GLXDrawable drawable);
typedef int (*PFNGLXENDFRAMETRACKINGMESAPROC)(Display *dpy, GLXDrawable drawable);
typedef int (*PFNGLXQUERYFRAMETRACKINGMESAPROC)(Display *dpy, GLXDrawable drawable, int64_t *swapCount, int64_t *missedFrames, float *lastMissedUsage);

#endif /* GLX_MESA_swap_frame_usage */


/* GLX_MESA_swap_control */
#ifndef GLX_MESA_swap_control
#define GLX_MESA_swap_control 1

int glXSwapIntervalMESA(unsigned int interval);
int glXGetSwapIntervalMESA(void);

typedef int (*PFNGLXSWAPINTERVALMESAPROC)(unsigned int interval);
typedef int (*PFNGLXGETSWAPINTERVALMESAPROC)(void);

#endif /* GLX_MESA_swap_control */


#endif

// }}}

#if 0

/*
** GLX Events
*/
typedef struct {
    int event_type;		/* GLX_DAMAGED or GLX_SAVED */
    int draw_type;		/* GLX_WINDOW or GLX_PBUFFER */
    unsigned long serial;	/* # of last request processed by server */
    Bool send_event;		/* true if this came for SendEvent request */
    Display *display;		/* display the event was read from */
    GLXDrawable drawable;	/* XID of Drawable */
    unsigned int buffer_mask;	/* mask indicating which buffers are affected */
    unsigned int aux_buffer;	/* which aux buffer was affected */
    int x, y;
    int width, height;
    int count;			/* if nonzero, at least this many more */
} GLXPbufferClobberEvent;

typedef union __GLXEvent {
    GLXPbufferClobberEvent glxpbufferclobber;
    long pad[24];
} GLXEvent;

#endif


/*** DEBUG ***/


CAMLprim value
ml_glXGetFBConfigs2( value dpy, value screen )
{
    CAMLparam2( dpy, screen );
    CAMLlocal1( ml_fbconfigs );

    int i, nfbconfigs;
    GLXFBConfig *fbconfigs;  /* XXX here probably a memory leek */

    fbconfigs = glXGetFBConfigs(
            Display_val(dpy),
            ScreenNB_val(screen),
            &nfbconfigs );

    ml_fbconfigs = caml_alloc(nfbconfigs, 0);

    for (i = 0; i < nfbconfigs; i++)
    {
        Store_field( ml_fbconfigs, i, Val_GLXFBConfig( fbconfigs[i] ) );
    }

    CAMLreturn( ml_fbconfigs );
}


static const int drawable_type_table[] = {
    GLX_WINDOW_BIT,
    GLX_PIXMAP_BIT,
    GLX_PBUFFER_BIT
};
#define Drawable_type_val(v) drawable_type_table[Long_val(v)]


static const int bind_to_tex_target_table[] = {
    GLX_TEXTURE_1D_BIT_EXT,
    GLX_TEXTURE_2D_BIT_EXT,
    GLX_TEXTURE_RECTANGLE_BIT_EXT
};
#define Bind_to_tex_target_val(v) bind_to_tex_target_table[Long_val(v)]

/*
type attribute_and_value =
  | D_GLX_DRAWABLE_TYPE of drawable_type
  | D_GLX_BIND_TO_TEXTURE_TARGETS_EXT of bind_to_tex_target
  | D_GLX_BIND_TO_TEXTURE_RGBA_EXT of bool
  | D_GLX_BIND_TO_TEXTURE_RGB_EXT of bool
  | D_GLX_Y_INVERTED_EXT of bool
*/

CAMLprim value
ml_glXHasFBConfigAttrib( value dpy, value fbconfig, value attr_n_val )
{
    value v = attr_n_val;
    int _value;
    int attrib;
    int test_value;
    test_value = -1;

    if (Is_block(v))
    {
       switch (Tag_val(v))
       {
          case 0: /* GLX_DRAWABLE_TYPE of drawable_type */
              attrib = GLX_DRAWABLE_TYPE;
              test_value = Drawable_type_val(Field(v,0));
              break;
          case 1: /* GLX_BIND_TO_TEXTURE_TARGETS_EXT of bind_to_tex_target */
              attrib = GLX_BIND_TO_TEXTURE_TARGETS_EXT;
              test_value = Bind_to_tex_target_val(Field(v,0));
              break;

          case 2: /* GLX_BIND_TO_TEXTURE_RGBA_EXT of bool */
              attrib = GLX_BIND_TO_TEXTURE_RGBA_EXT;
              test_value = Bool_val(Field(v,0));
              break;

          case 3: /* GLX_BIND_TO_TEXTURE_RGB_EXT of bool */
              attrib = GLX_BIND_TO_TEXTURE_RGB_EXT;
              test_value = Bool_val(Field(v,0));
              break;

          case 4: /* GLX_Y_INVERTED_EXT of bool */
              attrib = GLX_Y_INVERTED_EXT;
              test_value = Bool_val(Field(v,0));
              break;

          default: caml_failwith("glXHasFBConfigAttrib: variant handling bug");
       }
    }
    else {caml_failwith("glXHasFBConfigAttrib: variant bug");}
    if (test_value == -1) caml_failwith("glXHasFBConfigAttrib: enum bug");

    glXGetFBConfigAttrib(
            Display_val(dpy),
            GLXFBConfig_val(fbconfig),
            attrib,
            &_value
    );
    if (_value & test_value) {
        return Val_true;
    } else {
        return Val_false;
    }
}

/* TODO */
#if 1
CAMLprim value
/* static GLXFBConfig */
ml_ChoosePixmapFBConfig( value dpy )
{
   CAMLparam1( dpy );
   CAMLlocal1( res );

   Display *display = Display_val(dpy);
   int screen = DefaultScreen(display);
   GLXFBConfig *fbconfigs;
   int i, nfbconfigs, val;
   float top, bottom;

   fbconfigs = glXGetFBConfigs(display, screen, &nfbconfigs);
   for (i = 0; i < nfbconfigs; i++) {

      glXGetFBConfigAttrib(display, fbconfigs[i], GLX_DRAWABLE_TYPE, &val);
      if (!(val & GLX_PIXMAP_BIT))
         continue;

      glXGetFBConfigAttrib(display, fbconfigs[i], GLX_BIND_TO_TEXTURE_TARGETS_EXT, &val);
      if (!(val & GLX_TEXTURE_2D_BIT_EXT))
         continue;

      glXGetFBConfigAttrib(display, fbconfigs[i], GLX_BIND_TO_TEXTURE_RGBA_EXT, &val);
      if (val == False) {
         glXGetFBConfigAttrib(display, fbconfigs[i], GLX_BIND_TO_TEXTURE_RGB_EXT, &val);
         if (val == False)
            continue;
      }

      glXGetFBConfigAttrib(display, fbconfigs[i], GLX_Y_INVERTED_EXT, &val);
      if (val == True) {
         top = 0.0f;
         bottom = 1.0f;
      }
      else {
         top = 1.0f;
         bottom = 0.0f;
      }

      break;
   }

   if (i == nfbconfigs) {
      printf("Unable to find FBconfig for texturing\n");
      exit(1);
   }

   res = caml_alloc(3, 0);
   Store_field( res, 0, Val_GLXFBConfig( fbconfigs[i] ) );
   Store_field( res, 1, caml_copy_double( top ) );
   Store_field( res, 2, caml_copy_double( bottom ) );
   CAMLreturn( res );
}
#endif


// vim: sw=4 sts=4 ts=4 et fdm=marker
