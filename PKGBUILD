# Maintainer: Your Name <you@domain.com>
pkgname=qtpdfannotator
_pkgname=QtPdfAnnotator
pkgver=0.1.0
pkgrel=1
pkgdesc="A simple PDF annotator with an infinite canvas built with C++ and Qt."
arch=('x86_64')
url=""
license=('GPL3')
depends=('qt6-base' 'qt6-pdf')
makedepends=('cmake' 'qt6-tools')
source=("$pkgname-$pkgver.tar.gz"
        "CMakeLists.txt"
        "main.cpp")
sha256sums=('SKIP'
            'SKIP'
            'SKIP')

build() {
  cd "$srcdir"
  cmake -B build -S . \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release
  cmake --build build
}

package() {
  cd "$srcdir"
  DESTDIR="$pkgdir" cmake --install build
}
