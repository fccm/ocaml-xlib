(make -C ../src Xt.cma)
ocaml -I ../src Xt.cma intrinsic.ml $*
