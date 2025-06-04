static ssize_t sel_write_avc_cache_threshold(struct file *file,
					     const char __user *buf,
					     size_t count, loff_t *ppos)

{
	struct selinux_fs_info *fsi = file_inode(file)->i_sb->s_fs_info;
	struct selinux_state *state = fsi->state;
	char *page;
	ssize_t ret;
	unsigned int new_value;

	ret = avc_has_perm(&selinux_state,
			   current_sid(), SECINITSID_SECURITY,
			   SECCLASS_SECURITY, SECURITY__SETSECPARAM,
			   NULL);
	if (ret)
		return ret;

	if (count >= PAGE_SIZE)
		return -ENOMEM;

	/* No partial writes. */
	if (*ppos != 0)
		return -EINVAL;

	page = memdup_user_nul(buf, count);
	if (IS_ERR(page))
		return PTR_ERR(page);

	ret = -EINVAL;
	if (sscanf(page, "%u", &new_value) != 1)
		goto out;

	avc_set_cache_threshold(state->avc, new_value);

	ret = count;
out:
	kfree(page);
	return ret;
}


// Source: selinuxfs.c
// Lines 1428-1467
