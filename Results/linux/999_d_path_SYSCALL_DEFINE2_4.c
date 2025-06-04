SYSCALL_DEFINE2(getcwd, char __user *, buf, unsigned long, size)
{
	int error;
	struct path pwd, root;
	char *page = __getname();

	if (!page)
		return -ENOMEM;

	rcu_read_lock();
	get_fs_root_and_pwd_rcu(current->fs, &root, &pwd);

	error = -ENOENT;
	if (!d_unlinked(pwd.dentry)) {
		unsigned long len;
		char *cwd = page + PATH_MAX;
		int buflen = PATH_MAX;

		prepend(&cwd, &buflen, "\0", 1);
		error = prepend_path(&pwd, &root, &cwd, &buflen);
		rcu_read_unlock();

		if (error < 0)
			goto out;

		/* Unreachable from current root */
		if (error > 0) {
			error = prepend_unreachable(&cwd, &buflen);
			if (error)
				goto out;
		}

		error = -ERANGE;
		len = PATH_MAX + page - cwd;
		if (len <= size) {
			error = len;
			if (copy_to_user(buf, cwd, len))
				error = -EFAULT;
		}
	} else {
		rcu_read_unlock();
	}


// Source: d_path.c
// Lines 423-464
