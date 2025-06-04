static inline void storen(void *_p, size_t l, __m128i v)
{
    uint8_t buf[16], *p = _p;

    *(__m128i *)buf = v;

    for (size_t i = 0; i != l; ++i)
        p[i] = buf[i];
}


// Source: fusion.c
// Lines 229-237
