void proc_tty_register_driver(struct tty_driver *driver)
{
	struct proc_dir_entry *ent;
		
	if (!driver->driver_name || driver->proc_entry ||
	    !driver->ops->proc_show)
		return;

	ent = proc_create_single_data(driver->driver_name, 0, proc_tty_driver,
			       driver->ops->proc_show, driver);
	driver->proc_entry = ent;
}


// Source: proc_tty.c
// Lines 133-144
