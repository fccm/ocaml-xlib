(*
 * simple-text.ml - demonstrate drawing of text strings on a window. All
 *		    drawings are done in black color over a white background.
 *)
(* 
 * Program found here:
 * http://users.actcom.co.il/%7Echoo/lupg/tutorials/xlib-programming/xlib-programming.html
 * http://users.actcom.co.il/%7Echoo/lupg/tutorials/xlib-programming/simple-text.c
 * and converted to OCaml.
 *)
open Xlib

(*
 * function: create_simple_window. Creates a window with a white background
 *           in the given size.
 * input:    display, size of the window (in pixels), and location of the window
 *           (in pixels).
 * output:   the window's ID.
 * notes:    window is created with a black border, 2 pixels wide.
 *           the window is automatically mapped after its creation.
 *)
let create_simple_window display width height x y =
  let screen_num = xDefaultScreen display in
  let win_border_width = 2 in

  (* create a simple window, as a direct child of the screen's *)
  (* root window. Use the screen's black and white colors as   *)
  (* the foreground and background colors of the window,       *)
  (* respectively. Place the new window's top-left corner at   *)
  (* the given 'x,y' coordinates.                              *)
  let win = xCreateSimpleWindow display (xRootWindow display screen_num)
                                x y width height win_border_width
                                (xBlackPixel display screen_num)
                                (xWhitePixel display screen_num) in

  (* make the window actually appear on the screen. *)
  xMapWindow display win;

  (* flush all pending requests to the X server. *)
  xFlush display;

  (win)
;;

let create_gc display win reverse_video =
  let line_width = 2 in                 (* line width for the GC.       *)
  let line_style = LineSolid in         (* style for lines drawing and  *)
  let cap_style = CapButt in            (* style of the line's edje and *)
  let join_style = JoinBevel in         (*  joined lines.		*)
  let screen_num = xDefaultScreen display in

  let gc = xCreateGC display win [] (new_xGCValues()) in

  (* allocate foreground and background colors for this GC. *)
  if reverse_video then begin
    xSetForeground display gc (xWhitePixel display screen_num);
    xSetBackground display gc (xBlackPixel display screen_num);
  end
  else begin
    xSetForeground display gc (xBlackPixel display screen_num);
    xSetBackground display gc (xWhitePixel display screen_num);
  end;

  (* define the style of lines that will be drawn using this GC. *)
  xSetLineAttributes display gc
                     line_width line_style cap_style join_style;

  (* define the fill style for the GC. to be 'solid filling'. *)
  xSetFillStyle display gc FillSolid;

  (gc)
;;


let () =
  let display_name = Unix.getenv "DISPLAY" in  (* address of the X display.     *)
  let font_name = "*-helvetica-*-12-*" in      (* font to use for drawing text. *)

  (* open connection with the X server. *)
  let display = xOpenDisplay display_name in

  (* get the geometry of the default screen for our display. *)
  let screen_num = xDefaultScreen display in
  let display_width = xDisplayWidth display screen_num
  and display_height = xDisplayHeight display screen_num in

  (* make the new window occupy 1/9 of the screen's size. *)
  let win_width = (display_width / 3)
  and win_height = (display_height / 3) in
  Printf.printf "window width - '%d'; height - '%d'\n%!" win_width win_height;

  (* create a simple window, as a direct child of the screen's   *)
  (* root window. Use the screen's white color as the background *)
  (* color of the window. Place the new window's top-left corner *)
  (* at the given 'x,y' coordinates.                             *)
  let win = create_simple_window display win_width win_height 0 0 in

  (* allocate a new GC (graphics context) for drawing in the window. *)
  let gc = create_gc display win false in
  xSync display false;

  (* try to load the given font. *)
  let font_info = xLoadQueryFont display font_name in

  (* assign the given font to our GC. *)
  xSetFont display gc (xFontStruct_font font_info);

  begin
    (* find the height of the characters drawn using this font.        *)
    let font_height = (xFontStruct_ascent font_info) +
                      (xFontStruct_descent font_info) in

    (* draw a "hello world" string on the top-left side of our window. *)
    let text_string = "hello world" in
    let x = 0 in
    let y = font_height in
    xDrawString display win gc x y text_string;

    (* draw a "middle of the road" string in the middle of our window. *)
    let text_string = "middle of the road" in
    (* find the width, in pixels, of the text that will be drawn using *)
    (* the given font.                                                 *)
    let string_width = xTextWidth font_info text_string in
    let x = (win_width - string_width) / 2
    and y = (win_height + font_height) / 2 in
    xDrawString display win gc x y text_string;
  end;

  (* flush all pending requests to the X server. *)
  xFlush display;

  (* make a delay for a short period. *)
  Unix.sleep 4;

  (* close the connection to the X server. *)
  xCloseDisplay display;
;;

