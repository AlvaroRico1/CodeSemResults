ext4_xattr_set(struct inode *inode, int name_index, const char *name,
	       const void *value, size_t value_len, int flags)
{
	handle_t *handle;
	struct super_block *sb = inode->i_sb;
	int error, retries = 0;
	int credits;

	error = dquot_initialize(inode);
	if (error)
		return error;

retry:
	error = ext4_xattr_set_credits(inode, value_len, flags & XATTR_CREATE,
				       &credits);
	if (error)
		return error;

	handle = ext4_journal_start(inode, EXT4_HT_XATTR, credits);
	if (IS_ERR(handle)) {
		error = PTR_ERR(handle);
	} else {
		int error2;

		error = ext4_xattr_set_handle(handle, inode, name_index, name,
					      value, value_len, flags);
		error2 = ext4_journal_stop(handle);
		if (error == -ENOSPC &&
		    ext4_should_retry_alloc(sb, &retries))
			goto retry;
		if (error == 0)
			error = error2;
	}

	return error;
}


// Source: xattr.c
// Lines 2480-2515
