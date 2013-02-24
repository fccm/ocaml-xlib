(* Display a .ppm file
 *
 * Author: Pierre Ficheux (pierre@alienor.fr)  Novembre 1999
 * got from: http://pficheux.free.fr/articles/lmf/xlib-images/
 * Converted from C to OCaml by Florent Monnier
 *)
open Xlib

type color_cache = {
  red : int;
  green : int;
  blue : int;
  pixel : pixel_color;
}

let color_cache = Array.init 256 (fun _ ->
    { red = 0;
      green = 0;
      blue = 0;
      pixel = Obj.magic 0;
    }
  )

let image_width  = 100
let image_height = 100

(* utility: shifts on references *)
let ( >>= ) a b =
  a := !a lsr b

let ( <<= ) a b =
  a := !a lsl b


(* main *)
let () =
  if (Array.length Sys.argv) = 1 then begin
    Printf.eprintf "Usage: xppm file.ppm\n%!";
    exit 1;
  end;

  let display = xOpenDisplay "" in

  let screen = xDefaultScreen display in
  let gc = xDefaultGC display screen in
  let root = xRootWindow display screen in
  let white_pixel = xWhitePixel display screen
  and black_pixel = xBlackPixel display screen in
  let default_cmap = xDefaultColormap display screen in
  let nplanes = xDisplayPlanes display screen in
  let the_visual = xDefaultVisual display screen in

  let red_bits    = ref 0  and red_shift   = ref 0
  and green_bits  = ref 0  and green_shift = ref 0
  and blue_bits   = ref 0  and blue_shift  = ref 0 in

  let red_mask   = ref 0
  and green_mask = ref 0
  and blue_mask  = ref 0 in

  let current_cmap =
    if nplanes > 8 then begin
      let current_cmap = default_cmap in

      (* calculate shifts *)
      red_mask   := visual_red_mask the_visual;
      green_mask := visual_green_mask the_visual;
      blue_mask  := visual_blue_mask the_visual;

      let r = ref !red_mask
      and g = ref !green_mask
      and b = ref !blue_mask in

      while not((!r land 1) = 1) do
        r >>= 1;
        incr red_shift;
      done;

      while (!r land 1) = 1 do
        r >>= 1;
        incr red_bits;
      done;

      while not((!g land 1) = 1) do
        g >>= 1;
        incr green_shift;
      done;

      while (!g land 1) = 1 do
        g >>= 1;
        incr green_bits;
      done;

      while not((!b land 1) = 1) do
        b >>= 1;
        incr blue_shift;
      done;

      while (!b land 1) = 1 do
        b >>= 1;
        incr blue_bits;
      done;

      (current_cmap)
    end
    else
      let current_cmap = xCopyColormapAndFree display default_cmap in
      (current_cmap)
  in

  let fp = open_in Sys.argv.(1) in

  (* Read the header *)
  (* CAUTION, we don't handle the comments! *)
  let magic = input_line fp in

  if magic <> "P6" then begin
    Printf.eprintf "%s: %s is not a PPM file.\n%!" Sys.argv.(0) Sys.argv.(1);
    close_in fp;
    exit 1;
  end;

  let image_width, image_height =
    Scanf.sscanf (input_line fp) "%d %d" (fun a b -> a, b)
  in
  let nb_colors =
    Scanf.sscanf (input_line fp) "%d" (fun v -> v)
  in
  ignore(nb_colors); (* unused *)

  (* data allocation *)
  let image_data = String.create ((Sys.word_size / 8) * image_width * image_height) in

  (* creation of the image *)
  let ximage = xCreateImage display (xDefaultVisual display screen) nplanes ZPixmap 0
                            image_data image_width image_height Sys.word_size 0 in

  (* dunno how this piece should be translated, I mean the __i386__ in the ifdef *)
(*
#ifdef __i386__
    ximage->byte_order = LSBFirst ;
#else
    ximage->byte_order = MSBFirst ;
#endif
*)

  let pixel = ref(Obj.magic 0 : pixel_color) in
  let last_color = ref 0 in

  for y = 0 to pred image_height do
    for x = 0 to pred image_width do

      let red   = ref(input_byte fp) in
      let green = ref(input_byte fp) in
      let blue  = ref(input_byte fp) in

      if (nplanes > 8) then begin
        red   >>= (8 - !red_bits);
        green >>= (8 - !green_bits);
        blue  >>= (8 - !blue_bits);

        pixel := Obj.magic
                   ( ((!red   lsl !red_shift  ) land !red_mask  ) lor
                     ((!green lsl !green_shift) land !green_mask) lor
                     ((!blue  lsl !blue_shift ) land !blue_mask ) );
      end
      (* 8 planes *)
      else begin
        let color = new_xColor() in

        red <<= 8;
        green <<= 8;
        blue <<= 8;

        let j = ref 0 in
        begin try
          for i = 0 to pred !last_color do
            j := i;
            if (color_cache.(i).red   = !red &&
                color_cache.(i).green = !green &&
                color_cache.(i).blue  = !blue) then
              raise Exit
          done
        with Exit -> ()
        end;

        if (!j = !last_color) then begin
          xColor_set_red   color !red;
          xColor_set_green color !green;
          xColor_set_blue  color !blue;

          xAllocColor display current_cmap color;
          color_cache.(!j) <- {
              red   = !red;
              green = !green;
              blue  = !blue;
              pixel = xColor_pixel color;
            };
          pixel := xColor_pixel color;

          incr last_color;
          if (!last_color > 256) then begin
            Printf.eprintf "Too many colors...\n%!";
            exit 1;
          end;
        end
        else
          pixel := color_cache.(!j).pixel;
      end;

      xPutPixel ximage x y !pixel;
    done;
  done;

  close_in fp;

  let attr = new_xSetWindowAttributes() in

  winAttr_set_background_pixel attr white_pixel;
  winAttr_set_border_pixel attr black_pixel;
  winAttr_set_colormap attr current_cmap;

  let win = xCreateWindow display root 0 0 image_width image_height
                          2 nplanes CopyFromParent the_visual
                          [CWBackPixel; CWBorderPixel; CWColormap] attr in

  xSelectInput display win [ExposureMask; KeyPressMask; ButtonPressMask];

  xMapWindow display win;

  (* associated fonction for the Expose evenement *)
  let expose () =
    xPutImage display win gc ximage 0 0 0 0 image_width image_height;
  in

  let ev = new_xEvent() in
  while true do
    xNextEvent display ev;

    match xEventType ev with
    | Expose ->
        expose ();

    | KeyPress
    | ButtonPress ->
        exit 0;

    | _ -> ()
  done;
;;

