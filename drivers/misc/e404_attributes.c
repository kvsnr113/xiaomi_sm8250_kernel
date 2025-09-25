// E404 Manager by Project 113 (kvsnr113)

#include <linux/e404_attributes.h>

struct e404_attributes e404_data = {
    .kernelsu = 0,
    .effcpu = 0,
    .rom_type = 1,
    .dtbo_type = 1,
    .batt_profile = 1,
    .kgsl_skip_zeroing = 0,
    .pid_shrink = 0,
    .file_sync = 1,
    .panel_width = 70,
    .panel_height = 155,
    .oem_panel_width = 700,
    .oem_panel_height = 1550
};

static struct kobject *e404_kobj;

#ifdef CONFIG_E404_KSU_DEFAULT
int early_kernelsu = 1;
#else
int early_kernelsu = 0;
#endif

#ifdef CONFIG_E404_EFFCPU_DEFAULT
int early_effcpu = 1;
#else
int early_effcpu = 0;
#endif

#ifdef CONFIG_E404_MIUI
int early_rom_type = 2;
#else
int early_rom_type = 1;
#endif

#ifdef CONFIG_E404_MIUI
int early_dtbo_type = 2;
#else
int early_dtbo_type = 1;
#endif

#ifdef CONFIG_E404_ALIOTH_5K_BATT_DEFAULT
int early_batt_profile = 2;
#else
int early_batt_profile = 1;
#endif

static int __init parse_e404_args(char *str)
{
    char *arg;

    while ((arg = strsep(&str, " ,")) != NULL) {
        if (!*arg) continue;

        pr_alert("E404: Parsing flag: %s\n", arg);

        if (strcmp(arg, "root_ksu") == 0)
            early_kernelsu = 1;
        else if (strcmp(arg, "root_noksu") == 0)
            early_kernelsu = 0;
        else if (strcmp(arg, "dtb_effcpu") == 0)
            early_effcpu = 1;
        else if (strcmp(arg, "dtb_def") == 0)
            early_effcpu = 0;
        else if (strcmp(arg, "rom_port") == 0)
            early_rom_type = 3;
        else if (strcmp(arg, "rom_oem") == 0)
            early_rom_type = 2;
        else if (strcmp(arg, "rom_aosp") == 0)
            early_rom_type = 1;
        else if (strcmp(arg, "dtbo_def") == 0)
            early_dtbo_type = 1;
        else if (strcmp(arg, "dtbo_oem") == 0)
            early_dtbo_type = 2;
        else if (strcmp(arg, "batt_def") == 0)
            early_batt_profile = 1;
        else if (strcmp(arg, "batt_5k") == 0)
            early_batt_profile = 2;
        else
            pr_alert("E404: Unknown flag: %s\n", arg);
    }

    return 0;
}
early_param("e404_args", parse_e404_args);

static void e404_parse_attributes(void) {
    e404_data.kernelsu = early_kernelsu;
    e404_data.effcpu = early_effcpu;
    e404_data.rom_type = early_rom_type;
    e404_data.dtbo_type = early_dtbo_type;
    e404_data.batt_profile = early_batt_profile;

    pr_alert("E404 Early Attributes: KernelSU=%d, EFFCPU=%d, RomType=%d, DTBOType=%d, BatteryProfile=%d\n",
        e404_data.kernelsu,
        e404_data.effcpu,
        e404_data.rom_type,
        e404_data.dtbo_type,
        e404_data.batt_profile);
}

#define E404_ATTR_RO(name) \
static ssize_t name##_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { \
    return sprintf(buf, "%d\n", e404_data.name); \
} \
static struct kobj_attribute name##_attr = __ATTR(name, 0444, name##_show, NULL);

#define E404_ATTR_RW(name) \
static ssize_t name##_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) { \
    return sprintf(buf, "%d\n", e404_data.name); \
} \
static ssize_t name##_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) { \
    int ret, val, old_val; \
    ret = kstrtoint(buf, 10, &val); \
    if (ret) return ret; \
    old_val = e404_data.name; \
    e404_data.name = val; \
    pr_alert("E404: %s changed from %d to %d\n", #name, old_val, val); \
    sysfs_notify(e404_kobj, NULL, #name); \
    return count; \
} \
static struct kobj_attribute name##_attr = __ATTR(name, 0664, name##_show, name##_store);

E404_ATTR_RO(kernelsu);
E404_ATTR_RO(effcpu);
E404_ATTR_RO(rom_type);
E404_ATTR_RO(dtbo_type);
E404_ATTR_RO(batt_profile);
E404_ATTR_RO(panel_width);
E404_ATTR_RO(panel_height);
E404_ATTR_RO(oem_panel_width);
E404_ATTR_RO(oem_panel_height);

E404_ATTR_RW(kgsl_skip_zeroing);
E404_ATTR_RW(pid_shrink);
E404_ATTR_RW(file_sync);

static struct attribute *e404_attrs[] = {
    &kernelsu_attr.attr,
    &effcpu_attr.attr,
    &rom_type_attr.attr,
    &dtbo_type_attr.attr,
    &batt_profile_attr.attr,
    &kgsl_skip_zeroing_attr.attr,
    &pid_shrink_attr.attr,
    &file_sync_attr.attr,
    &panel_width_attr.attr,
    &panel_height_attr.attr,
    &oem_panel_width_attr.attr,
    &oem_panel_height_attr.attr,
    NULL,
};

static struct attribute_group e404_attr_group = {
    .attrs = e404_attrs,
};

static int __init e404_init(void) {
    int ret;

    e404_parse_attributes();

    e404_kobj = kobject_create_and_add("e404", kernel_kobj);
    if (!e404_kobj)
        return -ENOMEM;

    ret = sysfs_create_group(e404_kobj, &e404_attr_group);
    if (ret)
        kobject_put(e404_kobj);

    pr_alert("E404: Kernel Module & Sysfs Initialized\n");

    return ret;
}

static void __exit e404_exit(void) {
    kobject_put(e404_kobj);
    pr_alert("E404: Kernel Module & Sysfs Removed\n");
}

core_initcall(e404_init);
module_exit(e404_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kvsnr113");
MODULE_DESCRIPTION("E404 manager via early_param and sysfs");
MODULE_VERSION("1.4");