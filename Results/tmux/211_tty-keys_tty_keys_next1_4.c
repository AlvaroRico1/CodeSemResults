tty_keys_next1(struct tty *tty, const char *buf, size_t len, key_code *key,
    size_t *size, int expired)
{
	struct client		*c = tty->client;
	struct tty_key		*tk, *tk1;
	struct utf8_data	 ud;
	enum utf8_state		 more;
	utf8_char		 uc;
	u_int			 i;

	log_debug("%s: next key is %zu (%.*s) (expired=%d)", c->name, len,
	    (int)len, buf, expired);

	/* Is this a known key? */
	tk = tty_keys_find(tty, buf, len, size);
	if (tk != NULL && tk->key != KEYC_UNKNOWN) {
		tk1 = tk;
		do
			log_debug("%s: keys in list: %#llx", c->name, tk1->key);
		while ((tk1 = tk1->next) != NULL);
		if (tk->next != NULL && !expired)
			return (1);
		*key = tk->key;
		return (0);
	}

	/* Is this valid UTF-8? */
	more = utf8_open(&ud, (u_char)*buf);
	if (more == UTF8_MORE) {
		*size = ud.size;
		if (len < ud.size) {
			if (!expired)
				return (1);
			return (-1);
		}
		for (i = 1; i < ud.size; i++)
			more = utf8_append(&ud, (u_char)buf[i]);
		if (more != UTF8_DONE)
			return (-1);

		if (utf8_from_data(&ud, &uc) != UTF8_DONE)
			return (-1);
		*key = uc;

		log_debug("%s: UTF-8 key %.*s %#llx", c->name, (int)ud.size,
		    ud.data, *key);
		return (0);
	}

	return (-1);
}


// Source: tty-keys.c
// Lines 607-657
