/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * samsung-gsi-fix: kernel-side enablers for Samsung devices running a GSI.
 *
 * The hooks in this file are deliberately "allow" hooks:
 *   - DEFEX is skipped before it runs, because task_defex_enforce() can kill
 *     the calling process as a side effect.
 *   - FIVE is allowed after it runs, because its bookkeeping (task integrity
 *     accounting) is still useful and only the deny result needs overriding.
 *   - SELinux permission checks are skipped for configured UIDs (default:
 *     system + radio), so IMS/telephony userspace does not need a globally
 *     permissive kernel.
 *
 * Every symbol is looked up at load time.  Missing symbols are not fatal:
 * stock MediaTek Samsung kernels, for example, often have no DEFEX/FIVE.
 */

#include <compiler.h>
#include <kpmodule.h>
#include <hook.h>
#include <kallsyms.h>
#include <kputils.h>
#include <linux/printk.h>

#include "config.h"
#include "hooks.h"
#include "util.h"

struct gsi_hook_info g_hooks[GSI_FIX_MAX_HOOKS];
int g_hook_count;

#define GSI_HOOK_MISSING (-2)

static unsigned long gsi_lookup_symbol(const char *name)
{
    unsigned long addr;
    char cfi_name[64];

    addr = kallsyms_lookup_name(name);
    if (addr) return addr;

    /* CFI-enabled kernels expose the callable entry as <name>.cfi_jt. */
    gsi_strlcpy(cfi_name, name, sizeof(cfi_name));
    gsi_strlcat(cfi_name, ".cfi_jt", sizeof(cfi_name));
    return kallsyms_lookup_name(cfi_name);
}

/*
 * DEFEX: return DEFEX_ALLOW without running the enforcement body.
 *
 * task_defex_enforce() is variadic: the trusted-map feature reads up to two
 * extra arguments (bprm on >= 5.9, or argc + argv on older kernels). Use argno
 * 5 so the hook transit preserves all five possible arguments even when the
 * bypass is toggled off and the origin is allowed to run.
 */
static void before_defex(hook_fargs5_t *args, void *udata)
{
    (void)udata;

    if (g_cfg.defex_enabled) {
        args->skip_origin = 1;
        args->ret = 0;
    }
}

/* DEFEX credential-zeroing penalty path; do not run it either. */
static void before_defex_zero_creds(hook_fargs1_t *args, void *udata)
{
    (void)udata;

    if (g_cfg.defex_enabled) {
        args->skip_origin = 1;
        args->ret = 0;
    }
}

/*
 * FIVE/PROCA execution and mmap integrity checks.  Let the original run so
 * task-integrity state stays consistent, then force success.
 *
 * five_bprm_check() returns -ENOMEM when it cannot allocate the new task
 * integrity object.  Do not paper over that one: continuing with no object
 * would make later FIVE code dereference NULL.  Every other failure is
 * neutralized.
 */
static void after_five_bprm(hook_fargs2_t *args, void *udata)
{
    (void)udata;

    if (!g_cfg.five_enabled) return;
    if ((int32_t)args->ret == -12 /* -ENOMEM */) return;
    if (args->ret) args->ret = 0;
}

static void after_five(hook_fargs2_t *args, void *udata)
{
    (void)udata;

    if (g_cfg.five_enabled) args->ret = 0;
}

/*
 * SELinux central permission checks.  avc_has_perm* are separate from
 * avc_denied, which KernelPatch itself already owns; hooking the wrappers
 * avoids fighting the host for the same function.
 *
 * The argument layout differs across kernel versions (the newer kernels pass
 * struct selinux_state * first).  We do not inspect the arguments, so the
 * same callback is valid for both layouts.
 */
static void before_avc(hook_fargs8_t *args, void *udata)
{
    (void)udata;

    if (g_cfg.selinux_permissive) {
        args->skip_origin = 1;
        args->ret = 0;
        return;
    }

    if (g_cfg.selinux_targeted && gsi_uid_allowed((uint32_t)current_uid())) {
        args->skip_origin = 1;
        args->ret = 0;
    }
}

static void gsi_record_hook(const char *name, unsigned long addr, int err,
                            void *before, void *after)
{
    if (g_hook_count >= GSI_FIX_MAX_HOOKS) return;

    g_hooks[g_hook_count].name = name;
    g_hooks[g_hook_count].addr = addr;
    g_hooks[g_hook_count].err = addr ? err : GSI_HOOK_MISSING;
    g_hooks[g_hook_count].before = before;
    g_hooks[g_hook_count].after = after;
    g_hook_count++;
}

static void gsi_install_one(const char *name, int argno, void *before, void *after)
{
    unsigned long addr = gsi_lookup_symbol(name);
    int err = GSI_HOOK_MISSING;

    if (addr) {
        err = (int)hook_wrap((void *)addr, argno, before, after, 0);
    }

    gsi_record_hook(name, addr, err, before, after);

    if (!addr) {
        pr_info("[gsi-fix] %s: symbol not found (skipped)\n", name);
    } else if (err) {
        pr_info("[gsi-fix] %s: hook failed (%d)\n", name, err);
    } else {
        pr_info("[gsi-fix] %s: hooked at %lx\n", name, addr);
    }
}

void gsi_hooks_install(void)
{
    g_hook_count = 0;

    /* Samsung DEFEX */
    gsi_install_one("task_defex_enforce", 5, (void *)before_defex, 0);
    gsi_install_one("task_defex_zero_creds", 1, (void *)before_defex_zero_creds, 0);

    /* Samsung FIVE (used by PROCA) */
    gsi_install_one("five_bprm_check", 2, 0, (void *)after_five_bprm);
    gsi_install_one("five_file_mmap", 2, 0, (void *)after_five);
    gsi_install_one("five_file_verify", 2, 0, (void *)after_five);

    /* SELinux central checks */
    gsi_install_one("avc_has_perm", 6, (void *)before_avc, 0);
    gsi_install_one("avc_has_perm_flags", 7, (void *)before_avc, 0);
    gsi_install_one("avc_has_extended_perms", 8, (void *)before_avc, 0);
}

void gsi_hooks_uninstall(void)
{
    int i;

    for (i = 0; i < g_hook_count; i++) {
        if (!g_hooks[i].addr || g_hooks[i].err) continue;
        hook_unwrap((void *)g_hooks[i].addr, g_hooks[i].before, g_hooks[i].after);
    }
    g_hook_count = 0;
}
