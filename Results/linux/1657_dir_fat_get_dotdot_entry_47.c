int fat_get_dotdot_entry(struct inode *dir, struct buffer_head **bh,
			 struct msdos_dir_entry **de)
{
	loff_t offset = 0;

	*de = NULL;
	while (fat_get_short_entry(dir, &offset, bh, de) >= 0) {
		if (!strncmp((*de)->name, MSDOS_DOTDOT, MSDOS_NAME))
			return 0;
	}
	return -ENOENT;
}


// Source: dir.c
// Lines 896-907
