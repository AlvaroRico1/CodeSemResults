static int t_show(struct seq_file *m, void *v)
{
	const char **fmt = v;
	const char *str = *fmt;
	int i;

	if (!*fmt)
		return 0;

	seq_printf(m, "0x%lx : \"", *(unsigned long *)fmt);

	/*
	 * Tabs and new lines need to be converted.
	 */
	for (i = 0; str[i]; i++) {
		switch (str[i]) {
		case '\n':
			seq_puts(m, "\\n");
			break;
		case '\t':
			seq_puts(m, "\\t");
			break;
		case '\\':
			seq_putc(m, '\\');
			break;
		case '"':
			seq_puts(m, "\\\"");
			break;
		default:
			seq_putc(m, str[i]);
		}
	}
	seq_puts(m, "\"\n");

	return 0;
}


// Source: trace_printk.c
// Lines 299-334
