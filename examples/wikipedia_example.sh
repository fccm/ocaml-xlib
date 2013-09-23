(make Xlib.cma keysym.cma)
ocaml -I . Xlib.cma keysym.cma wikipedia_example.ml $*
