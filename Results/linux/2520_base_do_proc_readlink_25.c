static int do_proc_readlink(struct path *path, char __user *buffer, int buflen)
{
	char *tmp = (char *)__get_free_page(GFP_KERNEL);
	char *pathname;
	int len;

	if (!tmp)
		return -ENOMEM;

	pathname = d_path(path, tmp, PAGE_SIZE);
	len = PTR_ERR(pathname);
	if (IS_ERR(pathname))
		goto out;
	len = tmp + PAGE_SIZE - 1 - pathname;

	if (len > buflen)
		len = buflen;
	if (copy_to_user(buffer, pathname, len))
		len = -EFAULT;
 out:
	free_page((unsigned long)tmp);
	return len;
}


// Source: base.c
// Lines 1635-1657
