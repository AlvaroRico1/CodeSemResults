static int parse_tristate(int *b, const char *k, const char *v)
{
	if (v && !strcasecmp(v, "auto"))
		*b = -1;
	else
		*b = git_config_bool(k, v);
	return 0;
}


// Source: userdiff.c
// Lines 323-330
