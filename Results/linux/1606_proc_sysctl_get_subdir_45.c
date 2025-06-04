static struct ctl_dir *get_subdir(struct ctl_dir *dir,
				  const char *name, int namelen)
{
	struct ctl_table_set *set = dir->header.set;
	struct ctl_dir *subdir, *new = NULL;
	int err;

	spin_lock(&sysctl_lock);
	subdir = find_subdir(dir, name, namelen);
	if (!IS_ERR(subdir))
		goto found;
	if (PTR_ERR(subdir) != -ENOENT)
		goto failed;

	spin_unlock(&sysctl_lock);
	new = new_dir(set, name, namelen);
	spin_lock(&sysctl_lock);
	subdir = ERR_PTR(-ENOMEM);
	if (!new)
		goto failed;

	/* Was the subdir added while we dropped the lock? */
	subdir = find_subdir(dir, name, namelen);
	if (!IS_ERR(subdir))
		goto found;
	if (PTR_ERR(subdir) != -ENOENT)
		goto failed;

	/* Nope.  Use the our freshly made directory entry. */
	err = insert_header(dir, &new->header);
	subdir = ERR_PTR(err);
	if (err)
		goto failed;
	subdir = new;
found:
	subdir->header.nreg++;
failed:
	if (IS_ERR(subdir)) {
		pr_err("sysctl could not get directory: ");
		sysctl_print_dir(dir);
		pr_cont("/%*.*s %ld\n",
			namelen, namelen, name, PTR_ERR(subdir));
	}
	drop_sysctl_table(&dir->header);
	if (new)
		drop_sysctl_table(&new->header);
	spin_unlock(&sysctl_lock);
	return subdir;
}


// Source: proc_sysctl.c
// Lines 1002-1050
