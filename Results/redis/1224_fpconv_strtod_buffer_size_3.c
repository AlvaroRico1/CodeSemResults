static int strtod_buffer_size(const char *s)
{
    const char *p = s;

    while (valid_number_character(*p))
        p++;

    return p - s;
}


// Source: fpconv.c
// Lines 94-102
