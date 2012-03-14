#! /bin/sh

# Get psi sources
rm -fr psi
git clone --depth 1 git://github.com/psi-im/psi.git
pushd psi
git submodule init
git submodule update
popd

# Get psi-dev repositoris
rm -fr main plugins
git clone git://github.com/psi-plus/main.git
git clone --depth 1 git://github.com/psi-plus/plugins.git

# Prepare 
pushd main
rev=$(echo $((`git describe --tags | cut -d - -f 2`+5000)))
pkgrev=$(date +%Y%m%d)git${rev}
psiver=0.15-${pkgrev}
popd

# Russian language
rm -f language_ru.tar.bz2 psi-plus-ru
git clone --depth 1 git://github.com/ivan101/psi-plus-ru.git
tar -C psi-plus-ru/ -cjf language_ru.tar.bz2 psi_ru.ts qt/qt_ru.ts 

# Prepare psi-plus folder
rm -fr psi-plus-${psiver}
mkdir psi-plus-${psiver}
cp -r psi/* psi-plus-${psiver}

# Copy plugins sources to psi dir
cp -r plugins/* psi-plus-${psiver}/src/plugins

# Apply patches 
cat main/patches/*.diff | patch -d psi-plus-${psiver} -p1
cp -r main/iconsets/* psi-plus-${psiver}/iconsets

sed "s/\(.xxx\)/.${rev}/" -i "psi-plus-${psiver}/src/applicationinfo.cpp"
tar --exclude='.*' -cjf psi-plus-${psiver}.tar.bz2 psi-plus-${psiver}


