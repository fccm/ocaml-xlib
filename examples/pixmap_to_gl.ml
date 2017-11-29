(*
  A nice hack (converted from C to ocaml) got from:
  http://www.opengl.org/wiki/Programming_OpenGL_in_Linux:_Creating_a_texture_from_a_Pixmap
*)
open Xlib
open GL
open Glu
open GLX

open TexEnv

let pixmap_width = 128
let pixmap_height = 128

let redraw ~dpy ~win ~gwin =
  let wa = get_win_attrs dpy win in
  let width = wa.wattr_width
  and height = wa.wattr_height in

  glViewport 0 0 width height;
  glClearColor 0.3 0.3 0.3 1.0;
  glClear[GL_COLOR_BUFFER_BIT; GL_DEPTH_BUFFER_BIT];

  glMatrixMode GL_PROJECTION;
  glLoadIdentity();
  glOrtho(-1.25) (1.25) (-1.25) (1.25) (1.) (20.);

  glMatrixMode GL_MODELVIEW;
  glLoadIdentity();
  gluLookAt 0. 0. 10. 0. 0. 0. 0. 1. 0.;

  glColor3 1.0 1.0 1.0;

  glBegin GL_QUADS;
    glTexCoord2 0.0 0.0; glVertex3 (-1.0) ( 1.0) (0.0);
    glTexCoord2 1.0 0.0; glVertex3 ( 1.0) ( 1.0) (0.0);
    glTexCoord2 1.0 1.0; glVertex3 ( 1.0) (-1.0) (0.0);
    glTexCoord2 0.0 1.0; glVertex3 (-1.0) (-1.0) (0.0);
  glEnd();

  glXSwapBuffers dpy gwin;
;;


(* main *)
let () =
  let dpy = xOpenDisplay "" in

  let root = xDefaultRootWindow dpy in
  let scr = xDefaultScreen dpy in

  let att = [
    Visual.  GLX_RGBA;
    Visual.  GLX_DEPTH_SIZE 24;
    Visual.  GLX_DOUBLEBUFFER
  ] in
  let vi = glXChooseVisual dpy scr att in
  let vid = xVisualInfo_datas vi in

  let swa = new_win_attr() in
  swa.set_event_mask [ExposureMask; KeyPressMask];
  swa.set_colormap (xCreateColormap dpy root vid.visual AllocNone);

  let win = xCreateWindow dpy root 0 0 600 600 0 vid.depth InputOutput
                          vid.visual [CWEventMask; CWColormap] swa.attr in
  xMapWindow dpy win;
  xStoreName dpy win "PIXMAP TO TEXTURE";

  let glc = glXCreateContext dpy vi None true in

  let gwin = glXDrawable_of_window win in
  glXMakeCurrent dpy gwin glc;
  glEnable GL_DEPTH_TEST;

  (* create a pixmap and draw something *)

  let pixmap = xCreatePixmap dpy root pixmap_width pixmap_height vid.depth in
  let gc = xDefaultGC dpy scr in

  xSetForeground dpy gc (Obj.magic 0x00c0c0);
  xFillRectangle dpy pixmap gc 0 0 pixmap_width pixmap_height;

  xSetForeground dpy gc (Obj.magic 0x000000);
  xFillArc dpy pixmap gc 15 25 50 50 0 (360*64);

  xSetForeground dpy gc (Obj.magic 0x0000ff);
  xDrawString dpy pixmap gc 10 15 "PIXMAP TO TEXTURE";

  xSetForeground dpy gc (Obj.magic 0xff0000);
  xFillRectangle dpy pixmap gc 75 75 45 35;

  xFlush dpy;
  let xim = xGetImage dpy pixmap 0 0 pixmap_width pixmap_height (xAllPlanes()) ZPixmap in

  (* create texture from pixmap *)
  glEnable GL_TEXTURE_2D;
  let texture_id = glGenTexture() in
  glBindTexture BindTex.GL_TEXTURE_2D texture_id;
  glTexParameter TexParam.GL_TEXTURE_2D (TexParam.GL_TEXTURE_MIN_FILTER  Min.GL_LINEAR);
  glTexParameter TexParam.GL_TEXTURE_2D (TexParam.GL_TEXTURE_MAG_FILTER  Mag.GL_LINEAR);
  glTexEnv GL_TEXTURE_ENV GL_TEXTURE_ENV_MODE GL_MODULATE;
  (* using big-array encapsulation *)
  glTexImage2D TexTarget.GL_TEXTURE_2D 0 InternalFormat.GL_RGB pixmap_height pixmap_height
               GL_RGBA GL_UNSIGNED_BYTE (xImage_data_ba xim);
  (*
  (* using string encapsulation *)
  glTexImage2D_str TexTarget.GL_TEXTURE_2D 0 InternalFormat.GL_RGB pixmap_height pixmap_height 0
                   GL_RGBA GL_UNSIGNED_BYTE (xImage_data_str xim);
  *)

  xDestroyImage xim;

  let xev = new_xEvent() in
  while true do
    xNextEvent dpy xev;
    if (xEventType xev) = Expose then redraw ~dpy ~win ~gwin;
    if (xEventType xev) = KeyPress then
    begin
      glXMakeCurrentNone dpy;
      glXDestroyContext dpy glc;
      xDestroyWindow dpy win;
      xCloseDisplay dpy;
      exit 0;
    end;
  done;
;;

