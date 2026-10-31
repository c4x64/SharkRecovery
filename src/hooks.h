/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef GSI_FIX_HOOKS_H
#define GSI_FIX_HOOKS_H

#define GSI_FIX_MAX_HOOKS 10

struct gsi_hook_info {
    const char *name;
    unsigned long addr;
    int err;
    void *before;
    void *after;
};

extern struct gsi_hook_info g_hooks[GSI_FIX_MAX_HOOKS];
extern int g_hook_count;

void gsi_hooks_install(void);
void gsi_hooks_uninstall(void);

#endif /* GSI_FIX_HOOKS_H */
