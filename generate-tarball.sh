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
rev=$(echo $((`git describe --tags | cut -d - -f 2`)))
pkgrev=$(date +%Y%m%d)git${rev}
psiver=0.16-${pkgrev}
popd

# Translations
rm -fr psi-plus-l10n
git clone --depth 1 git://github.com/psi-plus/psi-plus-l10n.git
pushd psi-plus-l10n
tar --exclude='.*' -cjf ../psi-plus-l10n.tar.bz2 translations
popd
rm -fr psi-plus-l10n

# Prepare psi-plus folder
rm -fr psi-plus-${psiver}
mkdir psi-plus-${psiver}
cp -r psi/* psi-plus-${psiver}
rm -fr psi

# Copy plugins sources to psi dir
cp -r plugins/* psi-plus-${psiver}/src/plugins
rm -fr plugins

# Apply patches
cat main/patches/*.diff | patch -s --no-backup-if-mismatch -d psi-plus-${psiver} -p1
cp -r main/iconsets/* psi-plus-${psiver}/iconsets

rm -fr main

echo "0.16.${rev}-webkit (@@DATE@@)" > psi-plus-${psiver}/version
tar --exclude='.*' -cjf psi-plus-${psiver}.tar.bz2 psi-plus-${psiver}
rm -fr psi-plus-${psiver}
