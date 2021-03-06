#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "dat.h"

struct ms tubes;

tube
make_tube(const char *name)
{
    tube t;
    struct job tmp_job = {0};
    t = new(struct tube);
    if (!t) return NULL;

    t->name[MAX_TUBE_NAME_LEN - 1] = '\0';
    strncpy(t->name, name, MAX_TUBE_NAME_LEN - 1);
    if (t->name[MAX_TUBE_NAME_LEN - 1] != '\0') twarnx("truncating tube name");

    t->ready.less = job_pri_less;
    t->delay.less = job_delay_less;
    t->ready.rec = job_setheappos;
    t->delay.rec = job_setheappos;
    t->buried = tmp_job;
    t->buried.prev = t->buried.next = &t->buried;
    ms_init(&t->waiting, NULL, NULL);

    return t;
}

static void
tube_free(tube t)
{
    prot_remove_tube(t);
    free(t->ready.data);
    free(t->delay.data);
    ms_clear(&t->waiting);
    free(t);
}

void
tube_dref(tube t)
{
    if (!t) return;
    if (t->refs < 1) return twarnx("refs is zero for tube: %s", t->name);

    --t->refs;
    if (t->refs < 1) tube_free(t);
}

void
tube_iref(tube t)
{
    if (!t) return;
    ++t->refs;
}

static tube
make_and_insert_tube(const char *name)
{
    int r;
    tube t = NULL;

    t = make_tube(name);
    if (!t) return NULL;

    /* We want this global tube list to behave like "weak" refs, so don't
     * increment the ref count. */
    r = ms_append(&tubes, t);
    if (!r) return tube_dref(t), (tube) 0;

    return t;
}

tube
tube_find(const char *name)
{
    tube t;
    size_t i;

    for (i = 0; i < tubes.used; i++) {
        t = tubes.items[i];
        if (strncmp(t->name, name, MAX_TUBE_NAME_LEN) == 0) return t;
    }
    return NULL;
}

tube
tube_find_or_make(const char *name)
{
    tube ret = tube_find(name);
    return ret ? ret : make_and_insert_tube(name);
}

