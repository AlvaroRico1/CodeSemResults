static int __fat_remove_entries(struct inode *dir, loff_t pos, int nr_slots)
{
	struct super_block *sb = dir->i_sb;
	struct buffer_head *bh;
	struct msdos_dir_entry *de, *endp;
	int err = 0, orig_slots;

	while (nr_slots) {
		bh = NULL;
		if (fat_get_entry(dir, &pos, &bh, &de) < 0) {
			err = -EIO;
			break;
		}

		orig_slots = nr_slots;
		endp = (struct msdos_dir_entry *)(bh->b_data + sb->s_blocksize);
		while (nr_slots && de < endp) {
			de->name[0] = DELETED_FLAG;
			de++;
			nr_slots--;
		}
		mark_buffer_dirty_inode(bh, dir);
		if (IS_DIRSYNC(dir))
			err = sync_dirty_buffer(bh);
		brelse(bh);
		if (err)
			break;

		/* pos is *next* de's position, so this does `- sizeof(de)' */
		pos += ((orig_slots - nr_slots) * sizeof(*de)) - sizeof(*de);
	}

	return err;
}


// Source: dir.c
// Lines 1000-1033
