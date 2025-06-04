static ssize_t proc_loginuid_write(struct file * file, const char __user * buf,
				   size_t count, loff_t *ppos)
{
	struct inode * inode = file_inode(file);
	uid_t loginuid;
	kuid_t kloginuid;
	int rv;

	rcu_read_lock();
	if (current != pid_task(proc_pid(inode), PIDTYPE_PID)) {
		rcu_read_unlock();
		return -EPERM;
	}
	rcu_read_unlock();

	if (*ppos != 0) {
		/* No partial writes. */
		return -EINVAL;
	}

	rv = kstrtou32_from_user(buf, count, 10, &loginuid);
	if (rv < 0)
		return rv;

	/* is userspace tring to explicitly UNSET the loginuid? */
	if (loginuid == AUDIT_UID_UNSET) {
		kloginuid = INVALID_UID;
	} else {
		kloginuid = make_kuid(file->f_cred->user_ns, loginuid);
		if (!uid_valid(kloginuid))
			return -EINVAL;
	}

	rv = audit_set_loginuid(kloginuid);
	if (rv < 0)
		return rv;
	return count;
}


// Source: base.c
// Lines 1245-1282
