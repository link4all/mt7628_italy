#include "sysconf.h"

#include <uci.h>
#include <string.h>
#include <stdlib.h>

string sysconf_get(const char *key)
{
    struct uci_context *ctx = NULL;

    do {
        char o[strlen(key)+1];
        strcpy(o, key);

        ctx = uci_alloc_context();
        if (!ctx) break;

        struct uci_ptr ptr;
        if ((uci_lookup_ptr(ctx, &ptr, o, true) != UCI_OK) || !(ptr.flags & (1 << 1))) break;

        string value = ptr.o->v.string;
        uci_free_context(ctx);
        return value;
    } while(0);

    if (ctx) uci_free_context(ctx);
    return "";
}

int sysconf_set(const char *key, const char *value)
{
    struct uci_context *uctx = uci_alloc_context();
    struct uci_ptr uptr;

    char option_s[strlen(key)+strlen(value)+1];
    sprintf(option_s, "%s=%s", key, value);

    if (uci_lookup_ptr(uctx, &uptr, option_s, true) != UCI_OK) {
        uci_free_context(uctx);
        return -1;
    }

    uci_set(uctx, &uptr);
    uci_save(uctx, uptr.p);
    uci_commit(uctx, &uptr.p, false);

    uci_free_context(uctx);
    return 0;
}

int sysconf_del(const char *key)
{
    struct uci_context *uctx = uci_alloc_context();
    struct uci_ptr uptr;

    char option_s[strlen(key)+1];
    sprintf(option_s, "%s", key);

    if (uci_lookup_ptr(uctx, &uptr, option_s, true) != UCI_OK) {
        uci_free_context(uctx);
        return -1;
    }

    uci_delete(uctx, &uptr);
    uci_save(uctx, uptr.p);
    uci_commit(uctx, &uptr.p, false);

    uci_free_context(uctx);
    return 0;
}
