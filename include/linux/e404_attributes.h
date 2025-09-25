#ifndef _E404_ATTRIBUTES_H
#define _E404_ATTRIBUTES_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

struct e404_attributes {
    int kernelsu;
    int effcpu;
    int rom_type;
    int dtbo_type;
    int batt_profile;
    int kgsl_skip_zeroing;
    int pid_shrink;
    int file_sync;
    int panel_width;
    int panel_height;
    int oem_panel_width;
    int oem_panel_height;
};

extern struct e404_attributes e404_data;

extern int early_kernelsu;
extern int early_effcpu;
extern int early_rom_type;
extern int early_dtbo_type;
extern int early_batt_profile;

#endif /* _E404_ATTRIBUTES_H */
