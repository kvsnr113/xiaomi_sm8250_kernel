#ifndef _E404_ATTRIBUTES_H
#define _E404_ATTRIBUTES_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

struct e404_attributes {
    int e404_kernelsu;
    int e404_effcpu;
    int e404_rom_type;
    int e404_dtbo_type;
    int e404_ir_type;
    int e404_batt_profile;
    int e404_dvq_input_boost;
    int e404_kgsl_skip_zeroing;
    int e404_panel_width;
    int e404_panel_height;
    int e404_oem_panel_width;
    int e404_oem_panel_height;
};

extern struct e404_attributes e404_data;

extern int e404_early_kernelsu;
extern int e404_early_effcpu;
extern int e404_early_rom_type;
extern int e404_early_dtbo_type;
extern int e404_early_ir_type;
extern int e404_early_batt_profile;

#endif /* _E404_ATTRIBUTES_H */
