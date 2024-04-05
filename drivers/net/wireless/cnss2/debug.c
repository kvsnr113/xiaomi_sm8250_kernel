// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2016-2020, The Linux Foundation. All rights reserved. */

#include <linux/err.h>
#include <linux/seq_file.h>
#include <linux/debugfs.h>
#include "main.h"
#include "bus.h"
#include "debug.h"
#include "pci.h"

#define MMIO_REG_ACCESS_MEM_TYPE		0xFF
#define HEX_DUMP_ROW_SIZE			16

void *cnss_ipc_log_context;
void *cnss_ipc_log_long_context;

static void cnss_print_hex_dump(const void *buf, int len)
{
}

static int cnss_pin_connect_show(struct seq_file *s, void *data)
{
	return 0;
}

static int cnss_pin_connect_open(struct inode *inode, struct file *file)
{
	return single_open(file, cnss_pin_connect_show, inode->i_private);
}

static const struct file_operations cnss_pin_connect_fops = {
};

static int cnss_stats_show_state(struct seq_file *s,
				 struct cnss_plat_data *plat_priv)
{
	return 0;
}

static int cnss_stats_show(struct seq_file *s, void *data)
{
	struct cnss_plat_data *plat_priv = s->private;

	cnss_stats_show_state(s, plat_priv);

	return 0;
}

static int cnss_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, cnss_stats_show, inode->i_private);
}

static const struct file_operations cnss_stats_fops = {
};

static ssize_t cnss_dev_boot_debug_write(struct file *fp,
					 const char __user *user_buf,
					 size_t count, loff_t *off)
{
	return 0;
}

static int cnss_dev_boot_debug_show(struct seq_file *s, void *data)
{
	return 0;
}

static int cnss_dev_boot_debug_open(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations cnss_dev_boot_debug_fops = {
};

static int cnss_reg_read_debug_show(struct seq_file *s, void *data)
{
	return 0;
}

static ssize_t cnss_reg_read_debug_write(struct file *fp,
					 const char __user *user_buf,
					 size_t count, loff_t *off)
{
	return 0;
}

static int cnss_reg_read_debug_open(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations cnss_reg_read_debug_fops = {
};

static int cnss_reg_write_debug_show(struct seq_file *s, void *data)
{
	return 0;
}

static ssize_t cnss_reg_write_debug_write(struct file *fp,
					  const char __user *user_buf,
					  size_t count, loff_t *off)
{
	return 0;
}

static int cnss_reg_write_debug_open(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations cnss_reg_write_debug_fops = {
};

static ssize_t cnss_runtime_pm_debug_write(struct file *fp,
					   const char __user *user_buf,
					   size_t count, loff_t *off)
{
	return 0;
}

static int cnss_runtime_pm_debug_show(struct seq_file *s, void *data)
{
	return 0;
}

static int cnss_runtime_pm_debug_open(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations cnss_runtime_pm_debug_fops = {
};

static ssize_t cnss_control_params_debug_write(struct file *fp,
					       const char __user *user_buf,
					       size_t count, loff_t *off)
{
	return 0;
}

static int cnss_show_quirks_state(struct seq_file *s,
				  struct cnss_plat_data *plat_priv)
{
	return 0;
}

static int cnss_control_params_debug_show(struct seq_file *s, void *data)
{
	struct cnss_plat_data *cnss_priv = s->private;

	seq_puts(s, "\nUsage: echo <params_name> <value> > <debugfs_path>/cnss/control_params\n");
	seq_puts(s, "<params_name> can be one of below:\n");
	seq_puts(s, "quirks: Debug quirks for driver\n");
	seq_puts(s, "mhi_timeout: Timeout for MHI operation in milliseconds\n");
	seq_puts(s, "qmi_timeout: Timeout for QMI message in milliseconds\n");
	seq_puts(s, "bdf_type: Type of board data file to be downloaded\n");
	seq_puts(s, "time_sync_period: Time period to do time sync with device in milliseconds\n");

	seq_puts(s, "\nCurrent value:\n");
	cnss_show_quirks_state(s, cnss_priv);
	seq_printf(s, "mhi_timeout: %u\n", cnss_priv->ctrl_params.mhi_timeout);
	seq_printf(s, "mhi_m2_timeout: %u\n",
		   cnss_priv->ctrl_params.mhi_m2_timeout);
	seq_printf(s, "qmi_timeout: %u\n", cnss_priv->ctrl_params.qmi_timeout);
	seq_printf(s, "bdf_type: %u\n", cnss_priv->ctrl_params.bdf_type);
	seq_printf(s, "time_sync_period: %u\n",
		   cnss_priv->ctrl_params.time_sync_period);

	return 0;
}

static int cnss_control_params_debug_open(struct inode *inode,
					  struct file *file)
{
	return single_open(file, cnss_control_params_debug_show,
			   inode->i_private);
}

static const struct file_operations cnss_control_params_debug_fops = {
	.read = seq_read,
	.write = cnss_control_params_debug_write,
	.open = cnss_control_params_debug_open,
	.owner = THIS_MODULE,
	.llseek = seq_lseek,
};

static ssize_t cnss_dynamic_feature_write(struct file *fp,
					  const char __user *user_buf,
					  size_t count, loff_t *off)
{
	struct cnss_plat_data *plat_priv =
		((struct seq_file *)fp->private_data)->private;
	int ret = 0;
	u64 val;

	ret = kstrtou64_from_user(user_buf, count, 0, &val);
	if (ret)
		return ret;

	plat_priv->dynamic_feature = val;
	ret = cnss_wlfw_dynamic_feature_mask_send_sync(plat_priv);
	if (ret < 0)
		return ret;

	return count;
}

static int cnss_dynamic_feature_show(struct seq_file *s, void *data)
{
	struct cnss_plat_data *cnss_priv = s->private;

	seq_printf(s, "dynamic_feature: 0x%llx\n", cnss_priv->dynamic_feature);

	return 0;
}

static int cnss_dynamic_feature_open(struct inode *inode,
				     struct file *file)
{
	return single_open(file, cnss_dynamic_feature_show,
			   inode->i_private);
}

static const struct file_operations cnss_dynamic_feature_fops = {
	.read = seq_read,
	.write = cnss_dynamic_feature_write,
	.open = cnss_dynamic_feature_open,
	.owner = THIS_MODULE,
	.llseek = seq_lseek,
};

static int cnss_wfc_call_status_debug_show(struct seq_file *s, void *data)
{
	seq_puts(s, "\nUsage: echo <data_len> <hex data> > <debugfs_path>/cnss/wfc_call_status\n");
	seq_puts(s, "e.g. Send 4 bytes of hex data for WFC call status using QMI message:\n");
	seq_puts(s, "echo '0x4 0xA 0xB 0xC 0xD' > /d/cnss/wfc_call_status\n");

	return 0;
}

static ssize_t cnss_wfc_call_status_debug_write(struct file *fp,
						const char __user *user_buf,
						size_t count, loff_t *off)
{
	struct cnss_plat_data *plat_priv =
		((struct seq_file *)fp->private_data)->private;
	char buf[(QMI_WLFW_MAX_WFC_CALL_STATUS_DATA_SIZE_V01 + 1) * 5];
	char *sptr, *token;
	unsigned int len = 0;
	const char *delim = " ";
	u32 data_len;
	u8 data[QMI_WLFW_MAX_WFC_CALL_STATUS_DATA_SIZE_V01] = {0}, data_byte;
	int ret = 0, i;

	if (!test_bit(CNSS_FW_READY, &plat_priv->driver_state)) {
		cnss_pr_err("Firmware is not ready yet\n");
		return -EINVAL;
	}

	len = min(count, sizeof(buf) - 1);
	if (copy_from_user(buf, user_buf, len))
		return -EFAULT;

	buf[len] = '\0';
	sptr = buf;

	token = strsep(&sptr, delim);
	if (!token || !sptr)
		return -EINVAL;

	if (kstrtou32(token, 0, &data_len))
		return -EINVAL;

	cnss_pr_dbg("Parsing 0x%x bytes data for WFC call status\n", data_len);

	if (data_len > QMI_WLFW_MAX_WFC_CALL_STATUS_DATA_SIZE_V01 ||
	    data_len == 0) {
		cnss_pr_err("Invalid data length 0x%x\n", data_len);
		return -EINVAL;
	}

	for (i = 0; i < data_len; i++) {
		token = strsep(&sptr, delim);
		if (!token || (!sptr && i < data_len - 1)) {
			cnss_pr_err("Input data is less than length\n");
			return -EINVAL;
		}

		if (kstrtou8(token, 0, &data_byte)) {
			cnss_pr_err("Data format is incorrect\n");
			return -EINVAL;
		}

		data[i] = data_byte;
	}

	cnss_print_hex_dump(data, data_len);

	ret = cnss_wlfw_wfc_call_status_send_sync(plat_priv, data_len, data);
	if (ret)
		return ret;

	return count;
}

static int cnss_wfc_call_status_debug_open(struct inode *inode,
					   struct file *file)
{
	return single_open(file, cnss_wfc_call_status_debug_show,
			   inode->i_private);
}

static const struct file_operations cnss_wfc_call_status_debug_fops = {
	.read		= seq_read,
	.write		= cnss_wfc_call_status_debug_write,
	.open		= cnss_wfc_call_status_debug_open,
	.owner		= THIS_MODULE,
	.llseek		= seq_lseek,
};

#ifdef CONFIG_CNSS2_DEBUG
static int cnss_create_debug_only_node(struct cnss_plat_data *plat_priv)
{
	struct dentry *root_dentry = plat_priv->root_dentry;

	debugfs_create_file("dev_boot", 0600, root_dentry, plat_priv,
			    &cnss_dev_boot_debug_fops);
	debugfs_create_file("reg_read", 0600, root_dentry, plat_priv,
			    &cnss_reg_read_debug_fops);
	debugfs_create_file("reg_write", 0600, root_dentry, plat_priv,
			    &cnss_reg_write_debug_fops);
	debugfs_create_file("runtime_pm", 0600, root_dentry, plat_priv,
			    &cnss_runtime_pm_debug_fops);
	debugfs_create_file("control_params", 0600, root_dentry, plat_priv,
			    &cnss_control_params_debug_fops);
	debugfs_create_file("dynamic_feature", 0600, root_dentry, plat_priv,
			    &cnss_dynamic_feature_fops);
	debugfs_create_file("wfc_call_status", 0600, root_dentry, plat_priv,
			    &cnss_wfc_call_status_debug_fops);

	return 0;
}

int cnss_debugfs_create(struct cnss_plat_data *plat_priv)
{
	int ret = 0;
	struct dentry *root_dentry;

	root_dentry = debugfs_create_dir("cnss", 0);
	if (IS_ERR(root_dentry)) {
		ret = PTR_ERR(root_dentry);
		cnss_pr_err("Unable to create debugfs %d\n", ret);
		goto out;
	}

	plat_priv->root_dentry = root_dentry;

	debugfs_create_file("pin_connect_result", 0644, root_dentry, plat_priv,
			    &cnss_pin_connect_fops);
	debugfs_create_file("stats", 0644, root_dentry, plat_priv,
			    &cnss_stats_fops);

	cnss_create_debug_only_node(plat_priv);

out:
	return ret;
}

void cnss_debugfs_destroy(struct cnss_plat_data *plat_priv)
{
	debugfs_remove_recursive(plat_priv->root_dentry);
}
#else
int cnss_debugfs_create(struct cnss_plat_data *plat_priv)
{
	return 0;
}

void cnss_debugfs_destroy(struct cnss_plat_data *plat_priv)
{
}
#endif

int cnss_debug_init(void)
{
	cnss_ipc_log_context = ipc_log_context_create(CNSS_IPC_LOG_PAGES,
						      "cnss", 0);
	if (!cnss_ipc_log_context) {
		cnss_pr_err("Unable to create IPC log context\n");
		return -EINVAL;
	}

	cnss_ipc_log_long_context = ipc_log_context_create(CNSS_IPC_LOG_PAGES,
							   "cnss-long", 0);
	if (!cnss_ipc_log_long_context) {
		cnss_pr_err("Unable to create IPC long log context\n");
		ipc_log_context_destroy(cnss_ipc_log_context);
		return -EINVAL;
	}

	return 0;
}

void cnss_debug_deinit(void)
{
	if (cnss_ipc_log_long_context) {
		ipc_log_context_destroy(cnss_ipc_log_long_context);
		cnss_ipc_log_long_context = NULL;
	}

	if (cnss_ipc_log_context) {
		ipc_log_context_destroy(cnss_ipc_log_context);
		cnss_ipc_log_context = NULL;
	}
}
