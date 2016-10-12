#
# build scripts for MCF54418 TWR Board
# Author: Han Pengfei <pengphei@sina.com>
#

export BUILD_TRUNK=$(pwd)
export BUILD_TRUNK_OUT=${BUILD_TRUNK}/out_m54418

export BUILD_LINUX_PATH=${BUILD_TRUNK}/package/linux-2.6.29
export BUILD_UBOOT_PATH=${BUILD_TRUNK}/package/u-boot-2009.08
export BUILD_TOOLS_PATH=${BUILD_TRUNK}/tools/toolchain
export BUILD_TOOLCHAIN_PATH=${BUILD_TOOLS_PATH}/bin
export BUILD_ARCH=m68k
export BUILD_CROSS_COMPILE=m68k-linux-gnu-
export BUILD_UBOOT_CONFIG=M54418TWR_nand_rmii_config
export BUILD_KERNEL_CONFIG=m54418twr_defconfig

export PATH=$PATH:${BUILD_TOOLCHAIN_PATH}:${BUILD_UBOOT_PATH}/tools
export LANG=C
export LC_ALL=C

function build_prepare()
{
    if [ ! -d ${BUILD_TRUNK_OUT} ]; then
        mkdir ${BUILD_TRUNK_OUT}
    fi
}

function build_uboot()
{
    cd ${BUILD_UBOOT_PATH}
    make ARCH=${BUILD_ARCH} CROSS_COMPILE=${BUILD_CROSS_COMPILE} distclean
    make ARCH=${BUILD_ARCH} CROSS_COMPILE=${BUILD_CROSS_COMPILE} ${BUILD_UBOOT_CONFIG}
    make ARCH=${BUILD_ARCH} CROSS_COMPILE=${BUILD_CROSS_COMPILE}
    cp ${BUILD_UBOOT_PATH}/u-boot.bin ${BUILD_TRUNK_OUT}
    cd -
}

function build_kernel()
{
    cd ${BUILD_LINUX_PATH}

    if [ -d output ]; then
        rm -rf output
        mkdir output
    else
        mkdir output
    fi

    if [ ! -e .config ]; then
        cp arch/${BUILD_ARCH}/configs/${BUILD_KERNEL_CONFIG} .config
    fi

    # build kernel uImage and modules
    make ARCH=${BUILD_ARCH} CROSS_COMPILE=${BUILD_CROSS_COMPILE} uImage modules

    # install modules to target directory
    make INSTALL_MOD_PATH=output ARCH=${BUILD_ARCH} CROSS_COMPILE=${BUILD_CROSS_COMPILE} modules_install

    if [ -r include/generated/utsrelease.h ]; then
        KERNEL_VERSION=`cat include/generated/utsrelease.h |awk -F\" '{print $2}'`
    fi

    BUILD_KERNEL_MODULES_OUT=${BUILD_LINUX_PATH}/output/lib/modules/${KERNEL_VERSION}

    rm -rf ${BUILD_LINUX_PATH}/output/lib/modules/${KERNEL_VERSION}/build
    rm -rf ${BUILD_LINUX_PATH}/output/lib/modules/${KERNEL_VERSION}/source

    cp ${BUILD_LINUX_PATH}/arch/${BUILD_ARCH}/boot/uImage ${BUILD_TRUNK_OUT}
    
    rm -rf ${BUILD_TRUNK_OUT/modules
    cp -r ${BUILD_LINUX_PATH}/output/lib/modules ${BUILD_TRUNK_OUT}

    cd -
}	

coldfire_prepare
