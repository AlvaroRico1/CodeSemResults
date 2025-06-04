int kernel_read_file(struct file *file, void **buf, loff_t *size,
		     loff_t max_size, enum kernel_read_file_id id)
{
	loff_t i_size, pos;
	ssize_t bytes = 0;
	int ret;

	if (!S_ISREG(file_inode(file)->i_mode) || max_size < 0)
		return -EINVAL;

	ret = deny_write_access(file);
	if (ret)
		return ret;

	ret = security_kernel_read_file(file, id);
	if (ret)
		goto out;

	i_size = i_size_read(file_inode(file));
	if (i_size <= 0) {
		ret = -EINVAL;
		goto out;
	}
	if (i_size > SIZE_MAX || (max_size > 0 && i_size > max_size)) {
		ret = -EFBIG;
		goto out;
	}

	if (id != READING_FIRMWARE_PREALLOC_BUFFER)
		*buf = vmalloc(i_size);
	if (!*buf) {
		ret = -ENOMEM;
		goto out;
	}

	pos = 0;
	while (pos < i_size) {
		bytes = kernel_read(file, *buf + pos, i_size - pos, &pos);
		if (bytes < 0) {
			ret = bytes;
			goto out_free;
		}

		if (bytes == 0)
			break;
	}

	if (pos != i_size) {
		ret = -EIO;
		goto out_free;
	}

	ret = security_kernel_post_read_file(file, *buf, i_size, id);
	if (!ret)
		*size = pos;

out_free:
	if (ret < 0) {
		if (id != READING_FIRMWARE_PREALLOC_BUFFER) {
			vfree(*buf);
			*buf = NULL;
		}
	}

out:
	allow_write_access(file);
	return ret;
}


// Source: exec.c
// Lines 896-963
