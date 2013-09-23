open Xt

let quit_app win app ~data ~value =
  xtUnmapWidget win;
  xtDestroyApplicationContext app;
  exit 0;
;;

let some_app w ~data ~value =
  Printf.printf "CB Handled\n%!";
;;

let () =
  let app_resources = [|
    "*some_command.Label: Button";
    "*quit_command.Label: Quit";
  |] in
  let app, window = xtOpenApplication "window" SessionShellWidgetClass app_resources in

  let box = xtCreateManagedWidget "box" BoxWidgetClass window in

  let some_command = xtCreateManagedWidget "some_command" CommandWidgetClass box
  and quit_command = xtCreateManagedWidget "quit_command" CommandWidgetClass box
  in

  xtAddCallback quit_command XtNcallback (quit_app window app) ();
  xtAddCallback some_command XtNcallback (some_app window) ();

  xtRealizeWidget window;
  xtAppMainLoop app;
;;

