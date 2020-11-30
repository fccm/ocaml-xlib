(make -C ../src Xlib.cma GLX.cma)

opam install glMLite

GL_DIR=`ocamlfind query glMLite`

ocaml -I ../src Xlib.cma GLX.cma -I $GL_DIR GL.cma glxdemo.ml $*
