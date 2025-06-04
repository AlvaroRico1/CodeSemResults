static void *saved_cmdlines_next(struct seq_file *m, void *v, loff_t *pos)
{
	unsigned int *ptr = v;

	if (*pos || m->count)
		ptr++;

	(*pos)++;

	for (; ptr < &savedcmd->map_cmdline_to_pid[savedcmd->cmdline_num];
	     ptr++) {
		if (*ptr == -1 || *ptr == NO_CMDLINE_MAP)
			continue;

		return ptr;
	}

	return NULL;
}


// Source: trace.c
// Lines 5055-5073
