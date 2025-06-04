void proc_tty_unregister_driver(struct tty_driver *driver)
{
	struct proc_dir_entry *ent;

	ent = driver->proc_entry;
	if (!ent)
		return;
		
	remove_proc_entry(ent->name, proc_tty_driver);
	
	driver->proc_entry = NULL;
}


// Source: proc_tty.c
// Lines 149-160
