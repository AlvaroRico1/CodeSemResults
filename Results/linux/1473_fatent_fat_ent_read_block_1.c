static inline int fat_ent_read_block(struct super_block *sb,
				     struct fat_entry *fatent)
{
	const struct fatent_operations *ops = MSDOS_SB(sb)->fatent_ops;
	sector_t blocknr;
	int offset;

	fatent_brelse(fatent);
	ops->ent_blocknr(sb, fatent->entry, &offset, &blocknr);
	return ops->ent_bread(sb, fatent, offset, blocknr);
}


// Source: fatent.c
// Lines 434-444
