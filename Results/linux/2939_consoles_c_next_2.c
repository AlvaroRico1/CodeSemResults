static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct console *con = v;
	++*pos;
	return con->next;
}


// Source: consoles.c
// Lines 74-79
