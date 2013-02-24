(make Xlib.cma)
ocaml -I . Xlib.cma double_buffer.ml $*
