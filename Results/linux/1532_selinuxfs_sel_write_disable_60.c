static ssize_t sel_write_disable(struct file *file, const char __user *buf,
				 size_t count, loff_t *ppos)

{
	struct selinux_fs_info *fsi = file_inode(file)->i_sb->s_fs_info;
	char *page;
	ssize_t length;
	int new_value;
	int enforcing;

	if (count >= PAGE_SIZE)
		return -ENOMEM;

	/* No partial writes. */
	if (*ppos != 0)
		return -EINVAL;

	page = memdup_user_nul(buf, count);
	if (IS_ERR(page))
		return PTR_ERR(page);

	length = -EINVAL;
	if (sscanf(page, "%d", &new_value) != 1)
		goto out;

	if (new_value) {
		enforcing = enforcing_enabled(fsi->state);
		length = selinux_disable(fsi->state);
		if (length)
			goto out;
		audit_log(audit_context(), GFP_KERNEL, AUDIT_MAC_STATUS,
			"enforcing=%d old_enforcing=%d auid=%u ses=%u"
			" enabled=%d old-enabled=%d lsm=selinux res=1",
			enforcing, enforcing,
			from_kuid(&init_user_ns, audit_get_loginuid(current)),
			audit_get_sessionid(current), 0, 1);
	}

	length = count;
out:
	kfree(page);
	return length;
}


// Source: selinuxfs.c
// Lines 275-317
