void h2o_mem_swap(void *_x, void *_y, size_t len)
{
    char *x = _x, *y = _y;
    char buf[256];

    while (len != 0) {
        size_t blocksz = len < sizeof(buf) ? len : sizeof(buf);
        memcpy(buf, x, blocksz);
        memcpy(x, y, blocksz);
        memcpy(y, buf, blocksz);
        len -= blocksz;
        x += blocksz;
        y += blocksz;
    }
}


// Source: memory.c
// Lines 524-538
