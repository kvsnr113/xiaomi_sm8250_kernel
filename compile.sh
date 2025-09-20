#!/usr/bin/env bash
# E404 Kernel Compile Script !
# Put a fucking credit if you use something from here !

# Set kernel source directory and base directory to place tools
KERNEL_DIR="$PWD"
cd ..
BASE_DIR="$PWD"
cd "$KERNEL_DIR"

AK3_DIR="$BASE_DIR/AnyKernel3"
[[ ! -d "$AK3_DIR" ]] && echo -e "(X) Please Provide AnyKernel3 !" && exit 1

# Parse command line arguments
TYPE="CI"
TC="Unknown-Clang"
TARGET=""
DEFCONFIG=""

case "$*" in
    *st*)
        git checkout main
        TYPE="ST" ;;
    *dev*) TYPE="DEV" ;;
    *sus*) 
        git checkout main-susfs
        TYPE="SUSFS" 
        ;;
esac

case "$*" in
    *aosp*)
        export PATH="$BASE_DIR/aosp-clang/bin:$PATH"
        TC="AOSP-Clang"
        ;;
    *gcc*)
        GCC64_DIR="$BASE_DIR/gcc/gcc-arm64/bin/"
        GCC32_DIR="$BASE_DIR/gcc/gcc-arm/bin/"
        export PATH="$GCC64_DIR:$GCC32_DIR:/usr/bin:$PATH"
        TC="GCC"
        ;;
    *)
        export PATH="$BASE_DIR/neutron-clang/bin:$PATH"
        TC="Neutron-Clang"
        ;;
esac

case "$*" in
    *munch*)
        sed -i '/devicename=/c\devicename=munch;' "$AK3_DIR/anykernel.sh"
        TARGET="MUNCH"
        DEFCONFIG="vendor/munch_defconfig"
        ;;
    *alioth*)
        sed -i '/devicename=/c\devicename=alioth;' "$AK3_DIR/anykernel.sh"
        TARGET="ALIOTH"
        DEFCONFIG="vendor/alioth_defconfig"
        ;;
    *apollo*)
        sed -i '/devicename=/c\devicename=apollo;' "$AK3_DIR/anykernel.sh"
        TARGET="APOLLO"
        DEFCONFIG="vendor/apollo_defconfig"
        ;;
    *lmi*)
        sed -i '/devicename=/c\devicename=lmi;' "$AK3_DIR/anykernel.sh"
        TARGET="LMI"
        DEFCONFIG="vendor/lmi_defconfig"
        ;;
    *umi*)
        sed -i '/devicename=/c\devicename=umi;' "$AK3_DIR/anykernel.sh"
        TARGET="UMI"
        DEFCONFIG="vendor/umi_defconfig"
        ;;
    *cmi*)
        sed -i '/devicename=/c\devicename=cmi;' "$AK3_DIR/anykernel.sh"
        TARGET="CMI"
        DEFCONFIG="vendor/cmi_defconfig"
        ;;
    *cas*)
        sed -i '/devicename=/c\devicename=cas;' "$AK3_DIR/anykernel.sh"
        TARGET="CAS"
        DEFCONFIG="vendor/cas_defconfig"
        ;;
esac

# Set kernel image paths
K_IMG="$KERNEL_DIR/out/arch/arm64/boot/Image"
K_DTBO="$KERNEL_DIR/out/arch/arm64/boot/dtbo.img"
K_DTB="$KERNEL_DIR/out/arch/arm64/boot/dtb"

# Telegram configuration - Load from external file
TELEGRAM_CONFIG="$BASE_DIR/kernel_build"
if [[ -f "$TELEGRAM_CONFIG" ]]; then
    source "$TELEGRAM_CONFIG"
    export TOKEN="$TELEGRAM_TOKEN"
    export CHATID="$TELEGRAM_CHATID"
else
    echo "Warning: Telegram config file not found at $TELEGRAM_CONFIG"
    echo "Telegram notifications will be disabled"
    export TOKEN=""
    export CHATID=""
fi

# Build environment
export ARCH="arm64"
export SUBARCH="arm64"
export KBUILD_BUILD_USER="vyn"
export KBUILD_BUILD_HOST="wsl2"
export TZ="Asia/Jakarta"

# Clean previous builds
rm -rf ../*E404R*.zip

# Function definitions
build_msg() {
    send_msg "
<b>Build Triggered !</b>
<b>Machine :</b>
<b>•</b> <code>$(lscpu | sed -nr '/Model name/ s/.*:\s*(.*) */\1/p')</code>
<b>•</b> RAM <code>$(cat /proc/meminfo | numfmt --field 2 --from-unit=Ki --to-unit=Mi | sed 's/ kB/M/g' | grep 'MemTotal' | awk -F ':' '{print $2}' | tr -d ' ')</code> | Free <code>$(cat /proc/meminfo | numfmt --field 2 --from-unit=Ki --to-unit=Mi | sed 's/ kB/M/g' | grep 'MemFree' | awk -F ':' '{print $2}' | tr -d ' ')</code> | Swap <code>$(cat /proc/meminfo | numfmt --field 2 --from-unit=Ki --to-unit=Mi | sed 's/ kB/M/g' | grep 'SwapTotal' | awk -F ':' '{print $2}' | tr -d ' ')</code>
<b>==================================</b>
<b>Device : </b><code>$TARGET</code>
<b>Branch : </b><code>$(git rev-parse --abbrev-ref HEAD)</code>
<b>Commit : </b><code>$(git log --pretty=format:'%s' -1)</code>
<b>TC     : </b><code>$TC</code>
<b>==================================</b>"
}

success_msg() {
    send_msg "
<b>Build Success !</b>
<b>==================================</b>
<b>Build Date : </b>
<code>$(date +"%A, %d %b %Y, %H:%M:%S")</code>
<b>Build Took : </b>
<code>$(($TOTAL_TIME / 60)) Minutes, $(($TOTAL_TIME % 60)) Second</code>
<b>==================================</b>"
}

send_msg() {
    curl -s -X POST \
        "https://api.telegram.org/bot$TOKEN/sendMessage" \
        -d chat_id="$CHATID" \
        -d text="$1" \
        -d "parse_mode=html" \
        -d "disable_web_page_preview=true"
}

send_file() {
    curl -s -X POST \
        "https://api.telegram.org/bot$TOKEN/sendDocument" \
        -F chat_id="$CHATID" \
        -F document=@"$1" \
        -F caption="$2" \
        -F "parse_mode=html" \
        -F "disable_web_page_preview=true"
}

clearbuild() {
    rm -rf "$K_IMG" "$K_DTB" "$K_DTBO"
    rm -rf out/arch/arm64/boot/dts/vendor/qcom
}

zipbuild() {
    echo -e "(OK) Zipping Kernel !"
    cd "$AK3_DIR" || exit 1
    ZIP_NAME="E404R-${TYPE}-${TARGET}-$(date "+%y%m%d").zip"
    zip -r9 "$BASE_DIR/$ZIP_NAME" */ "${TARGET}"-* anykernel.sh -x .git README.md LICENSE
    cd "$KERNEL_DIR" || exit 1
}

uploadbuild() {
    send_file "$BASE_DIR/$ZIP_NAME" ""
    send_msg "<b>Kernel Flashable Zip Uploaded</b>"
}

setup_build_flags() {
    if [[ $TC == *Clang* ]]; then
        BUILD_FLAGS=(
            CC="ccache clang"
            LD="ld.lld"
            AR="llvm-ar"
            NM="llvm-nm"
            OBJCOPY="llvm-objcopy"
            OBJDUMP="llvm-objdump"
            STRIP="llvm-strip"
            LLVM=1
            LLVM_IAS=1
            CROSS_COMPILE="aarch64-linux-gnu-"
            CROSS_COMPILE_ARM32="arm-linux-gnueabi-"
            CROSS_COMPILE_COMPAT="arm-linux-gnueabi-"
        )
        
        # Export for defconfig (without ccache)
        export CC="clang"
        export LD="ld.lld"
        export AR="llvm-ar"
        export NM="llvm-nm"
        export OBJCOPY="llvm-objcopy"
        export OBJDUMP="llvm-objdump"
        export STRIP="llvm-strip"
        export LLVM=1
        export LLVM_IAS=1
        export CROSS_COMPILE="aarch64-linux-gnu-"
        export CROSS_COMPILE_ARM32="arm-linux-gnueabi-"
        export CROSS_COMPILE_COMPAT="arm-linux-gnueabi-"
        
    else
        BUILD_FLAGS=(
            CC="ccache aarch64-elf-gcc"
            LD="aarch64-elf-ld.lld"
            AR="llvm-ar"
            NM="llvm-nm"
            OBJCOPY="llvm-objcopy"
            OBJDUMP="llvm-objdump"
            STRIP="llvm-strip"
            CROSS_COMPILE="aarch64-elf-"
            CROSS_COMPILE_COMPAT="arm-eabi-"
        )
        
        # Export for defconfig (without ccache)
        export CC="aarch64-elf-gcc"
        export LD="aarch64-elf-ld.lld"
        export AR="llvm-ar"
        export NM="llvm-nm"
        export OBJCOPY="llvm-objcopy"
        export OBJDUMP="llvm-objdump"
        export STRIP="llvm-strip"
        export CROSS_COMPILE="aarch64-elf-"
        export CROSS_COMPILE_COMPAT="arm-eabi-"
    fi

    echo "=== Build Flags ==="
    printf '%s\n' "${BUILD_FLAGS[@]}"
    echo "CC: $(which clang)"
    echo "CCache: $(which ccache)"
    ccache -s
    echo "==================="
}

setup_build_flags

compilebuild() {
    # Show ccache configuration before build
    echo "=== CCache Stats Before Build ==="
    ccache -s
    echo "================================"
    
    local make_flags=(-kj16 O=out "${BUILD_FLAGS[@]}")
    
    if [[ $TC == *Clang* ]]; then
        echo "Compiling with Clang (using ccache)"
        make "${make_flags[@]}" 2>&1 | tee -a out/log.txt
    else
        echo "Compiling with GCC"
        make "${make_flags[@]}" 2>&1 | tee -a out/log.txt
    fi

    # Show ccache stats after build
    echo "=== CCache Stats After Build ==="
    ccache -s
    echo "==============================="
    
    if [[ ! -e $K_IMG ]]; then
        echo -e "(X) Kernel Build Error !"
        send_file "out/log.txt"
        git restore "arch/arm64/configs/$DEFCONFIG"
        send_msg "<b>! Kernel Build Error !</b>"
        exit 1
    fi
}

makebuild() {
    compilebuild
    cp "$K_IMG" "$AK3_DIR/${TARGET}-Image"
    cp "$K_DTBO" "$AK3_DIR/${TARGET}-dtbo.img"
    cp "$K_DTB" "$AK3_DIR/${TARGET}-dtb"
}

# Main menu
while true; do
    echo ""
    echo " Menu "
    echo " ╔════════════════════════════════════╗"
    echo " ║ 1. Export Defconfig                ║"
    echo " ║ 2. Start Build                     ║"
    echo " ║ 3. Send File                       ║"
    echo " ║ f. Clean Out Directory             ║"
    echo " ║ e. Exit                            ║"
    echo " ╚════════════════════════════════════╝"
    echo -n " Enter your choice: "
    read -r menu
    
    case "$menu" in
        1)
            make O=out "$DEFCONFIG"
            echo -e "(OK) Exported $DEFCONFIG to Out Dir !"
            ;;
        2)
            START="$(date +"%s")"
            
            # Config modifications
            sed -i '/CONFIG_KALLSYMS=/c\CONFIG_KALLSYMS=n' out/.config
            sed -i '/CONFIG_KALLSYMS_BASE_RELATIVE=/c\CONFIG_KALLSYMS_BASE_RELATIVE=n' out/.config
            sed -i '/CONFIG_E404_OPLUS/c\CONFIG_E404_OPLUS=y' out/.config
            
            # LTO configuration
            if [[ "$TC" != *GCC* ]]; then
                if [[ "$TYPE" == "FLTO" ]]; then
                    sed -i '/CONFIG_LTO_CLANG_THIN/c\CONFIG_LTO_CLANG_THIN=n' out/.config
                    sed -i '/CONFIG_LTO_CLANG_FULL/c\CONFIG_LTO_CLANG_FULL=y' out/.config
                else 
                    sed -i '/CONFIG_LTO_CLANG_THIN/c\CONFIG_LTO_CLANG_THIN=y' out/.config
                    sed -i '/CONFIG_LTO_CLANG_FULL/c\CONFIG_LTO_CLANG_FULL=n' out/.config
                fi
            else
                sed -i '/CONFIG_LTO_NONE/c\CONFIG_LTO_NONE=y' out/.config
                sed -i '/CONFIG_LTO=/c\CONFIG_LTO=n' out/.config
                sed -i '/CONFIG_LTO_CLANG=/c\# CONFIG_LTO_CLANG is not set' out/.config
                sed -i '/CONFIG_LTO_CLANG_THIN/c\# CONFIG_LTO_CLANG_THIN is not set' out/.config
                sed -i '/CONFIG_LTO_CLANG_FULL/c\# CONFIG_LTO_CLANG_FULL is not set' out/.config
            fi

            build_msg
            clearbuild
            makebuild
            zipbuild
            clearbuild

            TOTAL_TIME=$(("$(date +"%s")" - "$START"))
            success_msg
            send_file "out/log.txt" "<b>CI Log Uploaded</b>"
            rm -rf out/log.txt
            uploadbuild
            ;;
        3)
            echo -e "(OK) Sending to Telegram"
            send_file "$BASE_DIR/*E404*.zip" ""
            ;;
        f)
            rm -rf out
            ;;
        e)
            exit 0
            ;;
        *)
            echo "Invalid option!"
            ;;
    esac
done