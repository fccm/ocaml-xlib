if [ ! -f test_img.ppm ]; then convert logo: test_img.ppm; fi
(make -C ../src Xlib.cma)
ocaml -I ../src Xlib.cma xppm.ml test_img.ppm
