BASENAME=glMLite-0.03.53
pushd /tmp
wget -nc https://github.com/fccm/glMLite/archive/v0.03.53.tar.gz -O - > ${BASENAME}.tgz
tar xzf ${BASENAME}.tgz
pushd ${BASENAME}/
make
mkdir /tmp/glMLite
make install PREFIX=/tmp/glMLite
popd
popd
