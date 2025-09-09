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

struct dp_debug *dp_debug_get(struct dp_debug_in *in)
{
	return 0;
}

void dp_debug_put(struct dp_debug *dp_debug)
{
}
