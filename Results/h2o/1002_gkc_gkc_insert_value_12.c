void gkc_insert_value(struct gkc_summary *s, double value)
{
    struct list *cur = NULL;
    struct gkc_tuple *new, *tcur, *tnext = NULL;

    new = gkc_alloc(s);
    memset(new, 0, sizeof(*new));
    new->value = value;
    new->g = 1;
    list_init(&new->node);


    s->nr_elems++;


    /* first insert */
    if (list_empty(&s->head)) {
        list_add(&s->head, &new->node);
        return;
    }

    cur = s->head.next;
    tcur = list_to_tuple(cur);
    /* v < v0, new min */
    if (tcur->value > new->value) {
        list_add(&s->head, &new->node);
        goto out;
    }

    double gi = 0;
    while (cur->next != &s->head) {
        tnext = list_to_tuple(cur->next);
        tcur = list_to_tuple(cur);

        gi += tcur->g;
        if (tcur->value <= new->value && new->value < tnext->value) {
            /*     INSERT "(v, 1, Δ)" into S between vi and vi+1; */
            new->delta = tcur->g + tcur->delta - 1;
            list_add(cur, &new->node);
            goto out;
        }
        cur = cur->next;
    }
    /* v > vs-1, new max */
    list_add_tail(&s->head, &new->node);
out:
    if (s->nr_elems % (int)(1/(2*s->epsilon))) {
        gkc_compress(s);
    }
}


// Source: gkc.c
// Lines 309-358
