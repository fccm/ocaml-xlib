(make -C ../src Xlib.cma)
ocaml unix.cma -I ../src Xlib.cma simple_text.ml
