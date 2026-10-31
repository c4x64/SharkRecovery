/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <compiler.h>
#include <kpmodule.h>
#include <kputils.h>
#include <linux/printk.h>
#include <uapi/asm-generic/errno.h>

#include "config.h"
#include "hooks.h"
#include "util.h"

#define GSI_REPLY_MAX 1024

static long gsi_reply(char *__user out_msg, int outlen, const char *text)
{
    size_t len;

    if (!out_msg || outlen <= 0 || !text) return -EINVAL;

    len = gsi_strlen(text);
    if (len + 1 > (size_t)outlen) len = (size_t)outlen - 1;

    if (len && compat_copy_to_user(out_msg, text, (int)len)) return -EFAULT;
    if (compat_copy_to_user(out_msg + len, "", 1)) return -EFAULT;
    return 0;
}

static void append_hook_state(char *buf, size_t size, const char *name)
{
    int i;

    for (i = 0; i < g_hook_count; i++) {
        if (gsi_strcmp(g_hooks[i].name, name)) continue;

        if (!g_hooks[i].addr) {
            gsi_strlcat(buf, "missing", size);
        } else if (g_hooks[i].err) {
            gsi_strlcat(buf, "fail", size);
        } else {
            gsi_strlcat(buf, "ok", size);
        }
        return;
    }
    gsi_strlcat(buf, "n/a", size);
}

static long cmd_status(char *__user out_msg, int outlen)
{
    char buf[GSI_REPLY_MAX];
    int i;

    gsi_strlcpy(buf, "samsung-gsi-fix " GSI_FIX_VERSION "\n", sizeof(buf));

    gsi_strlcat(buf, "defex: ", sizeof(buf));
    gsi_strlcat(buf, g_cfg.defex_enabled ? "on" : "off", sizeof(buf));
    gsi_strlcat(buf, ", hook=", sizeof(buf));
    append_hook_state(buf, sizeof(buf), "task_defex_enforce");
    gsi_strlcat(buf, "\n", sizeof(buf));

    gsi_strlcat(buf, "five: ", sizeof(buf));
    gsi_strlcat(buf, g_cfg.five_enabled ? "on" : "off", sizeof(buf));
    gsi_strlcat(buf, ", bprm=", sizeof(buf));
    append_hook_state(buf, sizeof(buf), "five_bprm_check");
    gsi_strlcat(buf, " mmap=", sizeof(buf));
    append_hook_state(buf, sizeof(buf), "five_file_mmap");
    gsi_strlcat(buf, " verify=", sizeof(buf));
    append_hook_state(buf, sizeof(buf), "five_file_verify");
    gsi_strlcat(buf, "\n", sizeof(buf));

    gsi_strlcat(buf, "selinux: targeted=", sizeof(buf));
    gsi_strlcat(buf, g_cfg.selinux_targeted ? "on" : "off", sizeof(buf));
    gsi_strlcat(buf, " permissive=", sizeof(buf));
    gsi_strlcat(buf, g_cfg.selinux_permissive ? "on" : "off", sizeof(buf));
    gsi_strlcat(buf, ", avc_has_perm=", sizeof(buf));
    append_hook_state(buf, sizeof(buf), "avc_has_perm");
    gsi_strlcat(buf, " flags=", sizeof(buf));
    append_hook_state(buf, sizeof(buf), "avc_has_perm_flags");
    gsi_strlcat(buf, " xperms=", sizeof(buf));
    append_hook_state(buf, sizeof(buf), "avc_has_extended_perms");
    gsi_strlcat(buf, "\n", sizeof(buf));

    gsi_strlcat(buf, "uids:", sizeof(buf));
    for (i = 0; i < g_cfg.uid_count; i++) {
        gsi_strlcat(buf, " ", sizeof(buf));
        gsi_append_uint(buf, sizeof(buf), g_cfg.uids[i]);
    }
    gsi_strlcat(buf, "\n", sizeof(buf));

    gsi_strlcat(buf, "note: VoLTE/SMS still need a userspace IMS stack; "
                     "see docs/IMS.md\n", sizeof(buf));

    return gsi_reply(out_msg, outlen, buf);
}

static long cmd_defex(const char *arg, char *__user out_msg, int outlen)
{
    int value;

    if (gsi_parse_bool(arg, &value)) {
        return gsi_reply(out_msg, outlen, "usage: defex on|off");
    }
    g_cfg.defex_enabled = value;
    return gsi_reply(out_msg, outlen, value ? "defex=on" : "defex=off");
}

static long cmd_five(const char *arg, char *__user out_msg, int outlen)
{
    int value;

    if (gsi_parse_bool(arg, &value)) {
        return gsi_reply(out_msg, outlen, "usage: five on|off");
    }
    g_cfg.five_enabled = value;
    return gsi_reply(out_msg, outlen, value ? "five=on" : "five=off");
}

static long cmd_selinux(const char *args, char *__user out_msg, int outlen)
{
    char sub[32];
    char value_s[32];
    int value;

    args = gsi_next_token(args, sub, sizeof(sub));
    if (!sub[0]) {
        char buf[96];
        gsi_strlcpy(buf, "selinux: targeted=", sizeof(buf));
        gsi_strlcat(buf, g_cfg.selinux_targeted ? "on" : "off", sizeof(buf));
        gsi_strlcat(buf, " permissive=", sizeof(buf));
        gsi_strlcat(buf, g_cfg.selinux_permissive ? "on" : "off", sizeof(buf));
        return gsi_reply(out_msg, outlen, buf);
    }

    gsi_next_token(args, value_s, sizeof(value_s));

    if (!gsi_strcmp(sub, "targeted")) {
        if (gsi_parse_bool(value_s, &value)) {
            return gsi_reply(out_msg, outlen, "usage: selinux targeted on|off");
        }
        g_cfg.selinux_targeted = value;
        return gsi_reply(out_msg, outlen, value ? "selinux targeted=on"
                                                : "selinux targeted=off");
    }

    if (!gsi_strcmp(sub, "permissive") || !gsi_strcmp(sub, "global")) {
        if (gsi_parse_bool(value_s, &value)) {
            return gsi_reply(out_msg, outlen, "usage: selinux permissive on|off");
        }
        g_cfg.selinux_permissive = value;
        return gsi_reply(out_msg, outlen, value ? "selinux permissive=on"
                                                : "selinux permissive=off");
    }

    /* Shortcut: "selinux on" / "selinux off" means global permissive mode. */
    if (!gsi_strcmp(sub, "on") || !gsi_strcmp(sub, "off")) {
        if (gsi_parse_bool(sub, &value)) {
            return gsi_reply(out_msg, outlen, "usage: selinux on|off");
        }
        g_cfg.selinux_permissive = value;
        return gsi_reply(out_msg, outlen, value ? "selinux permissive=on"
                                                : "selinux permissive=off");
    }

    return gsi_reply(out_msg, outlen, "usage: selinux targeted|permissive on|off");
}

static long reply_uid(char *__user out_msg, int outlen, const char *prefix,
                      uint32_t uid)
{
    char buf[64];

    gsi_strlcpy(buf, prefix, sizeof(buf));
    gsi_append_uint(buf, sizeof(buf), uid);
    return gsi_reply(out_msg, outlen, buf);
}

static long cmd_uid(const char *args, char *__user out_msg, int outlen)
{
    char sub[32];
    char uid_s[32];
    uint32_t uid;

    args = gsi_next_token(args, sub, sizeof(sub));
    if (!sub[0] || !gsi_strcmp(sub, "list")) {
        char buf[GSI_REPLY_MAX];
        int i;

        gsi_strlcpy(buf, "uids:", sizeof(buf));
        for (i = 0; i < g_cfg.uid_count; i++) {
            gsi_strlcat(buf, " ", sizeof(buf));
            gsi_append_uint(buf, sizeof(buf), g_cfg.uids[i]);
        }
        return gsi_reply(out_msg, outlen, buf);
    }

    if (!gsi_strcmp(sub, "reset")) {
        gsi_uid_reset();
        return gsi_reply(out_msg, outlen, "uids reset to 1000 1001");
    }

    if (gsi_strcmp(sub, "add") && gsi_strcmp(sub, "del")) {
        return gsi_reply(out_msg, outlen,
                         "usage: uid add|del <uid> | uid list | uid reset");
    }

    gsi_next_token(args, uid_s, sizeof(uid_s));
    if (gsi_parse_uint(uid_s, &uid)) {
        return gsi_reply(out_msg, outlen, "usage: uid add|del <uid>");
    }

    if (!gsi_strcmp(sub, "add")) {
        if (gsi_uid_allowed(uid)) return reply_uid(out_msg, outlen, "already present: ", uid);
        if (gsi_uid_add(uid)) return gsi_reply(out_msg, outlen, "uid table full");
        return reply_uid(out_msg, outlen, "uid added: ", uid);
    }

    if (gsi_uid_del(uid)) return reply_uid(out_msg, outlen, "not present: ", uid);
    return reply_uid(out_msg, outlen, "uid removed: ", uid);
}

static long cmd_help(char *__user out_msg, int outlen)
{
    return gsi_reply(out_msg, outlen,
                     "samsung-gsi-fix commands:\n"
                     "  status\n"
                     "  defex on|off\n"
                     "  five on|off            (proca is an alias)\n"
                     "  selinux targeted on|off\n"
                     "  selinux permissive on|off\n"
                     "  uid add <uid> | del <uid> | list | reset\n");
}

static long gsi_fix_ctl0(const char *args, char *__user out_msg, int outlen)
{
    char cmd[32];
    const char *rest;

    if (!args) args = "";
    rest = gsi_next_token(args, cmd, sizeof(cmd));

    if (!cmd[0]) {
        return gsi_reply(out_msg, outlen, "empty command; try help");
    }
    if (!gsi_strcmp(cmd, "help")) return cmd_help(out_msg, outlen);
    if (!gsi_strcmp(cmd, "status")) return cmd_status(out_msg, outlen);
    if (!gsi_strcmp(cmd, "defex")) return cmd_defex(rest, out_msg, outlen);
    if (!gsi_strcmp(cmd, "five") || !gsi_strcmp(cmd, "proca")) {
        return cmd_five(rest, out_msg, outlen);
    }
    if (!gsi_strcmp(cmd, "selinux")) return cmd_selinux(rest, out_msg, outlen);
    if (!gsi_strcmp(cmd, "uid")) return cmd_uid(rest, out_msg, outlen);

    pr_info("[gsi-fix] unknown ctl0 command: %s\n", cmd);
    return gsi_reply(out_msg, outlen, "unknown command; try help");
}

KPM_CTL0(gsi_fix_ctl0);
