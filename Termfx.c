#ifdef termfx
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
    const char *key;
    const char *value;
} TermfxMapping;

typedef int (*TermfxSendFunc)(int, const char *);
typedef void (*TermfxLogFunc)(const char *);

static int ensure_capacity(char **buf, size_t min_size, TermfxLogFunc logger) {
    if (!buf) return -1;
    size_t cur = *buf ? strlen(*buf) + 1 : 0;
    if (cur >= min_size) return 0;
    size_t new_size = min_size < 256 ? 256 : min_size;
    char *p = realloc(*buf, new_size);
    if (!p) {
        if (logger) logger("termfx realloc fail");
        return -2;
    }
    *buf = p;
    return 0;
}

int termfx_replace_all(char **buffer, const char *needle, const char *replacement, TermfxLogFunc logger) {
    if (!buffer || !*buffer || !needle || !replacement) return -1;
    size_t nlen = strlen(needle);
    if (!nlen) return 0;
    size_t rlen = strlen(replacement);
    char *s = *buffer;
    while ((s = strstr(s, needle))) {
        size_t curlen = strlen(*buffer);
        size_t pre = s - *buffer;
        size_t newlen = curlen - nlen + rlen;
        if (ensure_capacity(buffer, newlen + 1, logger) != 0) return -2;
        s = *buffer + pre;
        memmove(s + rlen, s + nlen, curlen - pre - nlen + 1);
        memcpy(s, replacement, rlen);
        s += rlen;
    }
    return 0;
}

int termfx_process(char **buffer, const TermfxMapping *map, TermfxLogFunc logger) {
    if (!buffer || !*buffer) return -1;
    const struct { const char *k; const char *v; } def[] = {
        {"<Red>", "\033[0;31m"}, {"<Green>", "\033[0;32m"}, {"<Yellow>", "\033[0;33m"},
        {"<Blue>", "\033[0;34m"}, {"<Magenta>", "\033[0;35m"}, {"<Cyan>", "\033[0;36m"},
        {"<White>", "\033[0;37m"}, {"<Bold>", "\033[1m"}, {"<Dim>", "\033[2m"},
        {"<Underline>", "\033[4m"}, {"<Blink>", "\033[5m"}, {"<Reset>", "\033[0m"},
        {"<New>", "\r\n"}, {NULL, NULL}
    };
    for (int i = 0; def[i].k; i++) termfx_replace_all(buffer, def[i].k, def[i].v, logger);
    if (map) for (int i = 0; map[i].key && map[i].value; i++) termfx_replace_all(buffer, map[i].key, map[i].value, logger);
    return 0;
}

int termfx_banner(const char *file, int fd, const TermfxMapping *map, TermfxSendFunc sendf, TermfxLogFunc logger) {
    if (!file || !sendf) return -1;
    FILE *fp = fopen(file, "r");
    if (!fp) {
        char msg[256];
        snprintf(msg, sizeof(msg), "banner open %s: %s\r\n", file, strerror(errno));
        sendf(fd, msg);
        return -1;
    }
    char *buf = calloc(1, 2048);
    char line[2048];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        while (len && (line[len - 1] == '\n' || line[len - 1] == '\r')) line[--len] = 0;
        ensure_capacity(&buf, strlen(line) + 2, logger);
        strcpy(buf, line);
        termfx_process(&buf, map, logger);
        sendf(fd, buf);
        sendf(fd, "\r\n");
    }
    free(buf);
    fclose(fp);
    return 0;
}
#endif
