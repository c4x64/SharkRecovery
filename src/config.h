/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef GSI_FIX_CONFIG_H
#define GSI_FIX_CONFIG_H

#include <stdint.h>

#define GSI_FIX_VERSION "1.0.0"
#define GSI_FIX_MAX_UIDS 16

/*
 * Runtime configuration.  These fields are written by the control callback
 * (process context) and read by the hooks.  They are plain aligned ints, so
 * aarch64 loads/stores are atomic; a toggle may take effect one permission
 * check late, which is harmless here.
 */
struct gsi_fix_config {
    int defex_enabled;
    int five_enabled;
    int selinux_targeted;
    int selinux_permissive;
    uint32_t uids[GSI_FIX_MAX_UIDS];
    int uid_count;
};

extern struct gsi_fix_config g_cfg;

void gsi_config_defaults(void);
int gsi_uid_allowed(uint32_t uid);
int gsi_uid_add(uint32_t uid);
int gsi_uid_del(uint32_t uid);
void gsi_uid_reset(void);
int gsi_parse_bool(const char *s, int *out);

#endif /* GSI_FIX_CONFIG_H */
