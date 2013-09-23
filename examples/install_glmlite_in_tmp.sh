BASENAME=glMLite-0.03.51
pushd /tmp
wget -nc http://www.linux-nantes.org/%7Efmonnier/OCaml/GL/download/${BASENAME}.tgz
tar xzf ${BASENAME}.tgz
pushd ${BASENAME}/
make
mkdir /tmp/glMLite
make install PREFIX=/tmp/glMLite
popd
popd
