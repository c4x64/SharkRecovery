/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "config.h"
#include "util.h"

struct gsi_fix_config g_cfg;

static const uint32_t gsi_default_uids[] = {
    1000, /* AID_SYSTEM */
    1001, /* AID_RADIO / telephony */
};

void gsi_config_defaults(void)
{
    g_cfg.defex_enabled = 1;
    g_cfg.five_enabled = 1;
    g_cfg.selinux_targeted = 1;
    g_cfg.selinux_permissive = 0;
    gsi_uid_reset();
}

void gsi_uid_reset(void)
{
    int i;

    g_cfg.uid_count = 0;
    for (i = 0; i < (int)(sizeof(gsi_default_uids) / sizeof(gsi_default_uids[0])); i++) {
        g_cfg.uids[g_cfg.uid_count++] = gsi_default_uids[i];
    }
}

int gsi_uid_allowed(uint32_t uid)
{
    int i;

    for (i = 0; i < g_cfg.uid_count; i++) {
        if (g_cfg.uids[i] == uid) return 1;
    }
    return 0;
}

int gsi_uid_add(uint32_t uid)
{
    if (gsi_uid_allowed(uid)) return 0;
    if (g_cfg.uid_count >= GSI_FIX_MAX_UIDS) return -1;
    g_cfg.uids[g_cfg.uid_count] = uid;
    g_cfg.uid_count++;
    return 0;
}

int gsi_uid_del(uint32_t uid)
{
    int i;

    for (i = 0; i < g_cfg.uid_count; i++) {
        if (g_cfg.uids[i] != uid) continue;

        for (; i + 1 < g_cfg.uid_count; i++) {
            g_cfg.uids[i] = g_cfg.uids[i + 1];
        }
        g_cfg.uid_count--;
        return 0;
    }
    return -1;
}

int gsi_parse_bool(const char *s, int *out)
{
    if (!s || !out) return -1;

    if (!gsi_strcmp(s, "on") || !gsi_strcmp(s, "1") ||
        !gsi_strcmp(s, "yes") || !gsi_strcmp(s, "true")) {
        *out = 1;
        return 0;
    }
    if (!gsi_strcmp(s, "off") || !gsi_strcmp(s, "0") ||
        !gsi_strcmp(s, "no") || !gsi_strcmp(s, "false")) {
        *out = 0;
        return 0;
    }
    return -1;
}
