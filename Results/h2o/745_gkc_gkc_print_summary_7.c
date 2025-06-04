void gkc_print_summary(struct gkc_summary *s)
{
    struct gkc_tuple *tcur;
    struct list *cur;

    fprintf(stderr, "nr_elems: %zu, epsilon: %.02f, alloced: %" PRIu64 ", overfilled: %.02f, max_alloced: %" PRIu64 "\n",
            s->nr_elems, s->epsilon, s->alloced, 2 * s->epsilon * s->nr_elems, s->max_alloced);
    if (list_empty(&s->head)) {
        fprintf(stderr, "Empty summary\n");
        return;
    }

    cur = s->head.next;
    while (cur != &s->head) {
        tcur = list_to_tuple(cur);
        fprintf(stderr, "(v: %" PRIu64 ", g: %.02f, d: %" PRIu64 ") ", tcur->value, tcur->g, tcur->delta);
        cur = cur->next;
    }
    fprintf(stderr, "\n");
}


// Source: gkc.c
// Lines 360-379
