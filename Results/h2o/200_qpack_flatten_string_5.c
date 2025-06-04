static void flatten_string(h2o_byte_vector_t *buf, const char *src, size_t len, unsigned prefix_bits, int dont_compress)
{
    size_t hufflen;

    if (dont_compress || (hufflen = h2o_hpack_encode_huffman(buf->entries + buf->size + 1, (void *)src, len)) == SIZE_MAX) {
        /* uncompressed */
        buf->entries[buf->size] &= ~((2 << prefix_bits) - 1); /* clear huffman mark */
        flatten_int(buf, len, prefix_bits);
        memcpy(buf->entries + buf->size, src, len);
        buf->size += len;
    } else {
        /* build huffman header and adjust the location (if necessary) */
        uint8_t tmpbuf[H2O_HPACK_ENCODE_INT_MAX_LENGTH], *p = tmpbuf;
        *p = buf->entries[buf->size] & ~((1 << prefix_bits) - 1);
        *p |= (1 << prefix_bits); /* huffman mark */
        p = h2o_hpack_encode_int(p, hufflen, prefix_bits);
        if (p - tmpbuf == 1) {
            buf->entries[buf->size] = tmpbuf[0];
        } else {
            memmove(buf->entries + buf->size + (p - tmpbuf), buf->entries + buf->size + 1, hufflen);
            memcpy(buf->entries + buf->size, tmpbuf, p - tmpbuf);
        }
        buf->size += p - tmpbuf + hufflen;
    }


// Source: qpack.c
// Lines 1028-1051
