/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef GSI_FIX_UTIL_H
#define GSI_FIX_UTIL_H

#include <stddef.h>
#include <stdint.h>

size_t gsi_strlen(const char *s);
void gsi_strlcpy(char *dst, const char *src, size_t size);
void gsi_strlcat(char *dst, const char *src, size_t size);
int gsi_strcmp(const char *a, const char *b);
int gsi_strncmp(const char *a, const char *b, size_t n);
int gsi_starts_with(const char *s, const char *prefix);
int gsi_parse_uint(const char *s, uint32_t *out);
const char *gsi_next_token(const char *p, char *tok, size_t tok_size);
void gsi_append_uint(char *buf, size_t size, uint32_t value);

#endif /* GSI_FIX_UTIL_H */
