## Building on linux

The easiest way to build on linux uses Docker to install Qt and required libraries.
Run `./dist_linux_docker.sh` to build a Docker image from the `Dockerfile` included in the repo and then use this Docker image to actually build DKV2. This will result in a `DKV2-[version].AppImage` in the `build-dist-linux` folder
