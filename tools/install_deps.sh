install_arch() {
	echo "Arch Linux detected. Installing dependencies..."
	sudo pacman -Sy --noconfirm gtk3 pkg-config
}

install_debian() {
	echo "Debian-based Linux detected. Installing dependencies..."
	sudo apt update
	sudo apt install -y libgtk-3-dev pkg-config
}

install_msys2() {
	echo "MSYS2 environment detected. Installing dependencies..."
	pacman -Sy --noconfirm mingw-w64-x86_64-gtk3 mingw-w64-x86_64-pkg-config
	pacman -S --noconfirm mingw-w64-ucrt-x86_64-toolchain base-devel
}

install_macos() {
	echo "macOS detected. Installing dependencies..."
	if ! command -v brew &> /dev/null; then
        echo "Homebrew not found, installing Homebrew..."
        /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    fi
    brew update
    brew install gtk+3 pkg-config
}

detect_os() {
    if [[ -f /etc/arch-release ]]; then
        install_arch
    elif [[ -f /etc/debian_version ]]; then
        install_debian
    elif [[ "$(uname -o)" == "Msys" ]]; then
        install_msys2
    elif [[ "$(uname)" == "Darwin" ]]; then
        install_macos
    else
        echo "Unsupported operating system."
        exit 1
    fi
}

detect_os