uint64_t gkc_query(struct gkc_summary *s, double q)
{
    struct list *cur, *next;
    int rank;
    double gi;
    double ne;

    rank = 0.5 + q * s->nr_elems;
    ne = s->nr_elems * s->epsilon;
    gi = 0;
    if (list_empty(&s->head)) {
        return 0;
    }

    cur = s->head.next;

    while (1) {
        struct gkc_tuple *tcur, *tnext;

        tcur = list_to_tuple(cur);
        next = cur->next;
        if (next == &s->head) {
            return tcur->value;
        }
        tnext = list_to_tuple(next);

        gi += tcur->g;
        if ((rank + ne) < (gi + tnext->g + tnext->delta)) {
            if ((rank + ne) < (gi + tnext->g)) {
                return tcur->value;
            }
            return tnext->value;
        }
        cur = next;
    }
}


// Source: gkc.c
// Lines 225-260
