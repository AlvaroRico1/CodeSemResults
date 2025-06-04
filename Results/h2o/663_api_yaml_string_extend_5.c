yaml_string_extend(yaml_char_t **start,
        yaml_char_t **pointer, yaml_char_t **end)
{
    yaml_char_t *new_start = (yaml_char_t *)yaml_realloc((void*)*start, (*end - *start)*2);

    if (!new_start) return 0;

    memset(new_start + (*end - *start), 0, *end - *start);

    *pointer = new_start + (*pointer - *start);
    *end = new_start + (*end - *start)*2;
    *start = new_start;

    return 1;
}


// Source: api.c
// Lines 74-88
