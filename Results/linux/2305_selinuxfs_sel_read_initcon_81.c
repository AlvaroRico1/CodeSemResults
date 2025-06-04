static ssize_t sel_read_initcon(struct file *file, char __user *buf,
				size_t count, loff_t *ppos)
{
	struct selinux_fs_info *fsi = file_inode(file)->i_sb->s_fs_info;
	char *con;
	u32 sid, len;
	ssize_t ret;

	sid = file_inode(file)->i_ino&SEL_INO_MASK;
	ret = security_sid_to_context(fsi->state, sid, &con, &len);
	if (ret)
		return ret;

	ret = simple_read_from_buffer(buf, count, ppos, con, len);
	kfree(con);
	return ret;
}


// Source: selinuxfs.c
// Lines 1606-1622
