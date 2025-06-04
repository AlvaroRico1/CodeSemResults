static int extend_abbrev_len(const struct object_id *oid, void *cb_data)
{
	struct min_abbrev_data *mad = cb_data;

	unsigned int i = mad->init_len;
	while (mad->hex[i] && mad->hex[i] == get_hex_char_from_oid(oid, i))
		i++;

	if (i < GIT_MAX_RAWSZ && i >= mad->cur_len)
		mad->cur_len = i + 1;

	return 0;
}


// Source: object-name.c
// Lines 556-568
