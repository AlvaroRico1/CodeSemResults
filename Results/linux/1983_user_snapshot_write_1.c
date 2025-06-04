static ssize_t snapshot_write(struct file *filp, const char __user *buf,
                              size_t count, loff_t *offp)
{
	struct snapshot_data *data;
	ssize_t res;
	loff_t pg_offp = *offp & ~PAGE_MASK;

	lock_system_sleep();

	data = filp->private_data;

	if (!pg_offp) {
		res = snapshot_write_next(&data->handle);
		if (res <= 0)
			goto unlock;
	} else {
		res = PAGE_SIZE - pg_offp;
	}

	if (!data_of(data->handle)) {
		res = -EINVAL;
		goto unlock;
	}

	res = simple_write_to_buffer(data_of(data->handle), res, &pg_offp,
			buf, count);
	if (res > 0)
		*offp += res;
unlock:
	unlock_system_sleep();

	return res;
}


// Source: user.c
// Lines 167-199
