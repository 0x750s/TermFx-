#include <stdio.h>
#define termfx
#include "termfx.c"

int send_safe(int fd, const char *msg) { (void)fd; return printf("%s", msg); }

int main() {
    TermfxMapping vars[] = {
        {"<Username>", "root"},
        {"<Plan>", "Eternal"},
        {"<Expiry>", "Never"},
        {NULL, NULL}
    };
    char *buf = strdup("<Red>Welcome <Username><Reset>");
    termfx_process(&buf, vars, NULL);
    printf("%s\n", buf);
    free(buf);
    termfx_banner("banner.txt", 0, vars, send_safe, NULL);
    return 0;
}
