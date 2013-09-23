(make Xlib.cma keysym_match.cma GLX.cma GLX_P2T.cma)
GL_DIR="+glMLite"
if [ ! -f `ocamlc -where`/glMLite/GL.cma ]
then
      sh ./install_glmlite_in_tmp.sh;
      GL_DIR="/tmp/glMLite"
fi
echo " gl is $GL_DIR"
echo ""
echo "  Press 'a' to run the animation"
echo ""
ocaml str.cma -I . Xlib.cma keysym_match.cma GLX.cma GLX_P2T.cma -I $GL_DIR GL.cma texture_from_pixmap.ml $*
