(** OCaml bindings for the GLX library. *)
(* Copyright (C) 2008, 2009 by Florent Monnier
   Contact: ("fmonnier@" ^ "linux-nantes.org")

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
*)

(**
{ul
  {- {{:http://www.opengl.org/documentation/specs/glx/glx1.4.pdf}GLX Specs (pdf)}
       (this one is the more complete documentation source)}
  {- {{:http://www.opengl.org/sdk/docs/man/xhtml/#glX}GLX Mans} /
     {{:http://www.opengl.org/sdk/docs/man/xhtml/glXIntro.xml}GLX intro}
     (on www.opengl.org)
  }
  {- {{:http://publib.boulder.ibm.com/infocenter/systems/index.jsp?topic=/com.ibm.aix.opengl/doc/openglrf/OpenGLXSubs.htm}
       IBM Reference}}
  {- {{:http://techpubs.sgi.com/library/tpl/cgi-bin/getdoc.cgi/0650/bks/SGI_Developer/books/OpenGL_Porting/sgi_html/ch04.html}
       SGI - OpenGL in the X Window System}}
}
*)

(** {3 Types} *)

type uint = int

type glXContext  (* pointer to a structure *)

type glXFBConfigID

type opengl_rendering_modes = GLX_RGBA_BIT | GLX_COLOR_INDEX_BIT
type drawable_type = GLX_WINDOW_BIT | GLX_PIXMAP_BIT | GLX_PBUFFER_BIT
type x_visual_type =
 | GLX_TRUE_COLOR | GLX_DIRECT_COLOR | GLX_PSEUDO_COLOR | GLX_STATIC_COLOR
 | GLX_GRAY_SCALE | GLX_STATIC_GRAY
type config_caveat = GLX_CONFIG_CAVEAT_NONE | GLX_SLOW_CONFIG | GLX_NON_CONFORMANT_CONFIG
type transparent_type = GLX_TRANSPARENT_NONE | GLX_TRANSPARENT_RGB | GLX_TRANSPARENT_INDEX

module FBConfig = struct
type attrib =
  | GLX_FBCONFIG_ID of glXFBConfigID  (* TODO: check me *)
  | GLX_BUFFER_SIZE of uint
  | GLX_LEVEL of int
  | GLX_DOUBLEBUFFER of bool
  | GLX_STEREO of bool
  | GLX_AUX_BUFFERS of uint
  | GLX_RED_SIZE of uint
  | GLX_GREEN_SIZE of uint
  | GLX_BLUE_SIZE of uint
  | GLX_ALPHA_SIZE of uint
  | GLX_DEPTH_SIZE of uint
  | GLX_STENCIL_SIZE of uint
  | GLX_ACCUM_RED_SIZE of uint
  | GLX_ACCUM_GREEN_SIZE of uint
  | GLX_ACCUM_BLUE_SIZE of uint
  | GLX_ACCUM_ALPHA_SIZE of int
  | GLX_RENDER_TYPE of opengl_rendering_modes list
  | GLX_DRAWABLE_TYPE of drawable_type list
  | GLX_X_RENDERABLE of bool
  | GLX_X_VISUAL_TYPE of x_visual_type
  | GLX_CONFIG_CAVEAT of config_caveat
  | GLX_TRANSPARENT_TYPE of transparent_type
  | GLX_TRANSPARENT_INDEX_VALUE of int
  | GLX_TRANSPARENT_RED_VALUE of int
  | GLX_TRANSPARENT_GREEN_VALUE of int
  | GLX_TRANSPARENT_BLUE_VALUE of int
  | GLX_TRANSPARENT_ALPHA_VALUE of int
end
;;

module Visual = struct
type attrib =
  | GLX_USE_GL
  | GLX_BUFFER_SIZE of uint
  | GLX_LEVEL of int
  | GLX_RGBA
  | GLX_DOUBLEBUFFER
  | GLX_STEREO
  | GLX_AUX_BUFFERS of uint
  | GLX_RED_SIZE of uint
  | GLX_GREEN_SIZE of uint
  | GLX_BLUE_SIZE of uint
  | GLX_ALPHA_SIZE of uint
  | GLX_DEPTH_SIZE of uint
  | GLX_STENCIL_SIZE of uint
  | GLX_ACCUM_RED_SIZE of uint
  | GLX_ACCUM_GREEN_SIZE of uint
  | GLX_ACCUM_BLUE_SIZE of uint
  | GLX_ACCUM_ALPHA_SIZE of int
end
;;

(** This parameter is in fact an OpenGL GL-enum with a GL_ prefix.
    This enum is included here in order to avoid a dependency across
    any ocaml-opengl bindings (there are 3 ones).
    The GL_ prefix was changed for a GLX_ prefix in order to not collide
    with this parameter in the OpenGL module. *)
type attrib_bit =
  | GLX_ACCUM_BUFFER_BIT
  | GLX_COLOR_BUFFER_BIT
  | GLX_CURRENT_BIT
  | GLX_DEPTH_BUFFER_BIT
  | GLX_ENABLE_BIT
  | GLX_EVAL_BIT
  | GLX_FOG_BIT
  | GLX_HINT_BIT
  | GLX_LIGHTING_BIT
  | GLX_LINE_BIT
  | GLX_LIST_BIT
  | GLX_MULTISAMPLE_BIT
  | GLX_PIXEL_MODE_BIT
  | GLX_POINT_BIT
  | GLX_POLYGON_BIT
  | GLX_POLYGON_STIPPLE_BIT
  | GLX_SCISSOR_BIT
  | GLX_STENCIL_BUFFER_BIT
  | GLX_TEXTURE_BIT
  | GLX_TRANSFORM_BIT
  | GLX_VIEWPORT_BIT


(** {4 glXDrawable} *)

type 'a glXDrawable
type _glXWindow  type glXWindow  = _glXWindow  glXDrawable
type _glXPbuffer type glXPbuffer = _glXPbuffer glXDrawable
type _glXPixmap  type glXPixmap  = _glXPixmap  glXDrawable
(** all \{ glxWindow, glxPixmap, glxPbuffer and Xlib.window \} are some glxDrawable *)

let glXDrawable_of_window ~(win : Xlib.window) = (Obj.magic win : 'a glXDrawable) ;;



(** {3 Initialization} *)

external glXQueryExtension: dpy:Xlib.display -> unit = "ml_glXQueryExtension"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXQueryExtension.xml}man} *)

external glXQueryVersion: dpy:Xlib.display -> int * int = "ml_glXQueryVersion"
(** returns (major, minor) *)


(** {3 GLX Versioning} *)

external glXQueryExtensionsString: dpy:Xlib.display -> screen:Xlib.screen_number -> string
    = "ml_glXQueryExtensionsString"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXQueryExtensionsString.xml}man} *)

type glx_name =
  | GLX_VENDOR
  | GLX_VERSION
  | GLX_EXTENSIONS

external glXQueryServerString: dpy:Xlib.display -> screen:Xlib.screen_number -> name:glx_name -> string
    = "ml_glXQueryServerString"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXQueryServerString.xml}man} *)

external glXGetClientString: dpy:Xlib.display -> name:glx_name -> string = "ml_glXGetClientString"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXGetClientString.xml}man} *)


(** {3 Configuration Management} *)

type glXFBConfig

external glXChooseFBConfig: dpy:Xlib.display -> screen:Xlib.screen_number ->
    attribs:FBConfig.attrib list -> glXFBConfig list = "ml_glXChooseFBConfig"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXChooseFBConfig.xml}man}

    To free the resources associated with the returned list, provide this list
    unchanged to {xFree_glXFBConfig} *)

external glXGetFBConfigs: dpy:Xlib.display -> screen:Xlib.screen_number -> glXFBConfig list = "ml_glXGetFBConfigs"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXGetFBConfigs.xml}man}

    To free the resources associated with the returned list, provide this list
    unchanged to {xFree_glXFBConfig} *)

(* TODO: Would it be possible to make it finalised ? *)
external xFree_glXFBConfig: glXFBConfig list -> unit = "ml_XFree_glXFBConfig"
(** Frees the resources associated with the list returned by [glXGetFBConfigs]
    or [glXChooseFBConfig], you have to provide the list unchanged. *)

external glXGetVisualFromFBConfig: dpy:Xlib.display -> config:glXFBConfig -> Xlib.xVisualInfo = "ml_glXGetVisualFromFBConfig"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXGetVisualFromFBConfig.xml}man} *)

(*
int glXGetFBConfigAttrib( Display *dpy, GLXFBConfig config, int attribute, int *value );
*)

(** {3 On Screen Rendering} *)

external glXCreateWindow: dpy:Xlib.display -> config:glXFBConfig -> win:Xlib.window  -> glXWindow = "ml_glXCreateWindow"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXCreateWindow.xml}man} *)

external glXDestroyWindow: dpy:Xlib.display -> win:glXWindow  -> unit = "ml_glXDestroyWindow"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXDestroyWindow.xml}man} *)


(** {3 Off Screen Rendering} *)

external glXCreatePixmap: dpy:Xlib.display -> config:glXFBConfig ->
                          pixmap:Xlib.pixmap -> attribs:'a list -> glXPixmap = "ml_glXCreatePixmap"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXCreatePixmap.xml}man} *)

external glXDestroyPixmap: dpy:Xlib.display -> pixmap:glXPixmap -> unit = "ml_glXDestroyPixmap"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXDestroyPixmap.xml}man} *)

(* Pbuffer *)

type pbuf_attrib =
  | GLX_PBUFFER_WIDTH of int
  | GLX_PBUFFER_HEIGHT of int
  | GLX_LARGEST_PBUFFER of bool
  | GLX_PRESERVED_CONTENTS of bool

external glXCreatePbuffer: dpy:Xlib.display -> config:glXFBConfig -> glXPbuffer = "ml_glXCreatePbuffer"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXCreatePbuffer.xml}man} *)

external glXDestroyPbuffer: dpy:Xlib.display -> pbuf:glXPbuffer -> unit = "ml_glXDestroyPbuffer"


(** {3 Querying Attributes} *)

(* TODO:
type drawable_attr_int =
  | GLX_WIDTH
  | GLX_HEIGHT

type drawable_attr_bool =
  | GLX_PRESERVED_CONTENTS
  | GLX_LARGEST_PBUFFER

type drawable_attr_FBConfigID =
  | GLX_FBCONFIG_ID

external glXQueryDrawableInt: dpy:Xlib.display -> draw:'a glXDrawable -> attribute:drawable_attr_int -> int = "ml_glXQueryDrawable_int"
external glXQueryDrawableBool: dpy:Xlib.display -> draw:'a glXDrawable -> attribute:drawable_attr_bool -> bool = "ml_glXQueryDrawable_bool"
external glXQueryDrawableFBConfigID: dpy:Xlib.display -> draw:'a glXDrawable -> attribute:drawable_attr_FBConfigID -> glXFBConfigID = "ml_glXQueryDrawable_glXFBConfigID"


void glXQueryDrawable( Display *dpy, GLXDrawable draw, int attribute, unsigned int *value );
http://www.opengl.org/sdk/docs/man/xhtml/glXQueryDrawable.xml
*)

(** {3 Rendering Contexts} *)

type render_type = GLX_RGBA_TYPE | GLX_COLOR_INDEX_TYPE

external glXCreateNewContext:
    dpy:Xlib.display ->
    config:glXFBConfig ->
    render_type:render_type ->
    share_list:glXContext option ->
    direct:bool ->
    glXContext = "ml_glXCreateNewContext"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXCreateNewContext.xml}man} *)

external glXDestroyContext: dpy:Xlib.display -> ctx:glXContext -> unit = "ml_glXDestroyContext"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXDestroyContext.xml}man} *)

external glXIsDirect: dpy:Xlib.display -> ctx:glXContext -> bool = "ml_glXIsDirect"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXIsDirect.xml}man} *)


external glXMakeContextCurrent: dpy:Xlib.display -> draw:'a glXDrawable -> read:'a glXDrawable ->
    ctx:glXContext -> unit = "ml_glXMakeContextCurrent"
external glXMakeContextCurrent_release: dpy:Xlib.display -> unit = "ml_glXMakeContextCurrent_release"

(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXMakeContextCurrent.xml}man} *)

external glXCopyContext: dpy:Xlib.display -> ctx:glXContext -> mask:attrib_bit list -> glXContext = "ml_glXCopyContext"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXCopyContext.xml}man} *)

(* TODO:
GLXContext glXGetCurrentContext( void );
http://www.opengl.org/sdk/docs/man/xhtml/glXGetCurrentContext.xml

GLXDrawable glXGetCurrentDrawable( void );
http://www.opengl.org/sdk/docs/man/xhtml/glXGetCurrentDrawable.xml

GLXDrawable glXGetCurrentReadDrawable( void );
http://www.opengl.org/sdk/docs/man/xhtml/glXGetCurrentReadDrawable.xml

Display *glXGetCurrentDisplay( void );
http://www.opengl.org/sdk/docs/man/xhtml/glXGetCurrentDisplay.xml

int glXQueryContext( Display *dpy, GLXContext ctx, int attribute, int *value );
http://www.opengl.org/sdk/docs/man/xhtml/glXQueryContext.xml
*)

(** {3 Events} *)

(* TODO:
void glXSelectEvent( Display *dpy, GLXDrawable drawable, unsigned long mask );
http://www.opengl.org/sdk/docs/man/xhtml/glXSelectEvent.xml
*)

(* TODO:
void glXGetSelectedEvent( Display *dpy, GLXDrawable drawable, unsigned long *mask );
http://www.opengl.org/sdk/docs/man/xhtml/glXGetSelectedEvent.xml
*)

(** {3 Synchronization Primitives} *)

external glXWaitGL: unit -> unit = "ml_glXWaitGL"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXWaitGL.xml}man} *)

external glXWaitX: unit -> unit = "ml_glXWaitX"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXWaitX.xml}man} *)

(** {3 Double Buffering} *)

external glXSwapBuffers: dpy:Xlib.display -> drawable:'a glXDrawable -> unit = "ml_glXSwapBuffers"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXSwapBuffers.xml}man} *)


(** {3 Access to X Fonts} *)

external glXUseXFont: font:Xlib.font -> first:int -> count:int -> gl_list:int -> unit = "ml_glXUseXFont"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXUseXFont.xml}man} *)


(* Obtaining Extension Function Pointers *)
(* TODO:
void (\*glXGetProcAddress(const GLubyte *procname))( void );
*)


(** {2 Backwards Compatibility} *)
(** {3 GLX 1.2 functions which are deprecated in GLX 1.4} *)

(*
int glXGetConfig( Display *dpy, XVisualInfo *visual, int attrib, int *value );
*)
external glXChooseVisual: dpy:Xlib.display -> screen:Xlib.screen_number -> attribs:Visual.attrib list ->
    Xlib.xVisualInfo (* TODO check this returned value *) = "ml_glXChooseVisual"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXChooseVisual.xml}man} *)

(*
GLXPixmap glXCreateGLXPixmap( Display *dpy, XVisualInfo *visual, Pixmap pixmap );
void glXDestroyGLXPixmap( Display *dpy, GLXPixmap pixmap );
*)

external glXCreateContext: dpy:Xlib.display -> vis:Xlib.xVisualInfo -> share_list:glXContext option ->
    direct:bool -> glXContext = "ml_glXCreateContext"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXCreateContext.xml}man} *)

external glXMakeCurrent: dpy:Xlib.display -> drawable:'a glXDrawable -> ctx:glXContext -> unit = "ml_glXMakeCurrent"
external glXMakeCurrentNone: dpy:Xlib.display -> unit = "ml_glXMakeCurrent_none"
(** {{:http://www.opengl.org/sdk/docs/man/xhtml/glXMakeCurrent.xml}man} *)


(** {3 Work in Progress} *)

(** {b Warning:} This module is currently under construction. *)

(* Find where is the place for this function *)
module GetConf = struct
type req_attribute =
  | GLX_FBCONFIG_ID
  | GLX_BUFFER_SIZE
  | GLX_LEVEL
  | GLX_DOUBLEBUFFER
  | GLX_STEREO
  | GLX_AUX_BUFFERS
  | GLX_RED_SIZE
  | GLX_GREEN_SIZE
  | GLX_BLUE_SIZE
  | GLX_ALPHA_SIZE
  | GLX_DEPTH_SIZE
  | GLX_STENCIL_SIZE
  | GLX_ACCUM_RED_SIZE
  | GLX_ACCUM_GREEN_SIZE
  | GLX_ACCUM_BLUE_SIZE
  | GLX_ACCUM_ALPHA_SIZE
  | GLX_RENDER_TYPE
  | GLX_DRAWABLE_TYPE
  | GLX_X_RENDERABLE
  | GLX_VISUAL_ID
  | GLX_X_VISUAL_TYPE
  | GLX_CONFIG_CAVEAT
  | GLX_TRANSPARENT_TYPE
  | GLX_TRANSPARENT_INDEX_VALUE
  | GLX_TRANSPARENT_RED_VALUE
  | GLX_TRANSPARENT_GREEN_VALUE
  | GLX_TRANSPARENT_BLUE_VALUE
  | GLX_TRANSPARENT_ALPHA_VALUE
  | GLX_MAX_PBUFFER_WIDTH
  | GLX_MAX_PBUFFER_HEIGHT
  | GLX_MAX_PBUFFER_PIXELS
end;;


module GetConfEXT = struct
(** should be transfered to GLX_P2T *)
type req_attribute =
  | GLX_FBCONFIG_ID
  | GLX_BUFFER_SIZE
  | GLX_LEVEL
  | GLX_DOUBLEBUFFER
  | GLX_STEREO
  | GLX_AUX_BUFFERS
  | GLX_RED_SIZE
  | GLX_GREEN_SIZE
  | GLX_BLUE_SIZE
  | GLX_ALPHA_SIZE
  | GLX_DEPTH_SIZE
  | GLX_STENCIL_SIZE
  | GLX_ACCUM_RED_SIZE
  | GLX_ACCUM_GREEN_SIZE
  | GLX_ACCUM_BLUE_SIZE
  | GLX_ACCUM_ALPHA_SIZE
  | GLX_RENDER_TYPE
  | GLX_DRAWABLE_TYPE
  | GLX_X_RENDERABLE
  | GLX_VISUAL_ID
  | GLX_X_VISUAL_TYPE
  | GLX_CONFIG_CAVEAT
  | GLX_TRANSPARENT_TYPE
  | GLX_TRANSPARENT_INDEX_VALUE
  | GLX_TRANSPARENT_RED_VALUE
  | GLX_TRANSPARENT_GREEN_VALUE
  | GLX_TRANSPARENT_BLUE_VALUE
  | GLX_TRANSPARENT_ALPHA_VALUE
  | GLX_MAX_PBUFFER_WIDTH
  | GLX_MAX_PBUFFER_HEIGHT
  | GLX_MAX_PBUFFER_PIXELS
  (* GLX_EXT_texture_from_pixmap extension follows: *)
  | GLX_BIND_TO_TEXTURE_TARGETS_EXT
  | GLX_BIND_TO_TEXTURE_RGBA_EXT
  | GLX_BIND_TO_TEXTURE_RGB_EXT
  | GLX_Y_INVERTED_EXT
end;;

(* XXX::DEBUG
let print_getConfEXT_req_attribute = function
  | GetConfEXT.GLX_FBCONFIG_ID             -> print_endline"GetConfEXT.GLX_FBCONFIG_ID"
  | GetConfEXT.GLX_BUFFER_SIZE             -> print_endline"GetConfEXT.GLX_BUFFER_SIZE"
  | GetConfEXT.GLX_LEVEL                   -> print_endline"GetConfEXT.GLX_LEVEL"
  | GetConfEXT.GLX_DOUBLEBUFFER            -> print_endline"GetConfEXT.GLX_DOUBLEBUFFER"
  | GetConfEXT.GLX_STEREO                  -> print_endline"GetConfEXT.GLX_STEREO"
  | GetConfEXT.GLX_AUX_BUFFERS             -> print_endline"GetConfEXT.GLX_AUX_BUFFERS"
  | GetConfEXT.GLX_RED_SIZE                -> print_endline"GetConfEXT.GLX_RED_SIZE"
  | GetConfEXT.GLX_GREEN_SIZE              -> print_endline"GetConfEXT.GLX_GREEN_SIZE"
  | GetConfEXT.GLX_BLUE_SIZE               -> print_endline"GetConfEXT.GLX_BLUE_SIZE"
  | GetConfEXT.GLX_ALPHA_SIZE              -> print_endline"GetConfEXT.GLX_ALPHA_SIZE"
  | GetConfEXT.GLX_DEPTH_SIZE              -> print_endline"GetConfEXT.GLX_DEPTH_SIZE"
  | GetConfEXT.GLX_STENCIL_SIZE            -> print_endline"GetConfEXT.GLX_STENCIL_SIZE"
  | GetConfEXT.GLX_ACCUM_RED_SIZE          -> print_endline"GetConfEXT.GLX_ACCUM_RED_SIZE"
  | GetConfEXT.GLX_ACCUM_GREEN_SIZE        -> print_endline"GetConfEXT.GLX_ACCUM_GREEN_SIZE"
  | GetConfEXT.GLX_ACCUM_BLUE_SIZE         -> print_endline"GetConfEXT.GLX_ACCUM_BLUE_SIZE"
  | GetConfEXT.GLX_ACCUM_ALPHA_SIZE        -> print_endline"GetConfEXT.GLX_ACCUM_ALPHA_SIZE"
  | GetConfEXT.GLX_RENDER_TYPE             -> print_endline"GetConfEXT.GLX_RENDER_TYPE"
  | GetConfEXT.GLX_DRAWABLE_TYPE           -> print_endline"GetConfEXT.GLX_DRAWABLE_TYPE"
  | GetConfEXT.GLX_X_RENDERABLE            -> print_endline"GetConfEXT.GLX_X_RENDERABLE"
  | GetConfEXT.GLX_VISUAL_ID               -> print_endline"GetConfEXT.GLX_VISUAL_ID"
  | GetConfEXT.GLX_X_VISUAL_TYPE           -> print_endline"GetConfEXT.GLX_X_VISUAL_TYPE"
  | GetConfEXT.GLX_CONFIG_CAVEAT           -> print_endline"GetConfEXT.GLX_CONFIG_CAVEAT"
  | GetConfEXT.GLX_TRANSPARENT_TYPE        -> print_endline"GetConfEXT.GLX_TRANSPARENT_TYPE"
  | GetConfEXT.GLX_TRANSPARENT_INDEX_VALUE -> print_endline"GetConfEXT.GLX_TRANSPARENT_INDEX_VALUE"
  | GetConfEXT.GLX_TRANSPARENT_RED_VALUE   -> print_endline"GetConfEXT.GLX_TRANSPARENT_RED_VALUE"
  | GetConfEXT.GLX_TRANSPARENT_GREEN_VALUE -> print_endline"GetConfEXT.GLX_TRANSPARENT_GREEN_VALUE"
  | GetConfEXT.GLX_TRANSPARENT_BLUE_VALUE  -> print_endline"GetConfEXT.GLX_TRANSPARENT_BLUE_VALUE"
  | GetConfEXT.GLX_TRANSPARENT_ALPHA_VALUE -> print_endline"GetConfEXT.GLX_TRANSPARENT_ALPHA_VALUE"
  | GetConfEXT.GLX_MAX_PBUFFER_WIDTH       -> print_endline"GetConfEXT.GLX_MAX_PBUFFER_WIDTH"
  | GetConfEXT.GLX_MAX_PBUFFER_HEIGHT      -> print_endline"GetConfEXT.GLX_MAX_PBUFFER_HEIGHT"
  | GetConfEXT.GLX_MAX_PBUFFER_PIXELS      -> print_endline"GetConfEXT.GLX_MAX_PBUFFER_PIXELS"
  | GetConfEXT.GLX_BIND_TO_TEXTURE_TARGETS_EXT -> print_endline"GetConfEXT.GLX_BIND_TO_TEXTURE_TARGETS_EXT"
  | GetConfEXT.GLX_BIND_TO_TEXTURE_RGBA_EXT    -> print_endline"GetConfEXT.GLX_BIND_TO_TEXTURE_RGBA_EXT"
  | GetConfEXT.GLX_BIND_TO_TEXTURE_RGB_EXT     -> print_endline"GetConfEXT.GLX_BIND_TO_TEXTURE_RGB_EXT"
  | GetConfEXT.GLX_Y_INVERTED_EXT              -> print_endline"GetConfEXT.GLX_Y_INVERTED_EXT"
;;
*)

(* XXX::DEBUG
let print_getConf_req_attribute = function
  | GetConf.GLX_FBCONFIG_ID             -> print_endline"GetConf.GLX_FBCONFIG_ID"
  | GetConf.GLX_BUFFER_SIZE             -> print_endline"GetConf.GLX_BUFFER_SIZE"
  | GetConf.GLX_LEVEL                   -> print_endline"GetConf.GLX_LEVEL"
  | GetConf.GLX_DOUBLEBUFFER            -> print_endline"GetConf.GLX_DOUBLEBUFFER"
  | GetConf.GLX_STEREO                  -> print_endline"GetConf.GLX_STEREO"
  | GetConf.GLX_AUX_BUFFERS             -> print_endline"GetConf.GLX_AUX_BUFFERS"
  | GetConf.GLX_RED_SIZE                -> print_endline"GetConf.GLX_RED_SIZE"
  | GetConf.GLX_GREEN_SIZE              -> print_endline"GetConf.GLX_GREEN_SIZE"
  | GetConf.GLX_BLUE_SIZE               -> print_endline"GetConf.GLX_BLUE_SIZE"
  | GetConf.GLX_ALPHA_SIZE              -> print_endline"GetConf.GLX_ALPHA_SIZE"
  | GetConf.GLX_DEPTH_SIZE              -> print_endline"GetConf.GLX_DEPTH_SIZE"
  | GetConf.GLX_STENCIL_SIZE            -> print_endline"GetConf.GLX_STENCIL_SIZE"
  | GetConf.GLX_ACCUM_RED_SIZE          -> print_endline"GetConf.GLX_ACCUM_RED_SIZE"
  | GetConf.GLX_ACCUM_GREEN_SIZE        -> print_endline"GetConf.GLX_ACCUM_GREEN_SIZE"
  | GetConf.GLX_ACCUM_BLUE_SIZE         -> print_endline"GetConf.GLX_ACCUM_BLUE_SIZE"
  | GetConf.GLX_ACCUM_ALPHA_SIZE        -> print_endline"GetConf.GLX_ACCUM_ALPHA_SIZE"
  | GetConf.GLX_RENDER_TYPE             -> print_endline"GetConf.GLX_RENDER_TYPE"
  | GetConf.GLX_DRAWABLE_TYPE           -> print_endline"GetConf.GLX_DRAWABLE_TYPE"
  | GetConf.GLX_X_RENDERABLE            -> print_endline"GetConf.GLX_X_RENDERABLE"
  | GetConf.GLX_VISUAL_ID               -> print_endline"GetConf.GLX_VISUAL_ID"
  | GetConf.GLX_X_VISUAL_TYPE           -> print_endline"GetConf.GLX_X_VISUAL_TYPE"
  | GetConf.GLX_CONFIG_CAVEAT           -> print_endline"GetConf.GLX_CONFIG_CAVEAT"
  | GetConf.GLX_TRANSPARENT_TYPE        -> print_endline"GetConf.GLX_TRANSPARENT_TYPE"
  | GetConf.GLX_TRANSPARENT_INDEX_VALUE -> print_endline"GetConf.GLX_TRANSPARENT_INDEX_VALUE"
  | GetConf.GLX_TRANSPARENT_RED_VALUE   -> print_endline"GetConf.GLX_TRANSPARENT_RED_VALUE"
  | GetConf.GLX_TRANSPARENT_GREEN_VALUE -> print_endline"GetConf.GLX_TRANSPARENT_GREEN_VALUE"
  | GetConf.GLX_TRANSPARENT_BLUE_VALUE  -> print_endline"GetConf.GLX_TRANSPARENT_BLUE_VALUE"
  | GetConf.GLX_TRANSPARENT_ALPHA_VALUE -> print_endline"GetConf.GLX_TRANSPARENT_ALPHA_VALUE"
  | GetConf.GLX_MAX_PBUFFER_WIDTH       -> print_endline"GetConf.GLX_MAX_PBUFFER_WIDTH"
  | GetConf.GLX_MAX_PBUFFER_HEIGHT      -> print_endline"GetConf.GLX_MAX_PBUFFER_HEIGHT"
  | GetConf.GLX_MAX_PBUFFER_PIXELS      -> print_endline"GetConf.GLX_MAX_PBUFFER_PIXELS"
;;
*)


type _a
(** a fake type to make a hack, read the doc of the function [glXGetFBConfigAttrib] *)

type bind_to_tex_target = (* EXT :: GLX_P2T *)
  | GLX_TEXTURE_1D_BIT_EXT
  | GLX_TEXTURE_2D_BIT_EXT
  | GLX_TEXTURE_RECTANGLE_BIT_EXT

let _GLX_FBCONFIG_ID             = GetConf.GLX_FBCONFIG_ID,             (fun (v : _a) -> (Obj.magic v : glXFBConfigID))
let _GLX_BUFFER_SIZE             = GetConf.GLX_BUFFER_SIZE,             (fun (v : _a) -> (Obj.magic v : bool))
let _GLX_LEVEL                   = GetConf.GLX_LEVEL,                   (fun (v : _a) -> (Obj.magic v : int ))
let _GLX_DOUBLEBUFFER            = GetConf.GLX_DOUBLEBUFFER,            (fun (v : _a) -> (Obj.magic v : bool))
let _GLX_STEREO                  = GetConf.GLX_STEREO,                  (fun (v : _a) -> (Obj.magic v : bool))
let _GLX_AUX_BUFFERS             = GetConf.GLX_AUX_BUFFERS,             (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_RED_SIZE                = GetConf.GLX_RED_SIZE,                (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_GREEN_SIZE              = GetConf.GLX_GREEN_SIZE,              (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_BLUE_SIZE               = GetConf.GLX_BLUE_SIZE,               (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_ALPHA_SIZE              = GetConf.GLX_ALPHA_SIZE,              (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_DEPTH_SIZE              = GetConf.GLX_DEPTH_SIZE,              (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_STENCIL_SIZE            = GetConf.GLX_STENCIL_SIZE,            (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_ACCUM_RED_SIZE          = GetConf.GLX_ACCUM_RED_SIZE,          (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_ACCUM_GREEN_SIZE        = GetConf.GLX_ACCUM_GREEN_SIZE,        (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_ACCUM_BLUE_SIZE         = GetConf.GLX_ACCUM_BLUE_SIZE,         (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_ACCUM_ALPHA_SIZE        = GetConf.GLX_ACCUM_ALPHA_SIZE,        (fun (v : _a) -> (Obj.magic v : int ))
let _GLX_RENDER_TYPE             = GetConf.GLX_RENDER_TYPE,             (fun (v : _a) -> (Obj.magic v : render_type list    ))
let _GLX_DRAWABLE_TYPE           = GetConf.GLX_DRAWABLE_TYPE,           (fun (v : _a) -> (Obj.magic v : drawable_type list  ))
let _GLX_X_RENDERABLE            = GetConf.GLX_X_RENDERABLE,            (fun (v : _a) -> (Obj.magic v : bool                ))
let _GLX_VISUAL_ID               = GetConf.GLX_VISUAL_ID,               (fun (v : _a) -> (Obj.magic v : Xlib.visualID option))
let _GLX_X_VISUAL_TYPE           = GetConf.GLX_X_VISUAL_TYPE,           (fun (v : _a) -> (Obj.magic v : x_visual_type option))
let _GLX_CONFIG_CAVEAT           = GetConf.GLX_CONFIG_CAVEAT,           (fun (v : _a) -> (Obj.magic v : config_caveat list  ))
let _GLX_TRANSPARENT_TYPE        = GetConf.GLX_TRANSPARENT_TYPE,        (fun (v : _a) -> (Obj.magic v : transparent_type    ))
let _GLX_TRANSPARENT_INDEX_VALUE = GetConf.GLX_TRANSPARENT_INDEX_VALUE, (fun (v : _a) -> (Obj.magic v : int ))
let _GLX_TRANSPARENT_RED_VALUE   = GetConf.GLX_TRANSPARENT_RED_VALUE,   (fun (v : _a) -> (Obj.magic v : int ))
let _GLX_TRANSPARENT_GREEN_VALUE = GetConf.GLX_TRANSPARENT_GREEN_VALUE, (fun (v : _a) -> (Obj.magic v : int ))
let _GLX_TRANSPARENT_BLUE_VALUE  = GetConf.GLX_TRANSPARENT_BLUE_VALUE,  (fun (v : _a) -> (Obj.magic v : int ))
let _GLX_TRANSPARENT_ALPHA_VALUE = GetConf.GLX_TRANSPARENT_ALPHA_VALUE, (fun (v : _a) -> (Obj.magic v : int ))
let _GLX_MAX_PBUFFER_WIDTH       = GetConf.GLX_MAX_PBUFFER_WIDTH,       (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_MAX_PBUFFER_HEIGHT      = GetConf.GLX_MAX_PBUFFER_HEIGHT,      (fun (v : _a) -> (Obj.magic v : uint))
let _GLX_MAX_PBUFFER_PIXELS      = GetConf.GLX_MAX_PBUFFER_PIXELS,      (fun (v : _a) -> (Obj.magic v : uint))

  (* GLX_EXT_texture_from_pixmap extension *)
let _GLX_BIND_TO_TEXTURE_TARGETS_EXT = GetConfEXT.GLX_BIND_TO_TEXTURE_TARGETS_EXT, (fun (v : _a) ->
                                                                                     (Obj.magic v : bind_to_tex_target list))
let _GLX_BIND_TO_TEXTURE_RGBA_EXT    = GetConfEXT.GLX_BIND_TO_TEXTURE_RGBA_EXT,    (fun (v : _a) -> (Obj.magic v : bool))
let _GLX_BIND_TO_TEXTURE_RGB_EXT     = GetConfEXT.GLX_BIND_TO_TEXTURE_RGB_EXT,     (fun (v : _a) -> (Obj.magic v : bool))
let _GLX_Y_INVERTED_EXT              = GetConfEXT.GLX_Y_INVERTED_EXT,              (fun (v : _a) -> (Obj.magic v : bool))


(* private function *)
external glXGetFBConfigAttrib: dpy:Xlib.display -> config:glXFBConfig -> attrib:GetConf.req_attribute -> _a
    = "ml_glXGetFBConfigAttrib"

(* "ml_glXGetFBConfigAttrib" is compatible with both *)
external glXGetFBConfigAttribEXT: dpy:Xlib.display -> config:glXFBConfig -> attrib:GetConfEXT.req_attribute -> _a
    = "ml_glXGetFBConfigAttrib"

(* sig:
val glXGetFBConfigAttrib: dpy:Xlib.display -> config:glXFBConfig -> attrib:(GetConf.req_attribute * ('a -> 'b)) -> 'b
*)
let glXGetFBConfigAttrib ~dpy ~config ~attrib =
  let req, func = attrib in
  let v = glXGetFBConfigAttrib ~dpy ~config ~attrib:req in
  (func v)
;;
(** as this function may return different types, instead of using for example
    [GLX_DRAWABLE_TYPE], just use [_GLX_DRAWABLE_TYPE] *)

let glXGetFBConfigAttribEXT ~dpy ~config ~attrib =
  let req, func = attrib in
  let v = glXGetFBConfigAttribEXT ~dpy ~config ~attrib:req in
  print_endline "A1";
  (func v)
;;
(** same than [glXGetFBConfigAttrib] but with additions from the GLX_EXT_texture_from_pixmap extension

    should be transfered to GLX_P2T *)


(** {3 Debug} *)

type glXFBConfig2

external glXGetFBConfigs2: dpy:Xlib.display -> Xlib.screen_number -> glXFBConfig2 array = "ml_glXGetFBConfigs2"

type attribute_and_value =
  | D_GLX_DRAWABLE_TYPE of drawable_type
  | D_GLX_BIND_TO_TEXTURE_TARGETS_EXT of bind_to_tex_target
  | D_GLX_BIND_TO_TEXTURE_RGBA_EXT of bool
  | D_GLX_BIND_TO_TEXTURE_RGB_EXT of bool
  | D_GLX_Y_INVERTED_EXT of bool

external glXHasFBConfigAttrib: dpy:Xlib.display -> glXFBConfig2 -> attribute_and_value -> bool = "ml_glXHasFBConfigAttrib"

external choosePixmapFBConfig: dpy:Xlib.display -> glXFBConfig * float * float = "ml_ChoosePixmapFBConfig"

