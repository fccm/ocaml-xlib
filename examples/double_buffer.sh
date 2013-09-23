(make -C ../src Xlib.cma)
ocaml -I ../src Xlib.cma double_buffer.ml $*
