int generic_cont_expand_simple(struct inode *inode, loff_t size)
{
	struct address_space *mapping = inode->i_mapping;
	struct page *page;
	void *fsdata;
	int err;

	err = inode_newsize_ok(inode, size);
	if (err)
		goto out;

	err = pagecache_write_begin(NULL, mapping, size, 0,
				    AOP_FLAG_CONT_EXPAND, &page, &fsdata);
	if (err)
		goto out;

	err = pagecache_write_end(NULL, mapping, size, 0, 0, page, fsdata);
	BUG_ON(err > 0);

out:
	return err;
}


// Source: buffer.c
// Lines 2306-2327
