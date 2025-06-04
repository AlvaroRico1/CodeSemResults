static int setup_configurators(h2o_configurator_context_t *ctx, int is_enter, yoml_t *node)
{
    h2o_linklist_t *n;

    for (n = ctx->globalconf->configurators.next; n != &ctx->globalconf->configurators; n = n->next) {
        h2o_configurator_t *c = H2O_STRUCT_FROM_MEMBER(h2o_configurator_t, _link, n);
        if (is_enter) {
            if (c->enter != NULL && c->enter(c, ctx, node) != 0)
                return -1;
        } else {
            if (c->exit != NULL && c->exit(c, ctx, node) != 0)
                return -1;
        }
    }

    return 0;
}


// Source: configurator.c
// Lines 109-125
