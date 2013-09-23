(make -C ../src Xlib.cma keysym.cma)
ocaml -I ../src Xlib.cma keysym.cma wikipedia_example.ml $*
