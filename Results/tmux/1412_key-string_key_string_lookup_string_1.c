key_string_lookup_string(const char *string)
{
	static const char	*other = "!#()+,-.0123456789:;<=>'\r\t\177`/";
	key_code		 key, modifiers;
	u_int			 u, i;
	struct utf8_data	 ud, *udp;
	enum utf8_state		 more;
	utf8_char		 uc;
	char			 m[MB_LEN_MAX + 1];
	int			 mlen;

	/* Is this no key or any key? */
	if (strcasecmp(string, "None") == 0)
		return (KEYC_NONE);
	if (strcasecmp(string, "Any") == 0)
		return (KEYC_ANY);

	/* Is this a hexadecimal value? */
	if (string[0] == '0' && string[1] == 'x') {
		if (sscanf(string + 2, "%x", &u) != 1)
			return (KEYC_UNKNOWN);
		if (u < 32)
			return (u);
		mlen = wctomb(m, u);
		if (mlen <= 0 || mlen > MB_LEN_MAX)
			return (KEYC_UNKNOWN);
		m[mlen] = '\0';

		udp = utf8_fromcstr(m);
		if (udp == NULL ||
		    udp[0].size == 0 ||
		    udp[1].size != 0 ||
		    utf8_from_data(&udp[0], &uc) != UTF8_DONE) {
			free(udp);
			return (KEYC_UNKNOWN);
		}
		free(udp);
		return (uc);
	}

	/* Check for modifiers. */
	modifiers = 0;
	if (string[0] == '^' && string[1] != '\0') {
		modifiers |= KEYC_CTRL;
		string++;
	}
	modifiers |= key_string_get_modifiers(&string);
	if (string == NULL || string[0] == '\0')
		return (KEYC_UNKNOWN);

	/* Is this a standard ASCII key? */
	if (string[1] == '\0' && (u_char)string[0] <= 127) {
		key = (u_char)string[0];
		if (key < 32)
			return (KEYC_UNKNOWN);
	} else {
		/* Try as a UTF-8 key. */
		if ((more = utf8_open(&ud, (u_char)*string)) == UTF8_MORE) {
			if (strlen(string) != ud.size)
				return (KEYC_UNKNOWN);
			for (i = 1; i < ud.size; i++)
				more = utf8_append(&ud, (u_char)string[i]);
			if (more != UTF8_DONE)
				return (KEYC_UNKNOWN);
			if (utf8_from_data(&ud, &uc) != UTF8_DONE)
				return (KEYC_UNKNOWN);
			return (uc|modifiers);
		}

		/* Otherwise look the key up in the table. */
		key = key_string_search_table(string);
		if (key == KEYC_UNKNOWN)
			return (KEYC_UNKNOWN);
		if (~modifiers & KEYC_META)
			key &= ~KEYC_IMPLIED_META;
	}

	/* Convert the standard control keys. */
	if (key <= 127 &&
	    (modifiers & KEYC_CTRL) &&
	    strchr(other, key) == NULL &&
	    key != 9 &&
	    key != 13 &&
	    key != 27) {
		if (key >= 97 && key <= 122)
			key -= 96;
		else if (key >= 64 && key <= 95)
                       key -= 64;
		else if (key == 32)
			key = 0;
		else if (key == 63)
			key = 127;
		else
			return (KEYC_UNKNOWN);
		modifiers &= ~KEYC_CTRL;
	}

	return (key|modifiers);
}


// Source: key-string.c
// Lines 165-263
