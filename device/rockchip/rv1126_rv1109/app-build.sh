#!/bin/bash

function dirclean()
{
	make  \
		dbserver-dirclean \
		common_algorithm-dirclean \
		ipcweb-backend-dirclean \
		libgdbus-dirclean \
		libIPCProtocol-dirclean \
		librkdb-dirclean \
		mediaserver-dirclean \
		camera_engine_rkaiq-dirclean \
		netserver-dirclean \
		storage_manager-dirclean \
		rkmedia-dirclean \
		rk_oem-dirclean \
		mpp-dirclean \
		ipc-daemon-dirclean \
		rockface-dirclean \
		CallFunIpc-dirclean \
		isp2-ipc-dirclean \
	###
}

function sync_mod()
{
	.repo/repo/repo sync -c --no-tags \
		app/dbserver \
		app/ipcweb-backend \
		app/libgdbus \
		app/libIPCProtocol \
		app/librkdb \
		app/mediaserver \
		app/netserver \
		app/ipc-daemon \
		app/storage_manager \
		external/camera_engine_rkaiq \
		external/rkmedia \
		external/common_algorithm \
		external/rockface \
		external/mpp \
		external/CallFunIpc \
		external/isp2-ipc \
###
}

function rebuild()
{
	make  \
		dbserver-rebuild \
		common_algorithm-rebuild \
		libgdbus-rebuild \
		libIPCProtocol-rebuild \
		librkdb-rebuild \
		CallFunIpc-rebuild \
		camera_engine_rkaiq-rebuild \
		isp2-ipc-rebuild \
		ipcweb-backend-rebuild \
		netserver-rebuild \
		storage_manager-rebuild \
		rk_oem-rebuild \
		mpp-rebuild \
		ipc-daemon-rebuild \
		rockface-rebuild \
		rkmedia-rebuild \
		mediaserver-rebuild \
###
}

unset NEW_OPTS
if [ "${RK_CFG_BUILDROOT}x" != "x" ];then
export TARGET_OUTPUT_DIR="$TOP_DIR/buildroot/output/$RK_CFG_BUILDROOT"
else
if [ "${RK_CFG_RAMBOOT}x" != "x" ];then
export TARGET_OUTPUT_DIR="$TOP_DIR/buildroot/output/$RK_CFG_RAMBOOT"
fi
fi
for option in ${OPTIONS}; do
        echo "processing board option: $option"
        case $option in
                # handle board commands
                app-clean)
						dirclean
						exit 0
                        ;;
                app-rebuild)
						rebuild
						exit 0
                        ;;
                app-sync)
						sync_mod
						exit 0
                        ;;
                *)
                        NEW_OPTS="$NEW_OPTS $option"
                        ;;
        esac
done
export OPTIONS=$NEW_OPTS
