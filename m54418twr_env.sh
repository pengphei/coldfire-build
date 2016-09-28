#
# build scripts for MCF54418 TWR Board
# Author: Han Pengfei <pengphei@sina.com>
#


export BUILD_TRUNK=$(pwd)
export BUILD_TRUNK_OUT=${BUILD_TRUNK}/out_m54418

export COLDFIRE_LINUX_PATH=${BUILD_TRUNK}/linux-2.6.29
export COLDFIRE_UBOOT_PATH=${BUILD_TRUNK}/u-boot-2009.08
export COLDFIRE_TOOLS_PATH=${BUILD_TRUNK}/coldfire-build/toolchain
export COLDFIRE_TOOLCHAIN_PATH=${COLDFIRE_TOOLS_PATH}/bin
export COLDFIRE_ARCH=m68k
export COLDFIRE_CROSS_COMPILE=m68k-linux-gnu-
export COLDFIRE_UBOOT_CONFIG=M54418TWR_nand_rmii_config
export COLDFIRE_KERNEL_CONFIG=m54418twr_defconfig

export PATH=$PATH:${COLDFIRE_TOOLCHAIN_PATH}:${COLDFIRE_UBOOT_PATH}/tools
export LANG=C
export LC_ALL=C

function coldfire_prepare()
{
    if [ ! -d ${BUILD_TRUNK_OUT} ]; then
        mkdir ${BUILD_TRUNK_OUT}
    fi
}

function coldfire_build_uboot()
{
    cd ${COLDFIRE_UBOOT_PATH}
    make ARCH=${COLDFIRE_ARCH} CROSS_COMPILE=${COLDFIRE_CROSS_COMPILE} distclean
    make ARCH=${COLDFIRE_ARCH} CROSS_COMPILE=${COLDFIRE_CROSS_COMPILE} ${COLDFIRE_UBOOT_CONFIG}
    make ARCH=${COLDFIRE_ARCH} CROSS_COMPILE=${COLDFIRE_CROSS_COMPILE}
    cp ${COLDFIRE_UBOOT_PATH}/u-boot.bin ${BUILD_TRUNK_OUT}
    cd -
}

function coldfire_build_kernel()
{
    cd ${COLDFIRE_LINUX_PATH}

    if [ -d output ]; then
        rm -rf output
        mkdir output
    else
        mkdir output
    fi

    if [ ! -e .config ]; then
        cp arch/${COLDFIRE_ARCH}/configs/${COLDFIRE_KERNEL_CONFIG} .config
    fi

    # build kernel uImage and modules
    make ARCH=${COLDFIRE_ARCH} CROSS_COMPILE=${COLDFIRE_CROSS_COMPILE} uImage modules

    # install modules to target directory
    make INSTALL_MOD_PATH=output ARCH=${COLDFIRE_ARCH} CROSS_COMPILE=${COLDFIRE_CROSS_COMPILE} modules_install

    if [ -r include/generated/utsrelease.h ]; then
        KERNEL_VERSION=`cat include/generated/utsrelease.h |awk -F\" '{print $2}'`
    fi

    COLDFIRE_KERNEL_MODULES_OUT=${COLDFIRE_LINUX_PATH}/output/lib/modules/${KERNEL_VERSION}

    cp ${COLDFIRE_LINUX_PATH}/arch/${COLDFIRE_ARCH}/boot/uImage ${BUILD_TRUNK_OUT}

    cd -
}	

coldfire_prepare
