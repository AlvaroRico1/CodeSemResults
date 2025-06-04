static struct gkc_tuple *gkc_alloc(struct gkc_summary *s)
{
    s->alloced++;
    if (s->alloced > s->max_alloced) {
        s->max_alloced = s->alloced;
    }

    if (s->fl) {
        void *ret;
        ret = s->fl;
        s->fl = s->fl->next;
        return ret;
    }


// Source: gkc.c
// Lines 178-190
