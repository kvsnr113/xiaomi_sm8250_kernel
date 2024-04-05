// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/debugfs.h>
#include <linux/slab.h>

#include "dp_power.h"
#include "dp_catalog.h"
#include "dp_aux.h"
#include "dp_debug.h"
#include "drm_connector.h"
#include "sde_connector.h"
#include "dp_display.h"

#define DEBUG_NAME "drm_dp"

struct dp_debug_private {
};

static int dp_debug_get_edid_buf(struct dp_debug_private *debug)
{
	return 0;
}

static int dp_debug_get_dpcd_buf(struct dp_debug_private *debug)
{
	return 0;
}

static ssize_t dp_debug_write_edid(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_dpcd(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_read_dpcd(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_hpd(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_edid_modes(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_edid_modes_mst(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_mst_con_id(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_mst_con_add(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_mst_con_remove(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_bw_code_write(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_mst_mode_write(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_max_pclk_khz_write(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_max_pclk_khz_read(struct file *file,
	char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_mst_sideband_mode_write(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_widebus_mode_write(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_tpg_write(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_exe_mode(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_read_connected(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_hdcp(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_read_hdcp(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static int dp_debug_check_buffer_overflow(int rc, int *max_size, int *len)
{
	return 0;
}

static ssize_t dp_debug_read_edid_modes(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_read_edid_modes_mst(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_read_mst_con_id(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_read_mst_conn_info(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_read_info(struct file *file, char __user *user_buff,
		size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_bw_code_read(struct file *file,
	char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_tpg_read(struct file *file,
	char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static int dp_debug_print_hdr_params_to_buf(struct drm_connector *connector,
		char *buf, u32 size)
{
	return 0;
}

static ssize_t dp_debug_read_hdr(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_read_hdr_mst(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static void dp_debug_set_sim_mode(struct dp_debug_private *debug, bool sim)
{
}

static ssize_t dp_debug_write_sim(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_attention(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_write_dump(struct file *file,
		const char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t dp_debug_read_dump(struct file *file,
		char __user *user_buff, size_t count, loff_t *ppos)
{
	return 0;
}

static const struct file_operations dp_debug_fops = {
};

static const struct file_operations edid_modes_fops = {
};

static const struct file_operations edid_modes_mst_fops = {
};

static const struct file_operations mst_conn_info_fops = {
};

static const struct file_operations mst_con_id_fops = {
};

static const struct file_operations mst_con_add_fops = {
};

static const struct file_operations mst_con_remove_fops = {
};

static const struct file_operations hpd_fops = {
};

static const struct file_operations edid_fops = {
};

static const struct file_operations dpcd_fops = {
};

static const struct file_operations connected_fops = {
};

static const struct file_operations bw_code_fops = {
};
static const struct file_operations exe_mode_fops = {
};

static const struct file_operations tpg_fops = {
};

static const struct file_operations hdr_fops = {
};

static const struct file_operations hdr_mst_fops = {
};

static const struct file_operations sim_fops = {
};

static const struct file_operations attention_fops = {
};

static const struct file_operations dump_fops = {
};

static const struct file_operations mst_mode_fops = {
};

static const struct file_operations mst_sideband_mode_fops = {
};

static const struct file_operations max_pclk_khz_fops = {
};

static const struct file_operations hdcp_fops = {
};

static const struct file_operations widebus_mode_fops = {
};

static int dp_debug_init(struct dp_debug *dp_debug)
{
	return 0;
}

u8 *dp_debug_get_edid(struct dp_debug *dp_debug)
{
	return 0;
}

static void dp_debug_abort(struct dp_debug *dp_debug)
{
}

struct dp_debug *dp_debug_get(struct dp_debug_in *in)
{
	return 0;
}

static int dp_debug_deinit(struct dp_debug *dp_debug)
{
	return 0;
}

void dp_debug_put(struct dp_debug *dp_debug)
{
}
