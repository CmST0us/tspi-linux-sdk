#!/bin/bash

CMD=`realpath $0`
CMD_DIR=`dirname $CMD`
BUILDROOT_DIR=`pwd`
SDK_DIR=`dirname $BUILDROOT_DIR`
RV_ROOT_DIR=$TARGET_OUTPUT_DIR/root
RV_IMAGE_DIR=$TARGET_OUTPUT_DIR/image

TOOLCHAINS_ARM_LINUX_READELF=$HOST_DIR/bin/arm-linux-readelf

NECESSARY_APP=`echo $RK_FIRST_START_APP | cut -d '"' -f2 | cut -d '"' -f1`

function list_usr_target()
{
	echo "#######################################"
	echo "#          list_usr_target            #"
	echo "#######################################"
	# bin full path
	temp_target_usr_bin_array=(
		`ls $TARGET_DIR/usr/bin/* | sort`
	)
	for element in ${temp_target_usr_bin_array[@]}
	do
		if [ ! -L $element ]; then
			target_usr_bin_array=(${target_usr_bin_array[@]} $element);
		fi
	done
	target_usr_bin_array_len=${#target_usr_bin_array[@]}
	echo target_usr_bin_array_len=$target_usr_bin_array_len

	# sbin full path
	temp_target_usr_sbin_array=(
		`ls $TARGET_DIR/usr/sbin/* | sort`
	)
	for element in ${temp_target_usr_sbin_array[@]}
	do
		if [ ! -L $element ]; then
			target_usr_sbin_array=(${target_usr_sbin_array[@]} $element);
		fi
	done
	target_usr_sbin_array_len=${#target_usr_sbin_array[@]}
	echo target_usr_sbin_array_len=$target_usr_sbin_array_len

	# lib full path
	temp_target_usr_lib_array=(
		`ls $TARGET_DIR/usr/lib/ | grep "so" | sort`
	)
	for element in ${temp_target_usr_lib_array[@]}
	do
		if [ ! -d $TARGET_DIR/usr/lib/$element ]; then
			path=`find $TARGET_DIR/usr/lib -name $element | sort`
			if [ ! -z $path ]; then
				target_usr_lib_array=(${target_usr_lib_array[@]} $path);
			fi
		fi
	done
	target_usr_lib_array_len=${#target_usr_lib_array[@]}
	echo target_usr_lib_array_len=$target_usr_lib_array_len
}


function collect_necessary_target()
{
	echo "#######################################"
	echo "#        choose_necessary_target      #"
	echo "#######################################"

	# app
	if [ $# == 0 ]; then return; fi
	app_targets=$@
	for target in ${app_targets}; do
		echo "Processing app: $target"
		if [ ! -z $target ]; then
			path=`find $TARGET_DIR/usr -name $target | sort`
			if [ -z $path ]; then
				path=`find $TARGET_DIR/sbin -name $target | sort`
			fi

			if [ ! -z $path ]; then
				necessary_bin_array=(${necessary_bin_array[@]} $path)
				necessary_bins_string+=" "$path
			fi
		fi
	done

	if [ 0 == ${#necessary_bin_array[@]} ]; then
		echo  necessary_bin_array is null
		return
	else
		necessary_bin_array_len=${#necessary_bin_array[@]}
		echo necessary_bin_array_len=$necessary_bin_array_len
	fi

	# lib
	temp_necessary_lib_array=(
		`$TOOLCHAINS_ARM_LINUX_READELF -d $necessary_bins_string | \
		grep NEEDED | cut -d '[' -f2 | cut -d ']' -f1 | cut -d '.' -f1 | cut -d '-' -f1 | sort -u`
	)
	temp_necessary_lib_array_len=${#temp_necessary_lib_array[@]}
	if [ 0 == $temp_necessary_lib_array_len ]; then
		return
	fi

	i=0
	for element in ${temp_necessary_lib_array[@]}
	do
		path=(`find $TARGET_DIR/usr/lib -name ${element}*so* | sort -u`)
		if [ 0 == ${#path[@]} ]; then continue; fi
		for path_element in ${path[@]}
		do
			necessary_lib_array[$i]=$path_element
			i=`expr $i + 1`
		done
	done

	necessary_lib_array_len=${#necessary_lib_array[@]}
	echo first necessary_lib_array_len=$necessary_lib_array_len

	#lib depends on lib
	for element in ${necessary_lib_array[@]}
	do
		if [ ! -z `echo $element | grep .py` ]; then continue; fi
		temp_necessary_lib_depend_array=(
			`$TOOLCHAINS_ARM_LINUX_READELF -d ${element} | \
			grep NEEDED | cut -d '[' -f2 | cut -d ']' -f1 | cut -d '.' -f1 | sort -u`)
		if [ 0 == ${#temp_necessary_lib_depend_array[@]} ]; then continue; fi

		for depend_element in ${temp_necessary_lib_depend_array[@]}
		do
			depend_path=(`find $TARGET_DIR/usr/lib -name ${depend_element}.so* | sort -u`)
			if [ 0 == ${#depend_path[@]} ]; then continue; fi
			for depend_path_element in ${depend_path[@]}
			do
				no_find_new_lib=0
				for path_element in ${necessary_lib_array[@]}
				do
					if [ $depend_path_element == $path_element ]; then
						no_find_new_lib=1
					fi
				done
				if [ $no_find_new_lib == 0 ]; then
					echo depend_path_element=$depend_path_element
					necessary_lib_array[$i]=$depend_path_element
					i=`expr $i + 1`
				else
					no_find_new_lib=0
				fi
			done
		done
	done

	necessary_lib_array_len=${#necessary_lib_array[@]}
	echo second necessary_lib_array_len=$necessary_lib_array_len
}

function collect_unnecessary_target()
{
	echo "#######################################"
	echo "#      collect_unnecessary_target     #"
	echo "#######################################"

	# bin
	is_unnecessary=1
	for temp_element in ${temp_target_usr_bin_array[@]}
	do
		if [ -L $temp_element ]; then
			continue
		fi

		for element in ${necessary_bin_array[@]}
		do
			if [ $temp_element == $element ]; then
				is_unnecessary=0
				break;
			fi
		done
		if [ $is_unnecessary == 1 ]; then
			unnecessary_usr_bin_array=(${unnecessary_usr_bin_array[@]} $temp_element)
		else
			is_unnecessary=1
		fi
	done
	unnecessary_usr_bin_array_len=${#unnecessary_usr_bin_array[@]}
	echo unnecessary_usr_bin_array_len=$unnecessary_usr_bin_array_len

	#sbin
	is_unnecessary=1
	for temp_element in ${temp_target_usr_sbin_array[@]}
	do
		if [ -L $temp_element ]; then
			continue
		fi

		for element in ${necessary_bin_array[@]}
		do
			if [ $temp_element == $element ]; then
				is_unnecessary=0
				break;
			fi
		done
		if [ $is_unnecessary == 1 ]; then
			unnecessary_usr_sbin_array=(${unnecessary_usr_sbin_array[@]} $temp_element)
		else
			is_unnecessary=1
		fi
	done
	unnecessary_usr_sbin_array_len=${#unnecessary_usr_sbin_array[@]}
	echo unnecessary_usr_sbin_array_len=$unnecessary_usr_sbin_array_len

	#lib
	is_unnecessary=1
	for temp_element in ${target_usr_lib_array[@]}
	do
		for element in ${necessary_lib_array[@]}
		do
			if [ $temp_element == $element ]; then
				is_unnecessary=0
				break;
			fi
		done
		if [ $is_unnecessary == 1 ]; then
			unnecessary_usr_lib_array=(${unnecessary_usr_lib_array[@]} $temp_element)
		else
			is_unnecessary=1
		fi
	done
	unnecessary_usr_lib_array_len=${#unnecessary_usr_lib_array[@]}
	echo unnecessary_usr_lib_array_len=$unnecessary_usr_lib_array_len
}

function pack_root()
{
	echo "#######################################"
	echo "#              pack_root              #"
	echo "#######################################"

	### clean root dir
	if [ ! -d $RV_ROOT_DIR ]; then mkdir -p $RV_ROOT_DIR; fi
	if [ ! -d $RV_ROOT_DIR/bin ]; then mkdir -p $RV_ROOT_DIR/bin; fi
	if [ ! -d $RV_ROOT_DIR/sbin ]; then mkdir -p $RV_ROOT_DIR/sbin; fi
	if [ ! -d $RV_ROOT_DIR/lib ]; then mkdir -p $RV_ROOT_DIR/lib; fi

	#### copy target to root & delete target
	for element in ${unnecessary_usr_bin_array[@]}
	do
		cp -fr $element $RV_ROOT_DIR/bin
		rm -fr $element
	done
	for element in ${unnecessary_usr_sbin_array[@]}
	do
		cp -fr $element $RV_ROOT_DIR/sbin
		rm -fr $element
	done
	for element in ${unnecessary_usr_lib_array[@]}
	do
		cp -fr $element $RV_ROOT_DIR/lib
		rm -fr $element
	done
}

list_usr_target
collect_necessary_target $NECESSARY_APP
collect_unnecessary_target
pack_root
