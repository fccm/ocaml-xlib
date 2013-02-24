(* Displays a window filled with small multicolored squares.
 *
 * Author: Pierre Ficheux (pierre@alienor.fr)  November 1999
 * got from: http://pficheux.free.fr/articles/lmf/xlib-images/
 * Converted to OCaml by Florent Monnier
 *)
open Xlib

let width  = 320
let height = 250

let color_names = [|
    "black";
    "red";
    "green";
    "yellow";
    "blue";
    "magenta";
    "cyan";
    "white";
  |]

let () =
  let display = xOpenDisplay "" in

  let screen = xDefaultScreen display in
  let gc = xDefaultGC display screen in
  let root = xRootWindow display screen in
  let white_pixel = xWhitePixel display screen
  and black_pixel = xBlackPixel display screen in
  let cmap = xDefaultColormap display screen in

  let win = xCreateSimpleWindow display root 0 0 width height 2 black_pixel white_pixel in
  xSelectInput display win [ExposureMask; KeyPressMask];

  (* colors allocation *)
  let x_colors = Array.init 8 (fun i -> xAllocNamedColor display cmap color_names.(i)) in
  let x_colors = Array.map fst x_colors in

  xMapWindow display win;

  let expose x_colors =
    let color = ref 0 in
    for x = 0 to pred(width/10) do
    let x = x * 10 in
      for y = 0 to pred(height/10) do
      let y = y * 10 in
        xSetForeground display gc (xColor_pixel x_colors.(!color));
        xFillRectangle display win gc x y 10 10;
        incr color;
        color := if !color > 7 then 0 else !color;
      done
    done
  in

  let ev = new_xEvent() in
  while true do
    xNextEvent display ev;
    match xEventType ev with
    | Expose -> expose x_colors;
    | KeyPress -> exit 0;
    | _ -> ()
  done

