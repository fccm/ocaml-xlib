(* Mesa 3-D graphics library
 * Version:  7.1
 * 
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * The Software is provided "AS IS", without warranty of any kind, express
 * or implied, including but not limited to the warranties of merchantability,
 * fitness for a particular purpose and noninfringement.  In no event shall
 * Brian Paul be liable for any claim, damages or other liability, whether in
 * an action of contract, tort or otherwise, arising from, out of or in
 * connection with the Software or the use or other dealings in the Software.
 *)

(* Test the GLX_EXT_texture_from_pixmap extension
 * Brian Paul
 * 19 May 2007
 *)

(* Converted from C to OCaml by
 * Florent Monnier
 * 26 January 2009
 *)

(* Example: Bind redirected window to texture
 * The documentation of this extension is here:
 * http://www.opengl.org/registry/specs/EXT/texture_from_pixmap.txt
 *)

(* The original version of this file in C is here:
 * http://cgit.freedesktop.org/mesa/mesa/tree/progs/xdemos/texture_from_pixmap.c
 * Or in Mesa sources in the at the location:
 * progs/xdemos/texture_from_pixmap.c
 *)

open GL
open Xlib
open GLX
open GLX_P2T
open Keysym_match

let top = ref 0.0
let bottom = ref 1.0

let invis = xVisualInfo_datas ;;
let ( += ) a b = (a := !a +. b)
let ( -= ) a b = (a := !a -. b)


let open_display() =
  let dpy = xOpenDisplay "" in
  let screen = xDefaultScreen dpy in
  let ext = glXQueryExtensionsString dpy screen in
  let ext_li = Str.split (Str.regexp "[ \n]+") ext in
  if not(List.mem "GLX_EXT_texture_from_pixmap" ext_li) then
    failwith "The extension GLX_EXT_texture_from_pixmap is not supported.";
  (dpy)
;;

(*
exception Found of glXFBConfig
let choose_pixmap_fbconfig ~dpy =
  let screen = xDefaultScreen ~dpy in
  let fbconfigs = glXGetFBConfigs dpy screen in

  try ListLabels.iter fbconfigs ~f:
    (fun config ->
      try
        let v = glXGetFBConfigAttrib ~dpy ~config ~attrib:_GLX_DRAWABLE_TYPE in
        if not(List.mem GLX_PIXMAP_BIT v) then (raise Exit);

        print_endline "A1";
        let v = glXGetFBConfigAttribEXT ~dpy ~config ~attrib:_GLX_BIND_TO_TEXTURE_TARGETS_EXT in
        if not(List.mem GLX_TEXTURE_2D_BIT_EXT v) then (raise Exit);
        print_endline "Z1";

        if (glXGetFBConfigAttribEXT ~dpy ~config ~attrib:_GLX_BIND_TO_TEXTURE_RGBA_EXT) = false
        && (glXGetFBConfigAttribEXT ~dpy ~config ~attrib:_GLX_BIND_TO_TEXTURE_RGB_EXT) = false
        then (raise Exit);

        if (glXGetFBConfigAttribEXT ~dpy ~config ~attrib:_GLX_Y_INVERTED_EXT)
        then ( top := 0.0; bottom := 1.0; )
        else ( top := 1.0; bottom := 0.0; );

        raise (Found config)
      with
      | Exit -> ()
    );
    failwith "Unable to find glXFBConfig for texturing";
  with
  | Found config -> (config)
;;


static GLXFBConfig
let choose_pixmap_fbconfig ~dpy =
  let display = dpy in
  let screen = xDefaultScreen(display) in
  int i, nfbconfigs, value;

  let fbconfigs = glXGetFBConfigs display screen in
  let fbconfigs = Array.of_list fbconfigs in
  let nfbconfigs = Array.length fbconfigs in
  for i=0 to pred nfbconfigs do

     let value = glXGetFBConfigAttrib display fbconfigs.(i) _GLX_DRAWABLE_TYPE in
     val fb_config_has : drawable_types  value GLX_PIXMAP_BIT)
     if (fb_config_has value GLX_PIXMAP_BIT)
        continue;

     glXGetFBConfigAttrib(display, fbconfigs[i],
                          GLX_BIND_TO_TEXTURE_TARGETS_EXT, &value);
     if (!(value & GLX_TEXTURE_2D_BIT_EXT))
        continue;

     glXGetFBConfigAttrib(display, fbconfigs[i],
                          GLX_BIND_TO_TEXTURE_RGBA_EXT, &value);
     if (value == False) {
        glXGetFBConfigAttrib(display, fbconfigs[i],
                             GLX_BIND_TO_TEXTURE_RGB_EXT, &value);
        if (value == False)
           continue;
     }

     glXGetFBConfigAttrib(display, fbconfigs[i],
                          GLX_Y_INVERTED_EXT, &value);
     if (value == True) {
        top = 0.0f;
        bottom = 1.0f;
     }
     else {
        top = 1.0f;
        bottom = 0.0f;
     }

     break;
  done;

  if (i == nfbconfigs) {
     printf("Unable to find FBconfig for texturing\n");
     exit(1);
  }

  return fbconfigs[i];
;;
*)



(*
(* static GLXFBConfig *)
let choose_pixmap_fbconfig ~dpy =
  let display = dpy in
  let screen = xDefaultScreen display in
  let _i = ref 0 in

  let fbconfigs = glXGetFBConfigs2 display screen in
  let nfbconfigs = Array.length fbconfigs in
  for i=0 to pred nfbconfigs do
    _i := i;

    if not(glXHasFBConfigAttrib display fbconfigs.(i) (D_GLX_DRAWABLE_TYPE  GLX_PIXMAP_BIT))
    then () (* continue *) else

    if not(glXHasFBConfigAttrib display fbconfigs.(i) (D_GLX_BIND_TO_TEXTURE_TARGETS_EXT  GLX_TEXTURE_2D_BIT_EXT))
    then () (* continue *) else

    if (glXHasFBConfigAttrib display fbconfigs.(i) (D_GLX_BIND_TO_TEXTURE_RGBA_EXT  false))
    && (glXHasFBConfigAttrib display fbconfigs.(i) (D_GLX_BIND_TO_TEXTURE_RGB_EXT   false))
    then () (* continue *) else

    if (glXHasFBConfigAttrib display fbconfigs.(i) (D_GLX_Y_INVERTED_EXT  true))
    then begin
      top := 0.0;
      bottom := 1.0;
    end else begin
      top := 1.0;
      bottom := 0.0;
    end;
  done;

  if (!_i = nfbconfigs) then begin
    prerr_endline "Unable to find FBconfig for texturing";
    exit 1;
  end;

  (fbconfigs.(!_i))
;;
*)


(*
let choose_pixmap_fbconfig ~dpy =
  let display = dpy in
  let screen = xDefaultScreen(display) in
   GLXFBConfig *fbconfigs;
   int i, nfbconfigs, value;

  let fbconfigs = glXGetFBConfigs2 display screen in
  let nfbconfigs = Array.length fbconfigs in
  let _i = ref 0 in

  begin
   try
    for i = 0 to pred nfbconfigs do

      glXGetFBConfigAttrib(display, fbconfigs[i], GLX_DRAWABLE_TYPE, &value);
      if (!(value & GLX_PIXMAP_BIT))
         continue;

      glXGetFBConfigAttrib(display, fbconfigs[i],
                           GLX_BIND_TO_TEXTURE_TARGETS_EXT, &value);
      if (!(value & GLX_TEXTURE_2D_BIT_EXT))
         continue;

      glXGetFBConfigAttrib(display, fbconfigs[i],
                           GLX_BIND_TO_TEXTURE_RGBA_EXT, &value);
      if (value == False) {
         glXGetFBConfigAttrib(display, fbconfigs[i],
                              GLX_BIND_TO_TEXTURE_RGB_EXT, &value);
         if (value == False)
            continue;
      }

      glXGetFBConfigAttrib(display, fbconfigs[i],
                           GLX_Y_INVERTED_EXT, &value);
      if (value == True) {
         top = 0.0f;
         bottom = 1.0f;
      }
      else {
         top = 1.0f;
         bottom = 0.0f;
      }

      raise Exit;
    done;
   with Exit -> ()
   end;

   if (i == nfbconfigs) {
      printf("Unable to find FBconfig for texturing\n");
      exit(1);
   }

   return fbconfigs[i];
}
*)


let choose_pixmap_fbconfig ~dpy =
  let fbconfig, _top, _bottom = choosePixmapFBConfig ~dpy in
  top    := _top;
  bottom := _bottom;
  (fbconfig)
;;





let create_pixmap ~dpy ~config ~w ~h =
  let pixmapAttribs = [
    (GLX_TEXTURE_TARGET_EXT  GLX_TEXTURE_2D_EXT);
    (GLX_TEXTURE_FORMAT_EXT  GLX_TEXTURE_FORMAT_RGB_EXT);
  ] in
  let root = xRootWindow dpy (xDefaultScreen dpy) in

  let p = xCreatePixmap dpy root w h 24 in
  xSync dpy false;
  let gp = glXCreatePixmapEXT ~dpy ~config:(Obj.magic config) ~pixmap:p ~attribs:pixmapAttribs in (* XXX DEBUG *)
  xSync dpy false;

  (gp, p)
;;


(* In fact this function could be just defined like this
   let col = Obj.magic ;;
   but I think doing the full process is more portable *)
let col dpy v =
  let red   = 257 * ((v land 0xFF0000) lsr 16)
  and green = 257 * ((v land 0x00FF00) lsr  8)
  and blue  = 257 * ((v land 0x0000FF)       )
  in
  let xc = new_xColor() in
  xColor_set_red   xc red;
  xColor_set_green xc green;
  xColor_set_blue  xc blue;
  xColor_set_flags xc [DoRed; DoGreen; DoBlue];
  let s = (xDefaultScreen dpy) in
  xAllocColor dpy (xDefaultColormap dpy s) xc;
  (xColor_pixel xc)
;;


let draw_pixmap_image ~dpy ~pm ~w ~h =
  let gcvals = new_gc_values() in
  let col = col dpy in

  gcvals.set_background (col 0x000000);
  let gc = xCreateGC dpy pm [GCBackground] gcvals.gcValues in

  xSetForeground dpy gc (col 0x000000);
  xFillRectangle dpy pm gc 0 0 w h;

  xSetForeground dpy gc (col 0xff0000);
  xFillRectangle dpy pm gc 0 0 50 50;

  xSetForeground dpy gc (col 0x00ff00);
  xFillRectangle dpy pm gc (w - 50) 0 50 50;

  xSetForeground dpy gc (col 0x0000ff);
  xFillRectangle dpy pm gc 0 (h - 50) 50 50;

  xSetForeground dpy gc (col 0xffffff);
  xFillRectangle dpy pm gc (h - 50) (h - 50) 50 50;

  xSetForeground dpy gc (col 0xffff00);
  xSetLineAttributes dpy gc 3 LineSolid CapButt JoinBevel;
  xDrawLine dpy pm gc 0 0 w h;
  xDrawLine dpy pm gc 0 h w 0;
;;


let choose_window_visual ~dpy =
  let screen = xDefaultScreen dpy in
  let attribs = [
    Visual.  GLX_RGBA;
    Visual.  GLX_RED_SIZE  1;
    Visual.  GLX_GREEN_SIZE  1;
    Visual.  GLX_BLUE_SIZE  1;
    Visual.  GLX_DOUBLEBUFFER;
  ] in
  let visinfo = glXChooseVisual dpy screen attribs in
  (visinfo)
;;


let create_x_window ~dpy ~visinfo ~width ~height ~name =
  let screen = xDefaultScreen dpy in
  let root = xRootWindow dpy screen in

  (* window attributes *)
  let attr = [
    BackPixel (col dpy 0);
    BorderPixel (col dpy 0);
    Colormap (xCreateColormap dpy root (invis visinfo).visual AllocNone);
    EventMask [StructureNotifyMask; ExposureMask; KeyPressMask];
  ] in
  let win = create_window dpy root 0 0 width height 0
                          (invis visinfo).depth InputOutput
       	                  (invis visinfo).visual attr in

  set_normal_hints dpy win [USSize(width, height)];
  set_standard_properties dpy win name name None [| |] [USSize(width, height)];

  xMapWindow dpy win;

  (win)
;;


let bind_pixmap_texture ~dpy ~gp =
  let texture = glGenTexture() in
  glBindTexture BindTex.GL_TEXTURE_2D texture;

  glXBindTexImageEXT dpy gp GLX_FRONT_LEFT_EXT [];

  glTexParameter TexParam.GL_TEXTURE_2D (TexParam.GL_TEXTURE_MIN_FILTER  Min.GL_LINEAR);
  glTexParameter TexParam.GL_TEXTURE_2D (TexParam.GL_TEXTURE_MAG_FILTER  Mag.GL_LINEAR);

  glEnable GL_TEXTURE_2D;
  (*
    glXReleaseTexImageEXT dpy gp GLX_FRONT_LEFT_EXT;
  *)
;;


let resize ~win ~width ~height =
  let sz = 1.5 in
  glViewport 0 0 width height;
  glMatrixMode GL_PROJECTION;
  glLoadIdentity();
  glOrtho (-.sz) (sz) (-.sz) (sz) (-1.0) (1.0);
  glMatrixMode GL_MODELVIEW;
;;


let redraw ~dpy ~win ~rot =
  glClearColor 0.25 0.25 0.25 0.0;
  glClear [GL_COLOR_BUFFER_BIT];
  glPushMatrix();
  glRotate rot 0. 0. 1.;
  glRotate (2.0 *. rot) 1. 0. 0.;

  glBegin GL_QUADS;
    glTexCoord2 0.0 !bottom;  glVertex2 (-1.) (-1.);
    glTexCoord2 1.0 !bottom;  glVertex2 (1.0) (-1.);
    glTexCoord2 1.0 !top;     glVertex2 (1.0) (1.0);
    glTexCoord2 0.0 !top;     glVertex2 (-1.) (1.0);
  glEnd();

  glPopMatrix();

  glXSwapBuffers dpy win;
;;


let event_loop ~dpy ~win =
  let rot = ref 0.0 in
  let anim = ref false in
 
  let event = new_xEvent() in
  while true do
    if (not !anim) || (xPending dpy > 0) then
    begin
      xNextEvent dpy event;

      match xEventKind event with
      | XExposeEvent _ ->
          redraw dpy win !rot;

      | XConfigureEvent event ->
          let a = xAnyEvent_datas event
          and e = xConfigureEvent_datas event in
          resize a.xany_window e.conf_width
                               e.conf_height;

      | XKeyPressedEvent event ->
          let _, keySym = xLookupString ~event ~buffer:"" in
          ( match keysym_var keySym with
            | XK_q | XK_Escape -> exit 0; (* exit *)
            | XK_a -> anim := (not !anim);
            | XK_r ->
                rot += 1.0;
                redraw dpy win !rot;
            | XK_R ->
                rot -= 1.0;
                redraw dpy win !rot;
            | _ -> ())

      | _ -> () (* no-op *)
    end
    else begin
      (* animate *)
      rot += 1.0;
      redraw dpy win !rot;
    end
  done;
;;


(* main *)
let () =
  let dpy = open_display() in

  let pixmapConfig = choose_pixmap_fbconfig ~dpy in
  let windowVis = choose_window_visual dpy in
  let win = create_x_window dpy windowVis 500 500 "Texture From Pixmap" in

  let gp, p = create_pixmap dpy pixmapConfig 512 512 in
  draw_pixmap_image dpy p 512 512;

  let ctx = glXCreateContext dpy windowVis None true in

  glXMakeCurrent dpy (glXDrawable_of_window win) ctx;

  bind_pixmap_texture ~dpy ~gp;

  event_loop ~dpy ~win:(glXDrawable_of_window win);
;;

