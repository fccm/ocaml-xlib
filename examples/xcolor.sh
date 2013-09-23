(make -C ../src Xlib.cma)
ocaml -I ../src Xlib.cma xcolor.ml $*
