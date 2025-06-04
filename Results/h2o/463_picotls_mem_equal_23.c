static int mem_equal(const void *_x, const void *_y, size_t len)
{
    const volatile uint8_t *x = _x, *y = _y;
    uint8_t t = 0;

    for (; len != 0; --len)
        t |= *x++ ^ *y++;

    return t == 0;
}


// Source: picotls.c
// Lines 5288-5297
