int git_config_parse_parameter(const char *text,
			       config_fn_t fn, void *data)
{
	const char *value;
	struct strbuf **pair;
	int ret;

	pair = strbuf_split_str(text, '=', 2);
	if (!pair[0])
		return error(_("bogus config parameter: %s"), text);

	if (pair[0]->len && pair[0]->buf[pair[0]->len - 1] == '=') {
		strbuf_setlen(pair[0], pair[0]->len - 1);
		value = pair[1] ? pair[1]->buf : "";
	} else {
		value = NULL;
	}

	strbuf_trim(pair[0]);
	if (!pair[0]->len) {
		strbuf_list_free(pair);
		return error(_("bogus config parameter: %s"), text);
	}


// Source: config.c
// Lines 501-523
