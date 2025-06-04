tty_term_read_list(const char *name, int fd, char ***caps, u_int *ncaps,
    char **cause)
{
	const struct tty_term_code_entry	*ent;
	int					 error, n;
	u_int					 i;
	const char				*s;
	char					 tmp[11];

	if (setupterm((char *)name, fd, &error) != OK) {
		switch (error) {
		case 1:
			xasprintf(cause, "can't use hardcopy terminal: %s",
			    name);
			break;
		case 0:
			xasprintf(cause, "missing or unsuitable terminal: %s",
			    name);
			break;
		case -1:
			xasprintf(cause, "can't find terminfo database");
			break;
		default:
			xasprintf(cause, "unknown error");
			break;
		}
		return (-1);
	}

	*ncaps = 0;
	*caps = NULL;

	for (i = 0; i < tty_term_ncodes(); i++) {
		ent = &tty_term_codes[i];
		switch (ent->type) {
		case TTYCODE_NONE:
			continue;
		case TTYCODE_STRING:
			s = tigetstr((char *)ent->name);
			if (s == NULL || s == (char *)-1)
				continue;
			break;
		case TTYCODE_NUMBER:
			n = tigetnum((char *)ent->name);
			if (n == -1 || n == -2)
				continue;
			xsnprintf(tmp, sizeof tmp, "%d", n);
			s = tmp;
			break;
		case TTYCODE_FLAG:
			n = tigetflag((char *) ent->name);
			if (n == -1)
				continue;
			if (n)
				s = "1";
			else
				s = "0";
			break;
		}
		*caps = xreallocarray(*caps, (*ncaps) + 1, sizeof **caps);
		xasprintf(&(*caps)[*ncaps], "%s=%s", ent->name, s);
		(*ncaps)++;
	}

#if !defined(NCURSES_VERSION_MAJOR) || NCURSES_VERSION_MAJOR > 5 || \
    (NCURSES_VERSION_MAJOR == 5 && NCURSES_VERSION_MINOR > 6)
	del_curterm(cur_term);
#endif
	return (0);
}


// Source: tty-term.c
// Lines 664-733
