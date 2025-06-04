static const char *find_funcname_matching_regexp(xdemitconf_t *xecfg, const char *start,
						 regex_t *regexp)
{
	int reg_error;
	regmatch_t match[1];
	while (1) {
		const char *bol, *eol;
		reg_error = regexec(regexp, start, 1, match, 0);
		if (reg_error == REG_NOMATCH)
			return NULL;
		else if (reg_error) {
			char errbuf[1024];
			regerror(reg_error, regexp, errbuf, 1024);
			die("-L parameter: regexec() failed: %s", errbuf);
		}
		/* determine extent of line matched */
		bol = start+match[0].rm_so;
		eol = start+match[0].rm_eo;
		while (bol > start && *bol != '\n')
			bol--;
		if (*bol == '\n')
			bol++;
		while (*eol && *eol != '\n')
			eol++;
		if (*eol == '\n')
			eol++;
		/* is it a funcname line? */
		if (match_funcname(xecfg, (char*) bol, (char*) eol))
			return bol;
		start = eol;
	}


// Source: line-range.c
// Lines 133-163
