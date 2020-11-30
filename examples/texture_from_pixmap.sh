(make -C ../src Xlib.cma keysym_match.cma GLX.cma GLX_P2T.cma)
opam install glMLite
GL_DIR=`ocamlfind query glMLite`
echo ""
echo "  Press 'a' to run the animation"
echo ""
ocaml str.cma -I ../src Xlib.cma keysym_match.cma GLX.cma GLX_P2T.cma \
  -I $GL_DIR GL.cma texture_from_pixmap.ml $*
