static ssize_t environ_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	char *page;
	unsigned long src = *ppos;
	int ret = 0;
	struct mm_struct *mm = file->private_data;
	unsigned long env_start, env_end;

	/* Ensure the process spawned far enough to have an environment. */
	if (!mm || !mm->env_end)
		return 0;

	page = (char *)__get_free_page(GFP_KERNEL);
	if (!page)
		return -ENOMEM;

	ret = 0;
	if (!mmget_not_zero(mm))
		goto free;

	spin_lock(&mm->arg_lock);
	env_start = mm->env_start;
	env_end = mm->env_end;
	spin_unlock(&mm->arg_lock);

	while (count > 0) {
		size_t this_len, max_len;
		int retval;

		if (src >= (env_end - env_start))
			break;

		this_len = env_end - (env_start + src);

		max_len = min_t(size_t, PAGE_SIZE, count);
		this_len = min(max_len, this_len);

		retval = access_remote_vm(mm, (env_start + src), page, this_len, FOLL_ANON);

		if (retval <= 0) {
			ret = retval;
			break;
		}

		if (copy_to_user(buf, page, retval)) {
			ret = -EFAULT;
			break;
		}

		ret += retval;
		src += retval;
		buf += retval;
		count -= retval;
	}
	*ppos = src;
	mmput(mm);

free:
	free_page((unsigned long) page);
	return ret;
}


// Source: base.c
// Lines 920-981
