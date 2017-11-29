============= SUMMARY %

OCaml bindings for the Xlib library.

============= ATTRIBUTION %

Copyright (C) 2008, 2009, 2010 Florent Monnier
Contact: <fmonnier@linux-nantes.roll> (replace .roll by .org)

============= LICENSE %

This library is released under the terms of the MIT license,
described in file COPYING.txt

============= ALTERNATIVE LICENSES %

If it is more convenient for you, you can also use this code
under the terms of any of these licenses:
- CC0, any kind of BSD license, WTFPL, ISC or zlib/libpng

============= HOMEPAGE %

http://www.linux-nantes.org/%7Efmonnier/OCaml/Xlib/

============= DESCRIPTION %

OCaml-Xlib is a work in progress Xlib bindings for OCaml.

It also includes a GLX module for OpenGL windowing,
and the very early beginning of Xt/Intrinsics.

There are 3 OCaml-OpenGL bindings: LablGL, GLCaml and glMLite.
The GLX module does not have any particular dependency across any
of these OpenGL bindings, so it can be used with either of them.
For exchanging pixmaps as textures GLCaml and glMLite use either
strings or bigarrays, for LablGL interoperability convert the
strings or the bigarrays with its module Raw.

Though the examples use glMLite.

============= INSTALL %

To install `make install` will only install the Xlib module.
`make install_all` will install all the Xlib, GLX and Xt modules.
If you only need X and GLX, use `make install_x install_glx`

The default prefix is `ocamlc -where`/Xlib
you can change the destination install dir with:
`make install PREFIX=/some/path`

============= DOCUMENTATION %

Get the HTML ocamldoc generated documentation with the command:
`make doc`

============= EXAMPLES %

For examples of use, run the *.sh scripts.

============= EXTENSIONS %

There is a module for the GLX extension texture_from_pixmap:
 http://www.opengl.org/registry/specs/EXT/texture_from_pixmap.txt

To build and install, run:
`make glx_p2t`
`make install_glx_p2t`

============= TODO %

XKB
On my computer the demo texture_from_pixmap.ml runs right in
interpreted mode, but segfaults in native mode.

============= FRIEND PROJECTS %

There are 2 other projects related to X programming in OCaml.
___

Fabrice Le Fessant has released an xlib library too. And it
is not a binding/wrapper of the C Xlib, it is an xlib written
natively in OCaml.
This package is the base for the projects 'EFuns' and 'GwML':
  http://caml.inria.fr/cgi-bin/hump.fr.cgi?contrib=183
  http://caml.inria.fr/cgi-bin/hump.fr.cgi?contrib=225

The license of this project is not clearly written in the archive,
I have written to its author to ask him, and it can be used under
the GPL license.

The default prefix for this pure ocaml xlib is `ocamlc -where`/xlib
(notice "xlib" != "Xlib")
_____

There is also the package "Olibrt" written by Daniel de Rauglaudre,
which contains basic bindings to the C Xlib, find it here:
  http://pauillac.inria.fr/%7Eddr/olibrt/
It is released under the BSD license.

============= CONTRIBUTORS %

Arlen Cuss, 2011-01-28: fix for the function xFetchName.

Alex Muscar <muscar(_)gmail.com>, 2012-01-01:
- provided a patch to add the function ml_XCreateWindowEvent_datas()
  and the type xCreateWindowEvent_contents

============= EOF %
