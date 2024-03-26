mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

build-python39:
	docker run -it --mount type=bind,source=$(mkfile_dir)/../../../,target=/opt/fmi/ --rm fmi-build-python39:latest bash -c "rm -rf /opt/fmi/python/build/* && mkdir -p /opt/fmi/python/build/ && cd /opt/fmi/python/build/ && cmake .. && make && sh /opt/fmi/python/get_so.sh"
	mkdir -p $(ARTIFACTS_DIR)/python/libfmi/extern/TCPunch/client/
	cp $(mkfile_dir)/../../build/libfmi/extern/TCPunch/client/libtcpunch.a "$(ARTIFACTS_DIR)/python/libfmi/extern/TCPunch/client/"
	cp $(mkfile_dir)/../../build/fmi.so "$(ARTIFACTS_DIR)/python/"
	cp $(mkfile_dir)/../../build/libfmi/libFMI.a "$(ARTIFACTS_DIR)/python/libfmi/"
	
	mkdir -p $(ARTIFACTS_DIR)/lib
	mv $(mkfile_dir)/../../so_files/* "$(ARTIFACTS_DIR)/lib/"
	rm -rf $(mkfile_dir)/../../so_files
