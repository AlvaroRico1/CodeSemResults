static X509 *to_x509(ptls_iovec_t vec)
{
    const uint8_t *p = vec.base;
    return d2i_X509(NULL, &p, (long)vec.len);
}


// Source: openssl.c
// Lines 1062-1066
