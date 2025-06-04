static int sysctl_err(const char *path, struct ctl_table *table, char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;

	pr_err("sysctl table check failed: %s/%s %pV\n",
	       path, table->procname, &vaf);

	va_end(args);
	return -EINVAL;
}


// Source: proc_sysctl.c
// Lines 1099-1113
