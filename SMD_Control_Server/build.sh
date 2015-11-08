#/bin/bash

if [ -f "/etc/arduino/openwrt-yun-release" ]; then
	
	echo "We are building on the Yun!"
	
	#environment variables that fix the uClibc issues
	export jm_cv_func_working_malloc="yes"
	export ac_cv_func_malloc_0_nonnull="yes"
	export ac_cv_func_realloc_works="yes"

	./configure CPPFLAGS="-I/mnt/sda1/smd_server/include/modbus" LDFLAGS="-L/mnt/sda1/smd_server/lib"

fi

