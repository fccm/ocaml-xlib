(make -C ../src Xlib.cma GLX.cma)

GL_DIR="+glMLite"
if [ ! -f `ocamlc -where`/glMLite/GL.cma ]
then
      sh ./install_glmlite_in_tmp.sh;
      GL_DIR="/tmp/glMLite"
fi

ocaml \
  -I $GL_DIR GL.cma Glu.cma \
  -I ../src Xlib.cma GLX.cma \
  pixmap_to_gl.ml

#ocamlopt -g \
#  -I +glMLite GL.cmxa Glu.cmxa \
#  -I ../src Xlib.cmxa GLX.cmxa \
#  pixmap_to_gl.ml -o pixmap_to_gl.opt

