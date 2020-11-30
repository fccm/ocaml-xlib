(make -C ../src Xlib.cma GLX.cma)

opam install glMLite
GL_DIR=`ocamlfind query glMLite`

ocaml \
  -I $GL_DIR GL.cma Glu.cma \
  -I ../src Xlib.cma GLX.cma \
  pixmap_to_gl.ml

#ocamlopt -g \
#  -I +glMLite GL.cmxa Glu.cmxa \
#  -I ../src Xlib.cmxa GLX.cmxa \
#  pixmap_to_gl.ml -o pixmap_to_gl.opt

