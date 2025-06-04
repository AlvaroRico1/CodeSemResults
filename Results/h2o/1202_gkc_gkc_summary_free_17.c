void gkc_summary_free(struct gkc_summary *s)
{
    struct freelist *fl;
    struct list *cur;

    cur = s->head.next;
    while (cur != &s->head) {
        struct list *next;
        next = cur->next;
        gkc_free(s, list_to_tuple(cur));
        cur = next;
    }


// Source: gkc.c
// Lines 203-214
