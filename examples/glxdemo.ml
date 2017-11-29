(* A demonstration of using the GLX functions.
   This program is in the public domain.

   Brian Paul
*)
(* OCaml version F.Monnier *)

open Xlib
open GLX
open GL

let redraw ~dpy ~win =
  print_endline "Redraw event";
  glClear [GL_COLOR_BUFFER_BIT];
  glColor3 1.0 1.0 0.0;
  glRect (-0.8) (-0.8) (0.8) (0.8);
  glXSwapBuffers dpy (glXDrawable_of_window win);
;;


let resize ~width ~height =
  print_endline "Resize event";
  glViewport 0 0 width height;
  glMatrixMode GL_PROJECTION;
  glLoadIdentity();
  glOrtho (-1.0) (1.0) (-1.0) (1.0) (-1.0) (1.0);
;;


let make_rgb_db_window ~dpy ~width ~height =
  let attrib = [
    Visual.  GLX_RGBA;
    Visual.  GLX_RED_SIZE (1);
    Visual.  GLX_GREEN_SIZE (1);
    Visual.  GLX_BLUE_SIZE (1);
    Visual.  GLX_DOUBLEBUFFER
  ] in

  let scrnum = xDefaultScreen ~dpy in
  let root_win = xRootWindow dpy scrnum in

  let visinfo = glXChooseVisual dpy scrnum attrib in

  let border =     xBlackPixel dpy scrnum
  and background = xWhitePixel dpy scrnum in

  let win = xCreateSimpleWindow dpy root_win 20 20 width height 1
                                border background in

  xSelectInput dpy win (PointerMotionMask ::
                        [ExposureMask; KeyPressMask; StructureNotifyMask]);

  let ctx = glXCreateContext dpy visinfo None true in
  glXMakeCurrent dpy (glXDrawable_of_window win) ctx;
  (win)
;;


let event_loop ~dpy ~win =
  let event = new_xEvent() in
  while true do
    xNextEvent dpy event;
    match xEventType event with
    | Expose ->
        redraw dpy win;

    | ConfigureNotify ->
        let xconfigure = to_xConfigureEvent event in
        let d = xConfigureEvent_datas xconfigure in
        resize d.conf_width d.conf_height;

    | MotionNotify ->
        let m = xMotionEvent_datas(to_xMotionEvent event) in
        (* Printf.printf "(%d,%d)%!" m.motion_x m.motion_y; *)
        ignore(m);

    | _ -> ()
  done;
;;


let () =
  let dpy = xOpenDisplay "" in
  let win = make_rgb_db_window dpy 300 300 in

  glShadeModel GL_FLAT;
  glClearColor 0.5 0.5 0.5 1.0;

  xMapWindow dpy win;

  event_loop dpy win;
;;

