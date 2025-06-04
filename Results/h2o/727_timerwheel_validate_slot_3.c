static int validate_slot(h2o_timerwheel_t *ctx, size_t wheel, size_t slot)
{
    h2o_linklist_t *anchor = &ctx->wheels[wheel][slot], *link;
    uint64_t at_min, at_max;
    int success = 1;

    calc_expire_for_slot(ctx->num_wheels, ctx->last_run, wheel, slot, &at_min, &at_max);

    for (link = anchor->next; link != anchor; link = link->next) {
        h2o_timerwheel_entry_t *e = H2O_STRUCT_FROM_MEMBER(h2o_timerwheel_entry_t, _link, link);
        if (!(at_min <= e->expire_at && e->expire_at <= at_max)) {
            REPORT_CORRUPT_TIMER(ctx, e, ", wheel=%zu, slot=%zu, expected_range=[%" PRIu64 ",%" PRIu64 "]", wheel, slot, at_min,
                                 at_max);
            success = 0;
        }
    }

    return success;
}


// Source: timerwheel.c
// Lines 127-145
