char *quicly_escape_unsafe_string(char *buf, const void *bytes, size_t len)
{
    char *dst = buf;
    const char *src = bytes, *end = src + len;

    for (; src != end; ++src) {
        if ((0x20 <= *src && *src <= 0x7e) && !(*src == '"' || *src == '\'' || *src == '\\')) {
            *dst++ = *src;
        } else {
            *dst++ = '\\';
            *dst++ = 'x';
            quicly_byte_to_hex(dst, (uint8_t)*src);
            dst += 2;
        }
    }
    *dst = '\0';

    return buf;
}


// Source: quicly.c
// Lines 6156-6174
