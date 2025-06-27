<div style="display: flex; justify-content: center; align-items: center; height: 100vh; width: 100vw; margin: 0;">
  <img src="graphics/Logos-Enigma-3.png" alt="Enigma Logo" style="width: 50%; height: auto; max-width: 100%;">
</div>

# Enigma Simulator

> [!NOTE]  
> This project was developed at RWU – University of Applied Sciences as part of the “Enigma Rebuild Project.”  
> I, **Tobias Steidle**, co-developed this simulator and expanded it for broader use and cross-platform compatibility.

> [!IMPORTANT]  
> The project is based on the original simulator developed by [Arif Hasanic](https://github.com/murderbaer/enigma).  
> It has since been significantly extended within a collaborative academic setting.

---

## Project Background and Lineage

This simulator is derived from the work of [Emanuel Schäffer](https://github.com/UPEV1sion/Enigma/tree/main):  
*“Kryptoanalyse der Enigma-Maschine durch eine Software-Nachbildung der Turing-Welchman-Bombe”* (2025, RWU),  
supervised by Prof. Dipl.-Math. Ekkehard Löhmann.  
The repository presented here is a **branch of that project** and was **co-authored and extended by Tobias Steidle**.  
Both developments are grounded in the original implementation by Arif Hasanic.

This simulator forms the computational core of a modular system involving multiple companion repositories:

- **[Enigma-API](https://github.com/UPEV1sion/Enigma-API)**  
  → Provides a REST interface to the simulator, accessible via Foreign Function & Memory (FFM) in modern Java applications.

- **[EnigmaServer](https://github.com/UPEV1sion/Enigma/tree/server)**  
  → Server-focused fork for backend integration and deployment.

- **[EnigmaSite](https://github.com/Bibble-code/EnigmaSite)**  
  → A web-based frontend for interacting with the Enigma and Cyclometer simulators.

---

## Third-Party Libraries

- **cjson** – MIT License  
  [License](https://github.com/DaveGamble/cJSON/blob/master/LICENSE)
- **GTK-3.0** – GNU General Public License  
  [License](https://github.com/GNOME/gtk/blob/main/COPYING)

---

## Quickstart

### Install Dependencies

You may use `install_deps.sh` or install manually depending on your platform:

```bash
# Arch Linux
sudo pacman -S gtk3
sudo pacman -S pkg-config

# Debian/Ubuntu
sudo apt install libgtk-3-dev
sudo apt install pkg-config

# Windows (MinGW)
pacman -S mingw-w64-x86_64-gtk3
(optional) pacman -S mingw-w64-ucrt-x86_64-toolchain base-devel
pacman -S mingw-w64-x86_64-pkg-config

# macOS (Homebrew)
brew install gtk+3
brew install pkg-config
```

If you're on macOS and encounter a linker error (e.g. with `cjson`), add Homebrew paths to your shell config:

```bash
export LDFLAGS="-L/opt/homebrew/lib"
export CPPFLAGS="-I/opt/homebrew/include"
```

> ℹ️ Note: Homebrew paths are already checked in `CMakeLists.txt`, so this is rarely needed.

---

### Build the Project

This project uses CMake as its build system:

```bash
mkdir build
cd build
cmake ../enigma_c
cmake --build .
```

> [!NOTE]  
> The project avoids C23 to maintain cross-platform compatibility.  
> While macOS support is under development, it should build on most systems with GTK and pkg-config installed.

After building, the `build/` directory will contain:

- `enigma`: command-line simulator binary  
- `enigma_test`: testing binary  
- `libenigma.so`: shared library required for FFM integration in companion applications

You can check usage instructions by running:

```bash
./build/bin/enigma -h
```

---


## Used In Production

This library powers the simulation logic behind the live site:  
🔗 https://enigma-zyklometer.rwu.de

---
## License

This project builds on open source software and is distributed under their respective licenses.  
Contributions should retain original attributions and follow licensing guidelines.
