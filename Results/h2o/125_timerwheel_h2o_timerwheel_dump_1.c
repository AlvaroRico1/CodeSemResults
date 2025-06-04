void h2o_timerwheel_dump(h2o_timerwheel_t *ctx)
{
    size_t wheel, slot;

    h2o_error_printf("%s(%p):\n", __FUNCTION__, ctx);
    for (wheel = 0; wheel < ctx->num_wheels; wheel++) {
        for (slot = 0; slot < H2O_TIMERWHEEL_SLOTS_PER_WHEEL; slot++) {
            h2o_linklist_t *anchor = &ctx->wheels[wheel][slot], *l;
            for (l = anchor->next; l != anchor; l = l->next) {
                h2o_timerwheel_entry_t *e = H2O_STRUCT_FROM_MEMBER(h2o_timerwheel_entry_t, _link, l);
                h2o_error_printf("  - {wheel: %zu, slot: %zu, expires:%" PRIu64 ", self: %p, cb:%p}\n", wheel, slot, e->expire_at,
                                 e, e->cb);
            }
        }


// Source: timerwheel.c
// Lines 64-77
