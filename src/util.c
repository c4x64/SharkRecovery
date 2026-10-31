/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "util.h"

size_t gsi_strlen(const char *s)
{
    const char *p = s;

    while (*p) p++;
    return (size_t)(p - s);
}

void gsi_strlcpy(char *dst, const char *src, size_t size)
{
    size_t i;

    if (!dst || !size) return;
    for (i = 0; i + 1 < size && src[i]; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

void gsi_strlcat(char *dst, const char *src, size_t size)
{
    size_t used;
    size_t i;

    if (!dst || !src || !size) return;
    used = gsi_strlen(dst);
    if (used >= size) return;

    for (i = 0; used + i + 1 < size && src[i]; i++) {
        dst[used + i] = src[i];
    }
    dst[used + i] = '\0';
}

int gsi_strcmp(const char *a, const char *b)
{
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int gsi_strncmp(const char *a, const char *b, size_t n)
{
    while (n && *a && *a == *b) {
        a++;
        b++;
        n--;
    }
    if (!n) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

int gsi_starts_with(const char *s, const char *prefix)
{
    return gsi_strncmp(s, prefix, gsi_strlen(prefix)) == 0;
}

int gsi_parse_uint(const char *s, uint32_t *out)
{
    uint32_t value = 0;
    int any = 0;

    if (!s || !out || !*s) return -1;

    while (*s) {
        uint32_t digit;

        if (*s < '0' || *s > '9') return -1;
        digit = (uint32_t)(*s - '0');
        if (value > (0xffffffffu - digit) / 10u) return -1;
        value = value * 10u + digit;
        any = 1;
        s++;
    }

    if (!any) return -1;
    *out = value;
    return 0;
}

const char *gsi_next_token(const char *p, char *tok, size_t tok_size)
{
    size_t i = 0;

    if (!p) p = "";
    while (*p == ' ' || *p == '\t' || *p == ',') p++;

    while (*p && *p != ' ' && *p != '\t' && *p != ',') {
        if (i + 1 < tok_size) tok[i++] = *p;
        p++;
    }
    if (tok_size) tok[i] = '\0';

    /* Leave the cursor on the next token, not on the delimiter. */
    while (*p == ' ' || *p == '\t' || *p == ',') p++;

    return p;
}

void gsi_append_uint(char *buf, size_t size, uint32_t value)
{
    char tmp[10];
    char out[11];
    int n = 0;
    int i = 0;

    if (!buf || !size) return;
    if (!value) {
        gsi_strlcat(buf, "0", size);
        return;
    }

    while (value && n < (int)sizeof(tmp)) {
        tmp[n++] = (char)('0' + (value % 10u));
        value /= 10u;
    }
    while (n) out[i++] = tmp[--n];
    out[i] = '\0';
    gsi_strlcat(buf, out, size);
}
