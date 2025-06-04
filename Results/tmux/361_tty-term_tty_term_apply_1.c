tty_term_apply(struct tty_term *term, const char *capabilities, int quiet)
{
	const struct tty_term_code_entry	*ent;
	struct tty_code				*code;
	size_t                                   offset = 0;
	char					*cp, *value, *s;
	const char				*errstr, *name = term->name;
	u_int					 i;
	int					 n, remove;

	while ((s = tty_term_override_next(capabilities, &offset)) != NULL) {
		if (*s == '\0')
			continue;
		value = NULL;

		remove = 0;
		if ((cp = strchr(s, '=')) != NULL) {
			*cp++ = '\0';
			value = xstrdup(cp);
			if (strunvis(value, cp) == -1) {
				free(value);
				value = xstrdup(cp);
			}
		} else if (s[strlen(s) - 1] == '@') {
			s[strlen(s) - 1] = '\0';
			remove = 1;
		} else
			value = xstrdup("");

		if (!quiet) {
			if (remove)
				log_debug("%s override: %s@", name, s);
			else if (*value == '\0')
				log_debug("%s override: %s", name, s);
			else
				log_debug("%s override: %s=%s", name, s, value);
		}

		for (i = 0; i < tty_term_ncodes(); i++) {
			ent = &tty_term_codes[i];
			if (strcmp(s, ent->name) != 0)
				continue;
			code = &term->codes[i];

			if (remove) {
				code->type = TTYCODE_NONE;
				continue;
			}
			switch (ent->type) {
			case TTYCODE_NONE:
				break;
			case TTYCODE_STRING:
				if (code->type == TTYCODE_STRING)
					free(code->value.string);
				code->value.string = xstrdup(value);
				code->type = ent->type;
				break;
			case TTYCODE_NUMBER:
				n = strtonum(value, 0, INT_MAX, &errstr);
				if (errstr != NULL)
					break;
				code->value.number = n;
				code->type = ent->type;
				break;
			case TTYCODE_FLAG:
				code->value.flag = 1;
				code->type = ent->type;
				break;
			}
		}

		free(value);
	}
}


// Source: tty-term.c
// Lines 357-430
