cmd_send_keys_inject_string(struct cmdq_item *item, struct cmdq_item *after,
    struct args *args, int i)
{
	const char		*s = args_string(args, i);
	struct utf8_data	*ud, *loop;
	utf8_char		 uc;
	key_code		 key;
	char			*endptr;
	long			 n;
	int			 literal;

	if (args_has(args, 'H')) {
		n = strtol(s, &endptr, 16);
		if (*s =='\0' || n < 0 || n > 0xff || *endptr != '\0')
			return (item);
		return (cmd_send_keys_inject_key(item, after, KEYC_LITERAL|n));
	}

	literal = args_has(args, 'l');
	if (!literal) {
		key = key_string_lookup_string(s);
		if (key != KEYC_NONE && key != KEYC_UNKNOWN) {
			after = cmd_send_keys_inject_key(item, after, key);
			if (after != NULL)
				return (after);
		}
		literal = 1;
	}
	if (literal) {
		ud = utf8_fromcstr(s);
		for (loop = ud; loop->size != 0; loop++) {
			if (loop->size == 1 && loop->data[0] <= 0x7f)
				key = loop->data[0];
			else {
				if (utf8_from_data(loop, &uc) != UTF8_DONE)
					continue;
				key = uc;
			}
			after = cmd_send_keys_inject_key(item, after, key);
		}
		free(ud);
	}
	return (after);
}


// Source: cmd-send-keys.c
// Lines 90-133
