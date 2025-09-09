// E404 Manager by Project 113 (kvsnr113)

#include <linux/e404_attributes.h>

struct e404_attributes e404_data = {
    .e404_kernelsu = 0,
    .e404_effcpu = 0,
    .e404_rom_type = 1,
    .e404_dtbo_type = 1,
    .e404_batt_profile = 1,
    .e404_dvq_input_boost = 1,
    .e404_kgsl_skip_zeroing = 0,
    .e404_panel_width = 70,
    .e404_panel_height = 155,
    .e404_oem_panel_width = 700,
    .e404_oem_panel_height = 1550
};

static struct kobject *e404_kobj;

#ifdef CONFIG_E404_KSU_DEFAULT
int e404_early_kernelsu = 1;
#else
int e404_early_kernelsu = 0;
#endif

#ifdef CONFIG_E404_EFFCPU_DEFAULT
int e404_early_effcpu = 1;
#else
int e404_early_effcpu = 0;
#endif

#ifdef CONFIG_E404_OPLUS
int e404_early_rom_type = 3;
#elif defined(CONFIG_E404_MIUI)
int e404_early_rom_type = 2;
#else
int e404_early_rom_type = 1;
#endif

#ifdef CONFIG_E404_MIUI
int e404_early_dtbo_type = 2;
#else
int e404_early_dtbo_type = 1;
#endif

#ifdef CONFIG_E404_ALIOTH_5K_BATT_DEFAULT
int e404_early_batt_profile = 2;
#else
int e404_early_batt_profile = 1;
#endif

static int __init parse_e404_args(char *str)
{
    char *arg;

    while ((arg = strsep(&str, " ,")) != NULL) {
        if (!*arg) continue;

        pr_alert("E404: Parsing flag: %s\n", arg);

        if (strcmp(arg, "root_ksu") == 0)
            e404_early_kernelsu = 1;
        else if (strcmp(arg, "root_noksu") == 0)
            e404_early_kernelsu = 0;
        else if (strcmp(arg, "dtb_effcpu") == 0)
            e404_early_effcpu = 1;
        else if (strcmp(arg, "dtb_def") == 0)
            e404_early_effcpu = 0;
        else if (strcmp(arg, "port") == 0)
            e404_early_rom_type = 3;
        else if (strcmp(arg, "miui") == 0)
            e404_early_rom_type = 2;
        else if (strcmp(arg, "aosp") == 0)
            e404_early_rom_type = 1;
        else if (strcmp(arg, "dtbo_def") == 0)
            e404_early_dtbo_type = 1;
        else if (strcmp(arg, "dtbo_oem") == 0)
            e404_early_dtbo_type = 2;
        else if (strcmp(arg, "batt_def") == 0)
            e404_early_batt_profile = 1;
        else if (strcmp(arg, "batt_5k") == 0)
            e404_early_batt_profile = 2;
        else
            pr_alert("E404: Unknown flag: %s\n", arg);
    }

    return 0;
}
early_param("e404_args", parse_e404_args);

static void e404_parse_attributes(void) {
    e404_data.e404_kernelsu = e404_early_kernelsu;
    e404_data.e404_effcpu = e404_early_effcpu;
    e404_data.e404_rom_type = e404_early_rom_type;
    e404_data.e404_dtbo_type = e404_early_dtbo_type;
    e404_data.e404_batt_profile = e404_early_batt_profile;

    pr_alert("E404 Early Attributes: KernelSU=%d, EFFCPU=%d, RomType=%d, DTBOType=%d, BatteryProfile=%d\n",
        e404_data.e404_kernelsu,
        e404_data.e404_effcpu,
        e404_data.e404_rom_type,
        e404_data.e404_dtbo_type,
        e404_data.e404_batt_profile);
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

E404_ATTR_RO(e404_kernelsu);
E404_ATTR_RO(e404_effcpu);
E404_ATTR_RO(e404_rom_type);
E404_ATTR_RO(e404_dtbo_type);
E404_ATTR_RO(e404_batt_profile);
E404_ATTR_RO(e404_panel_width);
E404_ATTR_RO(e404_panel_height);
E404_ATTR_RO(e404_oem_panel_width);
E404_ATTR_RO(e404_oem_panel_height);

E404_ATTR_RW(e404_dvq_input_boost);
E404_ATTR_RW(e404_kgsl_skip_zeroing);

static struct attribute *e404_attrs[] = {
    &e404_kernelsu_attr.attr,
    &e404_effcpu_attr.attr,
    &e404_rom_type_attr.attr,
    &e404_dtbo_type_attr.attr,
    &e404_batt_profile_attr.attr,
    &e404_dvq_input_boost_attr.attr,
    &e404_kgsl_skip_zeroing_attr.attr,
    &e404_panel_width_attr.attr,
    &e404_panel_height_attr.attr,
    &e404_oem_panel_width_attr.attr,
    &e404_oem_panel_height_attr.attr,
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
MODULE_VERSION("1.3");