ifeq ($(CONFIG_BOARD_XIAOMI_SM8250),y)
ifeq ($(CONFIG_BUILD_ARM64_DT_OVERLAY),y)

ifeq ($(CONFIG_BOARD_ALIOTH),y)
dtbo-$(CONFIG_BOARD_ALIOTH) += \
                alioth-sm8250-overlay.dtbo
alioth-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_APOLLO),y)
dtbo-$(CONFIG_BOARD_APOLLO) += \
                apollo-sm8250-overlay.dtbo
apollo-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_CAS),y)
dtbo-$(CONFIG_BOARD_CAS) += \
                cas-sm8250-overlay.dtbo
cas-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_CMI),y)
dtbo-$(CONFIG_BOARD_CMI) += \
                cmi-sm8250-overlay.dtbo
cmi-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_DAGU),y)
dtbo-$(CONFIG_BOARD_DAGU) += \
                dagu-sm8250-overlay.dtbo
dagu-sm8250-overlay.dtbo-base := kona.dtb kona-v2.dtb kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_ELISH),y)
dtbo-$(CONFIG_BOARD_ELISH) += \
                elish-sm8250-overlay.dtbo
elish-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_ENUMA),y)
dtbo-$(CONFIG_BOARD_ENUMA) += \
                enuma-sm8250-overlay.dtbo
enuma-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_LMI),y)
dtbo-$(CONFIG_BOARD_LMI) += \
                lmi-sm8250-overlay.dtbo
lmi-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_PIPA),y)
dtbo-$(CONFIG_BOARD_PIPA) += \
                pipa-sm8250-overlay.dtbo
pipa-sm8250-overlay.dtbo-base := kona.dtb kona-v2.dtb kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_THYME),y)
dtbo-$(CONFIG_BOARD_THYME) += \
                thyme-sm8250-overlay.dtbo
thyme-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif

ifeq ($(CONFIG_BOARD_UMI),y)
dtbo-$(CONFIG_BOARD_UMI) += \
                umi-sm8250-overlay.dtbo
umi-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif


ifeq ($(CONFIG_BOARD_MUNCH),y)
dtbo-$(CONFIG_BOARD_MUNCH) += \
		munch-sm8250-overlay.dtbo
munch-sm8250-overlay.dtbo-base := kona-v2.1.dtb
endif

endif
endif
always		:= $(dtb-y)
subdir-y	:= $(dts-dirs)
clean-files    := *.dtb *.dtbo
