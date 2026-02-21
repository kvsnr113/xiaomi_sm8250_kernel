#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uidgid.h>
#include <linux/user_namespace.h>

#define UFS_MB_BUF_SIZE     32
#define LCD_INFO_BUF_SIZE   64

static char ufs_total_read_mb[UFS_MB_BUF_SIZE]  = "0\n";
static char ufs_total_write_mb[UFS_MB_BUF_SIZE] = "0\n";
static char lcd_panel_info[LCD_INFO_BUF_SIZE]   = "unknown\n";

static void proc_set_owner(struct proc_dir_entry *de) {
    if (de)
        proc_set_user(de, make_kuid(&init_user_ns, 0), make_kgid(&init_user_ns, 1000));
}

static ssize_t buffer_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos, const char *kbuf) {
    return simple_read_from_buffer(ubuf, count, ppos, kbuf, strlen(kbuf));
}
static ssize_t buffer_write(struct file *filp, const char __user *ubuf,  size_t count, loff_t *ppos, char *kbuf, size_t bufsize) {
    if (count >= bufsize)
        return -EINVAL;

    if (copy_from_user(kbuf, ubuf, count))
        return -EFAULT;

    kbuf[count] = '\0';
    return count;
}
static ssize_t ufs_read_mb_proc_read(struct file *file, char __user *buf,
                                     size_t count, loff_t *ppos)
{
    return buffer_read(file, buf, count, ppos, ufs_total_read_mb);
}

static ssize_t ufs_read_mb_proc_write(struct file *file, const char __user *buf,
                                      size_t count, loff_t *ppos)
{
    return buffer_write(file, buf, count, ppos,
                        ufs_total_read_mb, sizeof(ufs_total_read_mb));
}

static const struct proc_ops ufs_read_mb_proc_ops = {
    .proc_read  = ufs_read_mb_proc_read,
    .proc_write = ufs_read_mb_proc_write,
};

static ssize_t ufs_write_mb_proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    return buffer_read(file, buf, count, ppos, ufs_total_write_mb);
}
static ssize_t ufs_write_mb_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    return buffer_write(file, buf, count, ppos, ufs_total_write_mb, sizeof(ufs_total_write_mb));
}
static const struct proc_ops ufs_write_mb_proc_ops = {
    .proc_read  = ufs_write_mb_proc_read,
    .proc_write = ufs_write_mb_proc_write,
};
static ssize_t dummy_zero_read(struct file *file, char __user *buf,
                               size_t count, loff_t *ppos)
{
    static const char zero_str[] = "0\n";
    return simple_read_from_buffer(buf, count, ppos, zero_str, sizeof(zero_str) - 1);
}

static ssize_t dummy_sink_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    return count; 
}
static const struct proc_ops dummy_proc_ops = {
    .proc_read  = dummy_zero_read,
    .proc_write = dummy_sink_write,
};

static ssize_t lcd_info_read(struct file *file, char __user *buf,
                             size_t count, loff_t *ppos)
{
    return buffer_read(file, buf, count, ppos, lcd_panel_info);
}

static ssize_t lcd_info_write(struct file *file, const char __user *buf,
                              size_t count, loff_t *ppos)
{
    return buffer_write(file, buf, count, ppos,
                        lcd_panel_info, sizeof(lcd_panel_info));
}
static const struct proc_ops lcd_info_proc_ops = {
    .proc_read  = lcd_info_read,
    .proc_write = lcd_info_write,
};

static ssize_t lcd_s_read(struct file *file, char __user *buf,
                          size_t count, loff_t *ppos)
{
    static const char none_str[] = "none\n";
    return simple_read_from_buffer(buf, count, ppos, none_str, sizeof(none_str) - 1);
}

static const struct proc_ops lcd_s_proc_ops = {
    .proc_read  = lcd_s_read,
    .proc_write = dummy_sink_write,     // reuse dummy write
};

static ssize_t mutual_cmd_show(struct device *dev, struct device_attribute *attr, char *buf) {
    msleep(5000);
    return sprintf(buf, "0\n");
}

static ssize_t mutual_cmd_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    msleep(5000);
    return count; 
}
static DEVICE_ATTR_RW(mutual_cmd);

static struct class *oplus_chg_class;
static struct device *oplus_chg_common_dev;

static int __init oplus_stub_init(void)
{
    struct proc_dir_entry *storage_dir;
    struct proc_dir_entry *oplus_storage_dir;
    struct proc_dir_entry *io_metrics_dir;
    struct proc_dir_entry *forever_dir;
    struct proc_dir_entry *scheduler_dir;
    struct proc_dir_entry *sched_assist_dir;
    struct proc_dir_entry *afs_dir;
    struct proc_dir_entry *mem_dir;
    struct proc_dir_entry *devinfo_dir;
    proc_set_owner(proc_create("bootprof",          0666, NULL, &dummy_proc_ops));
    proc_set_owner(proc_create("theiaPwkReport",    0666, NULL, &dummy_proc_ops));

    storage_dir = proc_mkdir("storage", NULL);
    proc_set_owner(storage_dir);
    if (storage_dir)
        proc_set_owner(proc_create("buf_log", 0666, storage_dir, &dummy_proc_ops));

    oplus_storage_dir = proc_mkdir("oplus_storage", NULL);
    proc_set_owner(oplus_storage_dir);
    if (oplus_storage_dir) {
        io_metrics_dir = proc_mkdir("io_metrics", oplus_storage_dir);
        proc_set_owner(io_metrics_dir);
        if (io_metrics_dir) {
            forever_dir = proc_mkdir("forever", io_metrics_dir);
            proc_set_owner(forever_dir);
            if (forever_dir) {
                proc_set_owner(proc_create("ufs_total_read_size_mb",  0666, forever_dir, &ufs_read_mb_proc_ops));
                proc_set_owner(proc_create("ufs_total_write_size_mb", 0666, forever_dir, &ufs_write_mb_proc_ops));
            }
        }
    }

    scheduler_dir = proc_mkdir("oplus_scheduler", NULL);
    proc_set_owner(scheduler_dir);
    if (scheduler_dir) {
        sched_assist_dir = proc_mkdir("sched_assist", scheduler_dir);
        proc_set_owner(sched_assist_dir);
        if (sched_assist_dir) {
            proc_set_owner(proc_create("sched_assist_scene", 0666, sched_assist_dir, &dummy_proc_ops));
            proc_set_owner(proc_create("sched_impt_task", 0666, sched_assist_dir, &dummy_proc_ops));
            proc_set_owner(proc_create("im_flag_app", 0666, sched_assist_dir, &dummy_proc_ops));
            proc_set_owner(proc_create("im_flag", 0666, sched_assist_dir, &dummy_proc_ops));
            proc_set_owner(proc_create("ux_task", 0666, sched_assist_dir, &dummy_proc_ops));
        }
    }

    afs_dir = proc_mkdir("oplus_afs_config", NULL);
    proc_set_owner(afs_dir);
    if (afs_dir)
        proc_set_owner(proc_create("afs_config", 0666, afs_dir, &dummy_proc_ops));

    mem_dir = proc_mkdir("oplus_mem", NULL);
    proc_set_owner(mem_dir);
    if (mem_dir)
        proc_set_owner(proc_create("memory_monitor", 0666, mem_dir, &dummy_proc_ops));

    devinfo_dir = proc_mkdir("devinfo", NULL);
    proc_set_owner(devinfo_dir);
    if (devinfo_dir) {
        proc_set_owner(proc_create("lcd",   0666, devinfo_dir, &lcd_info_proc_ops));
        proc_set_owner(proc_create("lcd_s", 0666, devinfo_dir, &lcd_s_proc_ops));
    }

    oplus_chg_class = class_create(THIS_MODULE, "oplus_chg");
    if (IS_ERR(oplus_chg_class))
        return PTR_ERR(oplus_chg_class);

    oplus_chg_common_dev = device_create(oplus_chg_class, NULL, MKDEV(0,0), NULL, "common");
    if (IS_ERR(oplus_chg_common_dev)) {
        class_destroy(oplus_chg_class);
        return PTR_ERR(oplus_chg_common_dev);
    }

    device_create_file(oplus_chg_common_dev, &dev_attr_mutual_cmd);

    return 0;
}

static void __exit oplus_stub_exit(void)
{
    if (!IS_ERR_OR_NULL(oplus_chg_common_dev)) {
        device_remove_file(oplus_chg_common_dev, &dev_attr_mutual_cmd);
        device_destroy(oplus_chg_class, MKDEV(0,0));
    }
    if (!IS_ERR_OR_NULL(oplus_chg_class))
        class_destroy(oplus_chg_class);

}

module_init(oplus_stub_init);
module_exit(oplus_stub_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Danda");