# This file describes an image that has everything necessary installed to build a target ROS workspace
# It uses QEmu user-mode emulation to perform dependency installation and build
# Assumptions: qemu-user-static directory is present in docker build context

ARG BASE_IMAGE
FROM ${BASE_IMAGE}

ARG ROS_VERSION
ARG ROS_DISTRO

SHELL ["/bin/bash", "-c"]

# Set timezone
RUN echo 'Etc/UTC' > /etc/timezone && \
    ln -sf /usr/share/zoneinfo/Etc/UTC /etc/localtime

RUN apt-get update && apt-get install --no-install-recommends -y \
        tzdata \
        locales \
        vim \
    && rm -rf /var/lib/apt/lists/*

# Set locale
RUN echo 'en_US.UTF-8 UTF-8' >> /etc/locale.gen && \
    locale-gen && \
    update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LC_ALL C.UTF-8

# Add the ros apt repo
RUN apt-get update && apt-get install --no-install-recommends -y \
        ca-certificates \
        curl wget git \
        dirmngr \
        gnupg2 \
        lsb-release \
    && rm -rf /var/lib/apt/lists/*
RUN if [[ "${ROS_VERSION}" == "ros2" ]]; then \
      curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg; \
      echo "deb [signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/${ROS_VERSION}/ubuntu `lsb_release -cs` main" | \
          tee /etc/apt/sources.list.d/ros2.list > /dev/null; \
    else \
      curl -s https://raw.githubusercontent.com/ros/rosdistro/master/ros.asc | apt-key add - ; \
      echo "deb http://packages.ros.org/${ROS_VERSION}/ubuntu `lsb_release -cs` main" \
          > /etc/apt/sources.list.d/${ROS_VERSION}-latest.list; \
    fi

# ROS dependencies
RUN apt-get update && apt-get install --no-install-recommends -y \
      build-essential \
      cmake libpcre2-dev \
      python3-colcon-common-extensions \
      python3-colcon-mixin \
      python3-dev \
      python3-pip \
      python3-lark-parser \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install -U \
    setuptools

# Install some pip packages needed for testing ROS 2
RUN if [[ "${ROS_VERSION}" == "ros2" ]]; then \
    python3 -m pip install -U \
    flake8 \
    flake8-blind-except \
    flake8-builtins \
    flake8-class-newline \
    flake8-comprehensions \
    flake8-deprecated \
    flake8-docstrings \
    flake8-import-order \
    flake8-quotes \
    pytest-repeat \
    pytest-rerunfailures \
    pytest \
    pytest-cov \
    pytest-runner \
  ; fi

# Install Fast-RTPS dependencies for ROS 2
RUN if [[ "${ROS_VERSION}" == "ros2" ]]; then \
    apt-get update && apt-get install --no-install-recommends -y \
        libasio-dev \
        libtinyxml2-dev \
    && rm -rf /var/lib/apt/lists/* \
  ; fi

RUN python3 -m pip install -U vcstool colcon-common-extensions numpy

# Run arbitrary user setup (copy data and run script)
# COPY user-custom-data/ custom-data/
# COPY user-custom-setup .
# RUN chmod +x ./user-custom-setup && \
#    ./user-custom-setup && \
#    rm -rf /var/lib/apt/lists/*

# mixins defines building environments
COPY mixins /root
RUN mkdir -p /buildroot /opt/ros/${ROS_DISTRO}/src

WORKDIR /opt/ros/${ROS_DISTRO}

# Disable packages
#  - rviz, X11/desktop required, if you want rviz, trying arm Ubuntu 18.04
#  - turtlesim, QT5 required
RUN wget https://raw.githubusercontent.com/ros2/ros2/${ROS_DISTRO}/ros2.repos && \
    vcs-import src < ros2.repos

RUN git clone https://github.com/ros2/cross_compile.git -b 0.9.0 /tmp/cross_compile && \
    cp /tmp/cross_compile/ros_cross_compile/qemu/qemu-aarch64-static /usr/bin && \
    cp /tmp/cross_compile/ros_cross_compile/qemu/qemu-arm-static /usr/bin && \
    rm -rf /tmp/cross_compile

RUN if [[ -e src/ros-visualization ]]; then \
    touch src/ros-visualization/COLCON_IGNORE ; \
    echo "IGNORE ros-visualization"; \
    fi
RUN if [[ -e src/ros2/rviz ]]; then \
    touch src/ros2/rviz/COLCON_IGNORE ; \
    echo "IGNORE ros2/rviz"; \
    fi
RUN if [[ -e src/ros/ros_tutorials/turtlesim ]]; then \
    touch src/ros/ros_tutorials/turtlesim/COLCON_IGNORE;  \
    echo "IGNORE ros_tutorials/turtlesim"; \
    fi
RUN if [[ -e src/eclipse-iceoryx ]]; then \
    touch src/eclipse-iceoryx/COLCON_IGNORE; \
    echo "IGNORE eclipse-iceoryx"; \
    fi
RUN if [[ -e src/eclipse-cyclonedds ]]; then \
    touch src/eclipse-cyclonedds/COLCON_IGNORE; \
    echo "IGNORE eclipse-cyclonedds"; \
    fi

COPY build_ros2.sh /opt/ros/${ROS_DISTRO}
RUN ln -s /opt/ros/${ROS_DISTRO}/build_ros2.sh /root
RUN sed -i "s/^ROS_DISTRO=.*$/ROS_DISTRO=${ROS_DISTRO}/" /root/build_ros2.sh

ENTRYPOINT ["/root/build_ros2.sh"]
