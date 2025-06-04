static ptls_cipher_suite_t *find_cipher_suite(ptls_context_t *ctx, uint16_t id)
{
    ptls_cipher_suite_t **cs;

    for (cs = ctx->cipher_suites; *cs != NULL && (*cs)->id != id; ++cs)
        ;
    return *cs;
}


// Source: picotls.c
// Lines 2164-2171
