(make Xlib.cma)
if [ ! -f test_img.ppm ]; then convert logo: test_img.ppm; fi
ocaml -I . Xlib.cma xppm.ml test_img.ppm
