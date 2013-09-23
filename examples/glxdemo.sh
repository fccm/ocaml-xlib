(make -C ../src Xlib.cma GLX.cma)
GL_DIR="+glMLite"
if [ ! -f `ocamlc -where`/glMLite/GL.cma ]
then
      sh ./install_glmlite_in_tmp.sh;
      GL_DIR="/tmp/glMLite"
fi
ocaml -I ../src Xlib.cma GLX.cma -I $GL_DIR GL.cma glxdemo.ml $*
