## Building on linux

### Ubuntu 24.04

The easiest way to build on Ubuntu Linux 24.04 uses Docker to install Qt and required libraries.

- Run `./dist_linux_docker.sh`
  - This builds a Docker image from the `Dockerfile` included in the repo
  - Then use this Docker image is used to actually build DKV2
  - This will result in a `DKV2-[version].AppImage` in the `build-dist-linux` folder.
- Copy the AppImage file to wherever you like.
- Start it with for example `./build-dist-linux/DKV2-[version].AppImage` (replace the version).

### Debian 12 (Bookworm)

- Install these packages:
  ```
  qt6-base-dev
  cmake
  g++
  ```
- Create build dir and go there: `mkdir build; cd build`
- Configure, using cmake: `cmake ..`
- Build: `make -j`
- The resulting executable is in `src/DKV2/`.  Run it like so:
  ```sh
  ./src/DKV2/DKV2
  ```
- For creating DK-Briefe, copy these files into the `vorlagen` directory in your output directory
  (often in `DKV2` in `Documents`):
  - `brieflogo.png`
  - `zinsbrief.css`
  - `zinsbrief.html`
  - `zinsliste.html`
