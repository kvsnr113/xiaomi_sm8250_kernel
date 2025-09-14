#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/irq.h>

#ifdef CONFIG_E404_SIGNATURE
#include <linux/e404_attributes.h>
#endif

#define linkdir "oemports10t"
#define link_file "oemports10t/danda"
#define link_source "/build.prop"

static int __init oemports10t_init(void) {
	char *driver_path;
	static struct proc_dir_entry *link_dir;
	static struct proc_dir_entry *link_t;
	int ret = 0;

#ifdef CONFIG_E404_SIGNATURE
	if (e404_data.rom_type != 3) {
		pr_alert("E404: Skipping oplus link init\n");
		return 0;
	}
#endif

	pr_alert("E404: Init oplus link\n");
	
	link_dir = proc_mkdir(linkdir, NULL);
	driver_path = kzalloc(PATH_MAX, GFP_KERNEL);
	if (!driver_path) {
		pr_err("%s: failed to allocate memory\n", __func__);
		ret = -ENOMEM;
	}

	sprintf(driver_path, "/vendor%s", link_source);
	pr_err("%s: driver_path=%s\n", __func__, driver_path);
	link_t = proc_symlink(link_file, NULL, driver_path);
	if (!link_t) {
		ret = -ENOMEM;
		printk(KERN_INFO "link failed");
	} else {
		printk(KERN_INFO "link initialized");
	}

	return ret;
}

static void __exit oemports10t_exit(void) {
	printk(KERN_INFO "link exit");
}

module_init(oemports10t_init);
module_exit(oemports10t_exit);