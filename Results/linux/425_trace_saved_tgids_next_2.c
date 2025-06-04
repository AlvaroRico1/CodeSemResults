static void *saved_tgids_next(struct seq_file *m, void *v, loff_t *pos)
{
	int *ptr = v;

	if (*pos || m->count)
		ptr++;

	(*pos)++;

	for (; ptr <= &tgid_map[PID_MAX_DEFAULT]; ptr++) {
		if (trace_find_tgid(*ptr))
			return ptr;
	}

	return NULL;
}


// Source: trace.c
// Lines 4985-5000
