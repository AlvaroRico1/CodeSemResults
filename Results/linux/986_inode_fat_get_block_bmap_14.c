static int fat_get_block_bmap(struct inode *inode, sector_t iblock,
		struct buffer_head *bh_result, int create)
{
	struct super_block *sb = inode->i_sb;
	unsigned long max_blocks = bh_result->b_size >> inode->i_blkbits;
	int err;
	sector_t bmap;
	unsigned long mapped_blocks;

	BUG_ON(create != 0);

	err = fat_bmap(inode, iblock, &bmap, &mapped_blocks, create, true);
	if (err)
		return err;

	if (bmap) {
		map_bh(bh_result, sb, bmap);
		max_blocks = min(mapped_blocks, max_blocks);
	}

	bh_result->b_size = max_blocks << sb->s_blocksize_bits;

	return 0;
}


// Source: inode.c
// Lines 290-313
