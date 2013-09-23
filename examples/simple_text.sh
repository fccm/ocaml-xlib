(make -C ../src Xlib.cma)
ocaml unix.cma Xlib.cma simple_text.ml
