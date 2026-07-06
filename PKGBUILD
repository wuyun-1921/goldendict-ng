# Maintainer: wuyun <wuyun1921@gmail.com>
# Based on: goldendict-ng-git by slbtty

pkgname=goldendict-wy-git
pkgver=26.7.0.r50.$(git rev-parse --short=10 HEAD 2>/dev/null || echo "unknown")
pkgrel=1
pkgdesc="GoldenDict fork with multi-panel, always-query, session restore and more (Supports Qt WebEngine & Qt6)"
arch=('x86_64' 'aarch64')
url="https://github.com/wuyun-1921/goldendict-ng"
license=('GPL3')
depends=(
	hunspell
	libvorbis
	libxtst
	lzo
	zlib
	xz
	tomlplusplus
	fmt
	opencc
	xapian-core
	libzim
	qt6-base
	qt6-svg
	qt6-multimedia
	qt6-webengine
	qt6-speech
	qt6-5compat
)
makedepends=(
	git
	cmake
	ninja
	qt6-tools
)
conflicts=('goldendict-ng-git')
provides=('goldendict')
source=("$pkgname::git+https://github.com/wuyun-1921/goldendict-ng.git")
md5sums=('SKIP')

pkgver() {
	cd "$srcdir/$pkgname"
	printf "%s.r%s.%s" "$(git describe --tags --abbrev=0 2>/dev/null | cut -c 2- || echo "26.7.0")" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

prepare() {
	export CXXFLAGS+=" -Wp,-U_GLIBCXX_ASSERTIONS"
}

build() {
	cmake -B build_dir -S "$pkgname" -G Ninja \
		-DCMAKE_INSTALL_PREFIX='/usr' \
		-DUSE_SYSTEM_FMT=ON \
		-DUSE_SYSTEM_TOML=ON \
		-DWITH_FFMPEG_PLAYER=OFF \
		-DWITH_EPWING_SUPPORT=OFF \
		-Wno-dev
	cmake --build build_dir
}

package() {
	DESTDIR="$pkgdir" cmake --install ./build_dir/
}
