#!/bin/bash
set -euxo pipefail

# ROS_DISTRO will be overrided when docker image build
ROS_DISTRO=

export LD_LIBRARY_PATH=/buildroot/host/lib/
export SYSROOT=/buildroot/host/aarch64-buildroot-linux-gnu/sysroot/
export CC=/buildroot/host/bin/aarch64-buildroot-linux-gnu-gcc
export CXX=/buildroot/host/bin/aarch64-buildroot-linux-gnu-g++
export CROSS_COMPILE=/buildroot/host/bin/aarch64-buildroot-linux-gnu-
export TARGET_ARCH=aarch64
# Consist with buildroot target, not flag:m.
# flag:m(cpython-38m...) is enabled by --with-pymalloc
export PYTHON_SOABI=cpython-38-aarch64-linux-gnu
export ROS2_INSTALL_PATH=/buildroot/target/opt/ros/
export TARGET_TRIPLE=/buildroot/host/bin/aarch64-buildroot-linux-gnu
export ARCH=aarch64

cleanup() {
  git -C src/ros2/mimick_vendor/ checkout .

  if [ -e src/ros2/pybind11_vendor/CMakeLists.txt ]; then
     git -C src/ros2/pybind11_vendor/ checkout .
  fi

  if [ -e src/ros2/python_cmake_module/ ]; then
     git -C src/ros2/python_cmake_module/ checkout .
  fi

  echo "Strip Elf Object files"

  # *.a : static library that must including symbols
  KNOWN_NO_BINARY="xml|py|dsv|sh|ps1|bash|zsh|cmake|msg|idl|in|action|srv|yaml|gz|em|cpp|h|hpp|md|cc|c|bat|a|txt|pyc"
  find ${ROS2_INSTALL_PATH} -type f -and ! -empty \
    -and -regextype posix-extended ! -regex ".*\.(${KNOWN_NO_BINARY})" \
    -exec grep -IL '' "{}" \; | xargs ${CROSS_COMPILE}strip

  echo "build ros quit & cleanup source changes"
}

trap 'cleanup' EXIT

prepare_source() {
  #FIXME: camke set wrong mimick_vendor ARCH when building, in file:
  #       CMakeFiles/3.16.3/CMakeSystem.cmake. This probably because
  #       qemu can run binary for aarch target directly inside Docker image, which broken cmake configuration.
  # Workaround by set SYSTEM_NAME and SYSTEM_PROCESSER manually
  sed -i '/^.*ExternalProject.*$/i \ \ list(APPEND cmake_configure_args "-DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}") \n\ \ list(APPEND cmake_configure_args "-DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR}")' \
    src/ros2/mimick_vendor/CMakeLists.txt

  # When building galactic's rclpy package:
  #   ./ros/share/cmake/pybind11/pybind11Config.cmake:
  #     -> ./ros/share/cmake/pybind11/pybind11Tools.cmake:   find_package(PythonLibsNew
  #       -> ./ros/share/cmake/pybind11/FindPythonLibsNew.cmake
  # it read host's PYTHON_INCLUDE_DIRS, for example:
  #  $ grep isystem /opt/ros/
  #    .... -isystem /data/ros/install/include ...
  # A hack as below can workaround and continue building:
  #  - colcon build --packages-up-to pybind11, and then,
  #  - Change PythonLibsNew to PythonLibs before building other packages
  # but it can not set PYTHON_MODULE_EXTENSION correctly.
  #
  # To cross compile for aarch64 target, update to new version of pybind11
  # see: https://github.com/pybind/pybind11/issues/2139, bug reported
  #      https://github.com/pybind/pybind11/pull/2370, which fixed by introducing "FindPython Mode"
  if [ -e src/ros2/pybind11_vendor/CMakeLists.txt ]; then
    sed -i 's|ExternalProject_Add(pybind11.*$|ExternalProject_Add(pybind11-2.7.1|' \
      src/ros2/pybind11_vendor/CMakeLists.txt
    sed -i 's|archive/v.*$|archive/v2.7.1.tar.gz|' \
      src/ros2/pybind11_vendor/CMakeLists.txt
    sed -i 's|URL_MD5.*$|URL_MD5 b87860218c143728f8e6efa6cba7e1ed|' \
      src/ros2/pybind11_vendor/CMakeLists.txt
  fi

  # Use new FindPython Mode, so pybind11 will include pybind11Tools.cmake instead of
  # pybind11Tools.cmake which finds PythonLibsNew and overrides PYTHON_* from host.
  if [ -e src/ros2/python_cmake_module/cmake/Modules/FindPythonExtra.cmake ]; then
    sed -i 's|^find_package(PythonInterp.*$|find_package(Python COMPONENTS Interpreter Development)|' \
      src/ros2/python_cmake_module/cmake/Modules/FindPythonExtra.cmake
    sed -i '/^.*find_package_handle_standard_args.*$/i set(PYTHON_MODULE_EXTENSION "${PythonExtra_EXTENSION_SUFFIX}${PythonExtra_EXTENSION_EXTENSION}" CACHE INTERNAL "")' \
      src/ros2/python_cmake_module/cmake/Modules/FindPythonExtra.cmake
  fi
}

prepare_source

mkdir -p /buildroot/build/ros /buildroot/target/opt/ros
# cmake's find_package only search sysroot path if CMAKE_SYSROOT has defined
# setting COLCON_CURRENT_PREFIX instead?
pushd /buildroot/host/aarch64-buildroot-linux-gnu/sysroot/
rm -rf opt
ln -sf ../../../target/opt/ros/ opt
popd

#FIXME: link buildroot binfmt just because CROSSCOMPILING_EMULATOR don't work
#       when building foonathan_memory_vendor, and installing qemu-user-static/qemu-user-binfmt
#       packages require root privilege. The CMAKE_CROSSCOMPILING_EMULATOR doesn't work too.
#       "-DCMAKE_CROSSCOMPILING_EMULATOR=/usr/bin/qemu-aarch64-static;-L;/buildroot/target/"
mkdir -p /etc/qemu-binfmt
ln -sf /buildroot/target/ /etc/qemu-binfmt/aarch64

# install mixin which setup building evinronments
colcon mixin remove default || true
colcon mixin add default file:///root/index.yaml
colcon mixin update default

# HINTS:
#  - Use --merge-install to avoid some find_package/find_path/find_library fails

LDFLAGS="-L/buildroot/host/lib -Wl,-rpath,/buildroot/host/lib" \
  CFLAGS="-O2 -I/buildroot/host/include/" \
  colcon build \
  --mixin aarch64-linux \
  --merge-install \
  --build-base /buildroot/build/ros/ \
  --install-base /buildroot/target/opt/ros \
  --cmake-force-configure \
# --packages-select  rclpy
