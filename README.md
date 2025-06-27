<div style="display: flex; justify-content: center; align-items: center; height: 100vh; width: 100vw; margin: 0;">
  <img src="graphics/Logos-Enigma-3.png" alt="Enigma Logo" style="width: 50%; height: auto; max-width: 100%;">
</div>

# Enigma Simulator

> [!NOTE]
> This project was developed at a university as part of the “Enigma Rebuild Project.”
> I, **Tobias Steidle**, continued and expanded the project with a focus on integrating it in a Website.

> [!IMPORTANT]
> The foundation of this simulator was laid by [Arif Hasanic](https://github.com/murderbaer/enigma), whose original implementation served as the basis for this work.
> Special thanks to **Paul Köster** for designing the logo above.

> [!TIP]
> This simulator is part of a **modular system** and is integrated into the following related repositories:
>
> - [Enigma-API](https://github.com/UPEV1sion/Enigma-API):
>   Provides a REST interface for programmatic access to the simulator.
>
> - [EnigmaServer](https://github.com/UPEV1sion/Enigma/tree/server):
>   A server-based extension of Emanuel Schäffer’s Enigma implementation.
>
> - [EnigmaSite](https://github.com/Bibble-code/EnigmaSite):
>   A web frontend for Enigma and Cyclometer simulations using the API backend.

---

## Third-Party Libraries

- **cjson**: MIT License
  - [License](https://github.com/DaveGamble/cJSON/blob/master/LICENSE)
- **GTK-3.0**: GNU General Public License (GPL)
  - [License](https://github.com/GNOME/gtk/blob/main/COPYING)

---

## Quickstart

### Install Dependencies

You can run `install_deps.sh` or follow the manual instructions below, depending on your platform:

```bash
# Arch Linux
sudo pacman -S gtk3
sudo pacman -S pkg-config

# Debian/Ubuntu-based
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

If you're on macOS and encounter a linker error (e.g. with `cjson`), try adding the Homebrew library paths in your `.zshrc` or `.bashrc`:

```bash
export LDFLAGS="-L/opt/homebrew/lib"
export CPPFLAGS="-I/opt/homebrew/include"
```

> ℹ️ Note: The `CMakeLists.txt` file already contains logic to detect Homebrew paths, so this step is rarely necessary.

---

### Build the Project

This project uses [CMake](https://cmake.org/) as its build system:

```bash
mkdir build
cd build
cmake ../enigma_c
cmake --build .
```

> [!NOTE]
> The goal was full cross-platform support. The project deliberately avoids the C23 standard due to limited compiler support as of now.
> While macOS support is still under development (due to lack of local testing hardware), the project is being actively improved to support it.

After building, the following output files will be generated in the `build/` directory:

- `libenigma.so`: A shared library required by other FFM-based programs that use the native simulation logic


---

## License

This project builds on prior open source work and is released under its respective licenses (see above). Modifications and contributions should follow the licensing terms and maintain attributions to original authors.

