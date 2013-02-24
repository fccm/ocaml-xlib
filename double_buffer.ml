(* Demonstration of double-buffer rendering.

   - left clic: with double-buffer
   - right clic: single buffer

   We render to a pixmap and then copy the contents of the pixmap to the
   window with xCopyArea().

   This will be similar to many implementations of glXSwapBuffers, however,
   many GL implementation allow synchronization with the refresh rate,
   while Xlib offers no such feature.
   Note that glXSwapBuffers executes an implicit glFlush, while xCopyArea
   does not execute an implicit xFlush.
*)
open Xlib

let () =
  let width = 500 and height = 500 in
  let dpy = xOpenDisplay "" in

  (* initialisation of the standard variables *)
  let screen = xDefaultScreen dpy in
  let root = xDefaultRootWindow dpy
  and visual = xDefaultVisual dpy screen
  and depth = xDefaultDepth dpy screen
  and fg = xBlackPixel dpy screen
  and bg = xWhitePixel dpy screen
  in

  (* set foreground and background in the graphics context *)
  let gcvalues = new_xGCValues() in
  xGCValues_set_foreground gcvalues fg;
  xGCValues_set_background gcvalues bg;
  let gc = xCreateGC dpy root [GCForeground; GCBackground] gcvalues in

  (* creation of the double buffer *)
  let db = xCreatePixmap dpy root width height depth in
  (* without these lines previous images from memory will appear *)
  xSetForeground dpy gc bg;
  xFillRectangle dpy db gc 0 0 width height;
  xSetForeground dpy gc fg;

  (* window attributes *)
  let xswa = new_win_attr() in

  (* the events we want *)
  xswa.set_event_mask [ExposureMask;ButtonPressMask;ButtonReleaseMask;
                       PointerMotionMask;KeyPressMask];

  (* border and background colors *)
  xswa.set_background_pixel bg;
  xswa.set_border_pixel fg;

  let win = xCreateWindow dpy root
                          100 100 width height 2
                          depth InputOutput visual
                          [CWEventMask;CWBorderPixel;CWBackPixel]
                          xswa.attr in
  (* END *)

  xMapRaised dpy win;

  let report = new_xEvent() in
  while true do
    xNextEvent dpy report;
    match xEventType report with
    | Expose ->
        (* remove all the Expose events from the event stack *)
        while (xCheckTypedEvent dpy Expose report) do () done;
        xCopyArea dpy db win gc 0 0 width height 0 0;
        (* force refresh the screen *)
        xFlush dpy;

    | ButtonPress ->
        let xbutton = xButtonEvent_datas(to_xButtonEvent report) in
        begin match xbutton.button with
        (* left clic *)
        | Button1 ->
            (* animation with the double buffer *)
            for j=0 to pred 100 do
              xSetForeground dpy gc bg;
              xFillRectangle dpy db gc 0 0 width height;
              xSetForeground dpy gc fg;

              for i=0 to pred 100 do
                let i = i  * 4 in
                xDrawArc dpy db gc (xbutton.button_x + j)
                                   (xbutton.button_y + j) i i 0 (360 * 64);
              done;

              xCopyArea dpy db win gc 0 0 width height 0 0;
              (* force refresh the screen *)
              xFlush dpy;
            done;

        (* right and middle clic *)
        | Button2
        | Button3 ->
            (* animation without double buffer *)
            for j=0 to pred 100 do
              xClearWindow dpy win;

              for i=0 to pred 100 do
                let i = i * 4 in
                xDrawArc dpy win gc (xbutton.button_x + j)
                                    (xbutton.button_y + j) i i  0 (360 * 64);
              done;
            done;
        | _ -> ()
        end;

    | KeyPress ->
        (* exit on any key press *)
        xCloseDisplay dpy;
        exit 0;

    | ConfigureNotify -> ()
    | _ -> ()
  done;
;;

