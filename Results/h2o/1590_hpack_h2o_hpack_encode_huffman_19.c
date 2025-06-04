size_t h2o_hpack_encode_huffman(uint8_t *_dst, const uint8_t *src, size_t len)
{
    uint8_t *dst = _dst, *dst_end = dst + len;
    const uint8_t *src_end = src + len;
    uint64_t bits = 0;
    int bits_left = 40;

    while (src != src_end) {
        const nghttp2_huff_sym *sym = huff_sym_table + *src++;
        bits |= (uint64_t)sym->code << (bits_left - sym->nbits);
        bits_left -= sym->nbits;
        while (bits_left <= 32) {
            *dst++ = bits >> 32;
            bits <<= 8;
            bits_left += 8;
            if (dst == dst_end) {
                return SIZE_MAX;
            }
        }
    }

    if (bits_left != 40) {
        bits |= ((uint64_t)1 << bits_left) - 1;
        *dst++ = bits >> 32;
    }
    if (dst == dst_end) {
        return SIZE_MAX;
    }

    return dst - _dst;
}


// Source: hpack.c
// Lines 682-712
