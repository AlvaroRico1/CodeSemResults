static int set_method(const char *name)
{
	int k;

	if (!name || !*name)
		name = "printf";

	for (k=0; (s_methods[k].name); k++) {
		if (strcmp(name, s_methods[k].name) == 0) {
			s_trace_method = &s_methods[k];
			return 0;
		}
	}
	fprintf(stderr, "Unknown CLAR_TRACE_METHOD: '%s'\n", name);
	return -1;
}


// Source: clar_libgit2_trace.c
// Lines 74-89
