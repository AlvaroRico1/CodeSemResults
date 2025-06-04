void gkc_sanity_check(struct gkc_summary *s)
{
    uint64_t nr_elems, nr_alloced;
    struct list *cur;
    struct gkc_tuple *tcur;

    nr_elems = 0;
    nr_alloced = 0;
    cur = s->head.next;
    while (cur != &s->head) {
        tcur = list_to_tuple(cur);
        cur = cur->next;
        nr_elems += tcur->g;
        nr_alloced++;
        if (s->nr_elems > (1/s->epsilon)) {
            /* there must be enough observations for this to become true */
            assert(tcur->g + tcur->delta <= (s->nr_elems * s->epsilon * 2));
        }
        assert(nr_alloced <= s->alloced);
    }
    assert(nr_elems == s->nr_elems);
    assert(nr_alloced == s->alloced);
}


// Source: gkc.c
// Lines 154-176
