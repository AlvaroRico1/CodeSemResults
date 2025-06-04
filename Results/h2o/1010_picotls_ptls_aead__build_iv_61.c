void ptls_aead__build_iv(ptls_aead_algorithm_t *algo, uint8_t *iv, const uint8_t *static_iv, uint64_t seq)
{
    size_t iv_size = algo->iv_size, i;
    const uint8_t *s = static_iv;
    uint8_t *d = iv;

    /* build iv */
    for (i = iv_size - 8; i != 0; --i)
        *d++ = *s++;
    i = 64;
    do {
        i -= 8;
        *d++ = *s++ ^ (uint8_t)(seq >> i);
    } while (i != 0);
}


// Source: picotls.c
// Lines 5264-5278
