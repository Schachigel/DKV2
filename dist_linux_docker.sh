#!/bin/bash
docker build -t dkv2-build .
docker run \
	--name dkv2-build \
	--privileged \
	--cap-add SYS_ADMIN \
	--device /dev/fuse \
	-v "$(pwd)":/app \
	-e LOCAL_USER=$(id -u $USER) \
	-e LOCAL_GROUP=$(id -g $USER) \
	--rm -ti \
	dkv2-build
