(** OCaml bindings to the GLX_EXT_texture_from_pixmap extension *)
(* Copyright (C) 2008, 2009 by Florent Monnier
   Contact: ("monnier.florent@" ^ "gmail.com")
  
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
  {b Warning:} This module is currently under construction.

  This module is a binding to this GLX extension:
  {{:http://www.opengl.org/registry/specs/EXT/texture_from_pixmap.txt}GLX_EXT_texture_from_pixmap}
*)

external init_glx_p2t: unit -> unit = "ml_init_glx_p2t"
let _ = init_glx_p2t() ;;

type buffer_param =
  | GLX_FRONT_LEFT_EXT
  | GLX_FRONT_RIGHT_EXT
  | GLX_BACK_LEFT_EXT
  | GLX_BACK_RIGHT_EXT
  | GLX_FRONT_EXT
  | GLX_BACK_EXT
  | GLX_AUX0_EXT
  | GLX_AUX1_EXT
  | GLX_AUX2_EXT
  | GLX_AUX3_EXT
  | GLX_AUX4_EXT
  | GLX_AUX5_EXT
  | GLX_AUX6_EXT
  | GLX_AUX7_EXT
  | GLX_AUX8_EXT
  | GLX_AUX9_EXT

external glXBindTexImageEXT: dpy:Xlib.display -> drawable:'a GLX.glXDrawable -> buffer:buffer_param ->
    attrib_list:'a list (* TODO *) -> unit
    = "ml_glXBindTexImageEXT"

external glXReleaseTexImageEXT: dpy:Xlib.display -> drawable:'a GLX.glXDrawable -> buffer:buffer_param -> unit
    = "ml_glXReleaseTexImageEXT"


type glx_texture_target_ext =
  | GLX_TEXTURE_1D_EXT
  | GLX_TEXTURE_2D_EXT
  | GLX_TEXTURE_RECTANGLE_EXT

type glx_texture_format_ext =
  | GLX_TEXTURE_FORMAT_NONE_EXT
  | GLX_TEXTURE_FORMAT_RGB_EXT
  | GLX_TEXTURE_FORMAT_RGBA_EXT

type glx_create_pixmap_attrib =
  | GLX_TEXTURE_FORMAT_EXT of glx_texture_format_ext
  | GLX_TEXTURE_TARGET_EXT of glx_texture_target_ext
  | GLX_MIPMAP_TEXTURE_EXT of bool

external glXCreatePixmapEXT: dpy:Xlib.display -> config:GLX.glXFBConfig ->
                             pixmap:Xlib.pixmap -> attribs:'a list -> GLX.glXPixmap = "ml_glXCreatePixmapEXT"


