static int show_console_dev(struct seq_file *m, void *v)
{
	static const struct {
		short flag;
		char name;
	} con_flags[] = {
		{ CON_ENABLED,		'E' },
		{ CON_CONSDEV,		'C' },
		{ CON_BOOT,		'B' },
		{ CON_PRINTBUFFER,	'p' },
		{ CON_BRL,		'b' },
		{ CON_ANYTIME,		'a' },
	};
	char flags[ARRAY_SIZE(con_flags) + 1];
	struct console *con = v;
	unsigned int a;
	dev_t dev = 0;

	if (con->device) {
		const struct tty_driver *driver;
		int index;
		driver = con->device(con, &index);
		if (driver) {
			dev = MKDEV(driver->major, driver->minor_start);
			dev += index;
		}
	}

	for (a = 0; a < ARRAY_SIZE(con_flags); a++)
		flags[a] = (con->flags & con_flags[a].flag) ?
			con_flags[a].name : ' ';
	flags[a] = 0;

	seq_setwidth(m, 21 - 1);
	seq_printf(m, "%s%d", con->name, con->index);
	seq_pad(m, ' ');
	seq_printf(m, "%c%c%c (%s)", con->read ? 'R' : '-',
			con->write ? 'W' : '-', con->unblank ? 'U' : '-',
			flags);
	if (dev)
		seq_printf(m, " %4d:%d", MAJOR(dev), MINOR(dev));

	seq_putc(m, '\n');
	return 0;
}


// Source: consoles.c
// Lines 15-59
