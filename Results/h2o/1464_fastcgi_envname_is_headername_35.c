static int envname_is_headername(const h2o_iovec_t *env, const h2o_iovec_t *header)
{
    const char *ep, *hp, *hend;

    if (env->len != 5 + header->len)
        return 0;
    if (memcmp(env->base, "HTTP_", 5) != 0)
        return 0;
    for (ep = env->base + 5, hp = header->base, hend = hp + header->len; hp < hend; ++ep, ++hp)
        if (*ep != h2o_toupper(*hp))
            return 0;
    return 1;
}


// Source: fastcgi.c
// Lines 202-214
