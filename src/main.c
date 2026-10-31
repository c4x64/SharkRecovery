/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * samsung-gsi-fix
 *
 * A KernelPatch Module (KPM) for Samsung phones running an AOSP GSI.
 * It removes kernel-side blockers that stop ported IMS/telephony userspace
 * from running: DEFEX, FIVE/PROCA integrity denials, and SELinux denials for
 * selected UIDs.
 *
 * It does NOT implement VoLTE or IMS.  Samsung's IMS stack is userspace and
 * proprietary; see docs/IMS.md for the userspace side.
 */

#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>

#include "config.h"
#include "hooks.h"
#include "util.h"

KPM_NAME("samsung-gsi-fix");
KPM_VERSION(GSI_FIX_VERSION);
KPM_LICENSE("GPL v2");
KPM_AUTHOR("c4x64");
KPM_DESCRIPTION("Samsung GSI enabler: DEFEX/FIVE bypass and targeted SELinux allow for IMS/radio");

static void gsi_parse_init_args(const char *args)
{
    char tok[64];

    if (!args) return;

    while (1) {
        int value;
        uint32_t uid;

        args = gsi_next_token(args, tok, sizeof(tok));
        if (!tok[0]) break;

        if (gsi_starts_with(tok, "defex=")) {
            if (!gsi_parse_bool(tok + 6, &value)) g_cfg.defex_enabled = value;
        } else if (gsi_starts_with(tok, "five=")) {
            if (!gsi_parse_bool(tok + 5, &value)) g_cfg.five_enabled = value;
        } else if (gsi_starts_with(tok, "selinux=")) {
            if (!gsi_parse_bool(tok + 8, &value)) g_cfg.selinux_targeted = value;
        } else if (gsi_starts_with(tok, "permissive=")) {
            if (!gsi_parse_bool(tok + 11, &value)) g_cfg.selinux_permissive = value;
        } else if (gsi_starts_with(tok, "uid=")) {
            if (!gsi_parse_uint(tok + 4, &uid)) gsi_uid_add(uid);
        } else {
            pr_info("[gsi-fix] ignoring init arg: %s\n", tok);
        }
    }
}

static long gsi_fix_init(const char *args, const char *event, void *__user reserved)
{
    (void)reserved;

    pr_info("[gsi-fix] init: event=%s args=%s\n",
            event ? event : "(none)", args ? args : "(none)");

    gsi_config_defaults();
    gsi_parse_init_args(args);
    gsi_hooks_install();

    pr_info("[gsi-fix] loaded; use ctl0 'status' for hook state\n");
    return 0;
}

static long gsi_fix_exit(void *reserved)
{
    (void)reserved;

    gsi_hooks_uninstall();
    pr_info("[gsi-fix] unloaded\n");
    return 0;
}

KPM_INIT(gsi_fix_init);
KPM_EXIT(gsi_fix_exit);
