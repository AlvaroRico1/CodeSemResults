format_each(struct format_tree *ft, void (*cb)(const char *, const char *,
    void *), void *arg)
{
	const struct format_table_entry	*fte;
	struct format_entry		*fe;
	u_int				 i;
	char				 s[64];
	void				*value;
	struct timeval			*tv;

	for (i = 0; i < nitems(format_table); i++) {
		fte = &format_table[i];

		value = fte->cb(ft);
		if (value == NULL)
			continue;
		if (fte->type == FORMAT_TABLE_TIME) {
			tv = value;
			xsnprintf(s, sizeof s, "%lld", (long long)tv->tv_sec);
			cb(fte->key, s, arg);
		} else {
			cb(fte->key, value, arg);
			free(value);
		}
	}
	RB_FOREACH(fe, format_entry_tree, &ft->tree) {
		if (fe->time != 0) {
			xsnprintf(s, sizeof s, "%lld", (long long)fe->time);
			cb(fe->key, s, arg);
		} else {
			if (fe->value == NULL && fe->cb != NULL) {
				fe->value = fe->cb(ft);
				if (fe->value == NULL)
					fe->value = xstrdup("");
			}
			cb(fe->key, fe->value, arg);
		}
	}
}


// Source: format.c
// Lines 3116-3154
