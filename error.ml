(* error.ml, from: http://www.u-picardie.fr/~ferment/xwindow/erreur.htm *)
(* tests the error handler *)
open Xlib

let errorHandler ~dpy ~event =
  Printf.printf "#### ERROR HANDLER ####\n%!";
;;

let () =
  let display = xOpenDisplay "" in
  let screen = xDefaultScreen display in
  let gc = xDefaultGC display screen in
  let root = xRootWindow display screen in
  let white_pixel = xWhitePixel display screen in
  let black_pixel = xBlackPixel display screen in
  let win = xCreateSimpleWindow display root 0 0 200 150 2
                                black_pixel white_pixel in
  xSelectInput display win [ExposureMask; ButtonPressMask];
  xStoreName display win "erreur";
  xMapWindow display win;

  (* will produce an error which will be handled by the error handler *)
  let wim = (Obj.magic 0 : Xlib.window) in

  xSetErrorHandler errorHandler;

  let ev = new_xEvent() in
  while true do
    xNextEvent display ev;
    match xEventType ev with
    | Expose ->
        xDrawString display win gc 10 30 "Hello !";

    | ButtonPress ->
        let d = xButtonEvent_datas(to_xButtonEvent ev) in
        if ((d.button_x + d.button_y) mod 2) = 0 then begin
          xDrawString display win gc (d.button_x)     (d.button_y) "hi";
          xDrawString display win gc (d.button_x +20) (d.button_y) "ho";
        end else begin
          xDrawString display win gc (d.button_x)     (d.button_y) "hi";
          xDrawString display wim gc (d.button_x +20) (d.button_y) "ho";
        end;

    | _ -> ()
  done;
;;

