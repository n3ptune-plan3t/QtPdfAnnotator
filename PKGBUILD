# Maintainer: Your Name <you@domain.com>
pkgname=qtpdfannotator
_pkgname=QtPdfAnnotator
pkgver=0.1.0
pkgrel=1
pkgdesc="A simple PDF annotator with an infinite canvas built with C++ and Qt."
arch=('x86_64')
url=""
license=('GPL3')
# Corrected the dependency from qt6-pdf to poppler-qt6
depends=('qt6-base' 'poppler-qt6')
makedepends=('cmake' 'qt6-tools')
# The source is the local files, not a tarball, because we build from a git checkout.
source=("CMakeLists.txt"
        "main.cpp")
sha256sums=('SKIP'
            'SKIP')

build() {
  # The source files listed above are automatically copied into this directory
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

