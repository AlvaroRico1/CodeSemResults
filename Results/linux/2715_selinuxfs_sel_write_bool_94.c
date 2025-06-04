static ssize_t sel_write_bool(struct file *filep, const char __user *buf,
			      size_t count, loff_t *ppos)
{
	struct selinux_fs_info *fsi = file_inode(filep)->i_sb->s_fs_info;
	char *page = NULL;
	ssize_t length;
	int new_value;
	unsigned index = file_inode(filep)->i_ino & SEL_INO_MASK;
	const char *name = filep->f_path.dentry->d_name.name;

	if (count >= PAGE_SIZE)
		return -ENOMEM;

	/* No partial writes. */
	if (*ppos != 0)
		return -EINVAL;

	page = memdup_user_nul(buf, count);
	if (IS_ERR(page))
		return PTR_ERR(page);

	mutex_lock(&fsi->mutex);

	length = avc_has_perm(&selinux_state,
			      current_sid(), SECINITSID_SECURITY,
			      SECCLASS_SECURITY, SECURITY__SETBOOL,
			      NULL);
	if (length)
		goto out;

	length = -EINVAL;
	if (index >= fsi->bool_num || strcmp(name,
					     fsi->bool_pending_names[index]))
		goto out;

	length = -EINVAL;
	if (sscanf(page, "%d", &new_value) != 1)
		goto out;

	if (new_value)
		new_value = 1;

	fsi->bool_pending_values[index] = new_value;
	length = count;

out:
	mutex_unlock(&fsi->mutex);
	kfree(page);
	return length;
}


// Source: selinuxfs.c
// Lines 1209-1258
