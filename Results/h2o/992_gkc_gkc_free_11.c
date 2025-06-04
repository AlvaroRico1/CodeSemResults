static void gkc_free(struct gkc_summary *s, struct gkc_tuple *p)
{
    struct freelist *flp = (void *)p;
    s->alloced--;

    flp->next = s->fl;
    s->fl = flp;
}


// Source: gkc.c
// Lines 194-201
