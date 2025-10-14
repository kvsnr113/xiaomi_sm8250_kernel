#!/usr/bin/env bash
# E404 Kernel Compile Script !
# Put a fucking credit if you use something from here !

# Set kernel source directory and base directory to place tools
KERNEL_DIR="$PWD"
cd ..
BASE_DIR="$PWD"
cd "$KERNEL_DIR"

set -eo pipefail
trap 'errorbuild' INT TERM ERR

AK3_DIR="$BASE_DIR/AnyKernel3"
[[ ! -d "$AK3_DIR" ]] && echo "!! Please Provide AnyKernel3 !!" && exit 1

# Parse command line arguments
TYPE="CI"
TC="Unknown-Clang"
TARGET=""
DEFCONFIG=""

case "$*" in
    *st*)
        git checkout main
        TYPE="STABLE" ;;
    *dev*) TYPE="DEV" ;;
    *sus*) 
        git checkout main-susfs
        TYPE="SUSFS" 
        ;;
esac

case "$*" in
    *aosp*)
        export PATH="$BASE_DIR/toolchains/aosp-clang/bin:$PATH"
        TC="AOSP-Clang"
        ;;
    *gcc*)
        GCC64_DIR="$BASE_DIR/toolchains/gcc/gcc-arm64/bin/"
        GCC32_DIR="$BASE_DIR/toolchains/gcc/gcc-arm/bin/"
        export PATH="$GCC64_DIR:$GCC32_DIR:/usr/bin:$PATH"
        TC="GCC"
        ;;
    *)
        export PATH="$BASE_DIR/toolchains/neutron-clang/bin:$PATH"
        TC="Neutron-Clang"
        ;;
esac

# Device selection using arrays
declare -A DEVICE_MAP=(
    ["munch"]="MUNCH:vendor/munch_defconfig"
    ["alioth"]="ALIOTH:vendor/alioth_defconfig"
    ["apollo"]="APOLLO:vendor/apollo_defconfig"
    ["pipa"]="PIPA:vendor/pipa_defconfig"
    ["lmi"]="LMI:vendor/lmi_defconfig"
    ["umi"]="UMI:vendor/umi_defconfig"
    ["cmi"]="CMI:vendor/cmi_defconfig"
    ["cas"]="CAS:vendor/cas_defconfig"
)

for device in "${!DEVICE_MAP[@]}"; do
    if [[ "$*" == *"$device"* ]]; then
        IFS=':' read -r TARGET DEFCONFIG <<< "${DEVICE_MAP[$device]}"
        sed -i "/devicename=/c\devicename=${device};" "$AK3_DIR/anykernel.sh"
        break
    fi
done

[[ ! "$TARGET" ]] && echo "-- !! Please set build device target !! --" && exit 1

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
    echo "-- Warning: Telegram config file not found at $TELEGRAM_CONFIG --"
    echo "-- Telegram notifications will be disabled --"
    export TOKEN=""
    export CHATID=""
fi

# Build environment
export ARCH="arm64"
export SUBARCH="arm64"
export TZ="Asia/Jakarta"

# Clean previous builds
rm -rf ../*E404R*.zip

# Function definitions
build_msg() {
    local BRANCH=$(git rev-parse --abbrev-ref HEAD)
    local COMMIT=$(git log -1 --pretty=format:'%s')
    local MSG=$(cat <<EOF
<b>Build Triggered !</b>
<code>Device : $TARGET</code>
<code>Branch : $BRANCH</code>
<code>ToolCh : $TC</code>
<b>Commit :</b>
<code>$COMMIT</code>
EOF
)
    send_msg "$MSG"
}

success_msg() {
    local MSG=$(cat <<EOF
<b>Build Success !</b>
<code>Date : $(date +"%d %b %Y, %H:%M:%S")</code>
<code>Time : $(($TIME_END / 60))m $(($TIME_END % 60))s</code>
EOF
)
    send_msg "$MSG"
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
        -F "parse_mode=html" \
        -F "disable_web_page_preview=true"
}

clearbuild() {
    if [[ "$1" == "all" ]]; then
        echo "-- Cleaning Out --"
        rm -rf out/*
    else
        rm -rf "$KERNEL_DIR/out/arch/arm64/boot"
    fi
}

zipbuild() {
    echo "-- Zipping Kernel --"
    cd "$AK3_DIR" || exit 1
    ZIP_NAME="E404R-${TYPE}-${TARGET}-$(date "+%y%m%d").zip"
    zip -r9 "$BASE_DIR/$ZIP_NAME" META-INF/ tools/ "${TARGET}"*-Image "${TARGET}"*-dtb "${TARGET}"*-dtbo.img anykernel.sh
    cd "$KERNEL_DIR" || exit 1
}

uploadbuild() {
    send_file "$BASE_DIR/compile.log"
    send_file "$BASE_DIR/$ZIP_NAME"
    send_msg "<b>Kernel Flashable Zip Uploaded</b>"
}

setupbuild() {
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
        export LLVM=1
        export LLVM_IAS=1
        
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
        export CROSS_COMPILE="aarch64-elf-"
        export CROSS_COMPILE_COMPAT="arm-eabi-"
    fi
}

errorbuild() {
    echo "-- !! Kernel Build Error !! --"
    send_file "$BASE_DIR/compile.log"
    send_msg "<b>! Kernel Build Error !</b>"
    clearbuild
    rm -f "$BASE_DIR/compile.log"
    exit 1
}

compilebuild() {    
    mkdir -p $KERNEL_DIR/out

    local make_flags=(-j"$(nproc)" O=out "${BUILD_FLAGS[@]}")
    
    if [[ $TC == *Clang* ]]; then
        echo "-- Compiling with Clang --"
        make "${make_flags[@]}" || errorbuild
    else
        echo "-- Compiling with GCC --"
        make "${make_flags[@]}" || errorbuild
    fi
}

makebuild() {
    # Config modifications
    sed -i '/CONFIG_KALLSYMS=/c\CONFIG_KALLSYMS=n' out/.config
    sed -i '/CONFIG_KALLSYMS_BASE_RELATIVE=/c\CONFIG_KALLSYMS_BASE_RELATIVE=n' out/.config
            
    if [[ "$1" == "SUSFS" ]]; then
        echo "-- Compiling with SUSFS --"
        sed -i '/CONFIG_KSU_SUSFS=/c\CONFIG_KSU_SUSFS=y' out/.config
        export CCACHE_DIR="$BASE_DIR/ccache/.ccache_susfs"
    else
        echo "-- Compiling without SUSFS --"
        sed -i '/CONFIG_KSU_SUSFS=/c\CONFIG_KSU_SUSFS=n' out/.config
        export CCACHE_DIR="$BASE_DIR/ccache/.ccache_nosusfs"
    fi
    compilebuild
    # Show ccache stats after build
    echo "======== CCache Stats =========="
    ccache -p | grep cache_dir
    ccache -s
    echo "================================"

    echo "-- Copying files to AnyKernel3 --"
    rm -f "$AK3_DIR/${TARGET}-$1-Image"
    rm -f "$AK3_DIR/${TARGET}-dtbo.img"
    rm -f "$AK3_DIR/${TARGET}-dtb"
    cp "$K_IMG" "$AK3_DIR/${TARGET}-$1-Image"
    cp "$K_DTBO" "$AK3_DIR/${TARGET}-dtbo.img"
    cp "$K_DTB" "$AK3_DIR/${TARGET}-dtb"
}

setupbuild

# Main menu
while true; do
    echo ""
    echo " Menu "
    echo " ╔════════════════════════════════════╗"
    echo " ║ 1. Export Defconfig                ║"
    echo " ║ 2. Start Build                     ║"
    echo " ║ 3. Send File                       ║"
    echo " ║ 4. Repack Last Build               ║"
    echo " ║ f. Clean Out Directory             ║"
    echo " ║ fc. Clean Ccache                   ║"
    echo " ║ e. Exit                            ║"
    echo " ╚════════════════════════════════════╝"
    echo -n " Enter your choice : "
    read -r menu
    
    case "$menu" in
        1)
            make O=out "$DEFCONFIG"
            echo "-- Exported $DEFCONFIG to Out Dir --"
            ;;
        2)
            TIME_START="$(date +"%s")"
            build_msg
            clearbuild
            makebuild "SUSFS" 2>&1 | tee -a "$BASE_DIR/compile.log"
            clearbuild
            makebuild "NOSUSFS" 2>&1 | tee -a "$BASE_DIR/compile.log"
            zipbuild
            uploadbuild
            rm -f "$BASE_DIR/compile.log"
            TIME_END=$(("$(date +"%s")" - "$TIME_START"))
            success_msg
            ;;
        3)
            echo "-- Sending to Telegram --"
            send_file "$BASE_DIR/$ZIP_NAME"
            ;;
        4)
            zipbuild
            send_file "$BASE_DIR/$ZIP_NAME"
            ;;
        f)
            clearbuild "all"
            ;;
        fc)
            rm -rf "$BASE_DIR/ccache"
            ;;
        e)
            exit 0
            ;;
        *)
            echo "-- !! Invalid option !! --"
            ;;
    esac
done