static int ext4_getfsmap_helper(struct super_block *sb,
				struct ext4_getfsmap_info *info,
				struct ext4_fsmap *rec)
{
	struct ext4_fsmap fmr;
	struct ext4_sb_info *sbi = EXT4_SB(sb);
	ext4_fsblk_t rec_fsblk = rec->fmr_physical;
	ext4_group_t agno;
	ext4_grpblk_t cno;
	int error;

	if (fatal_signal_pending(current))
		return -EINTR;

	/*
	 * Filter out records that start before our startpoint, if the
	 * caller requested that.
	 */
	if (ext4_getfsmap_rec_before_low_key(info, rec)) {
		rec_fsblk += rec->fmr_length;
		if (info->gfi_next_fsblk < rec_fsblk)
			info->gfi_next_fsblk = rec_fsblk;
		return EXT4_QUERY_RANGE_CONTINUE;
	}

	/* Are we just counting mappings? */
	if (info->gfi_head->fmh_count == 0) {
		if (rec_fsblk > info->gfi_next_fsblk)
			info->gfi_head->fmh_entries++;

		if (info->gfi_last)
			return EXT4_QUERY_RANGE_CONTINUE;

		info->gfi_head->fmh_entries++;

		rec_fsblk += rec->fmr_length;
		if (info->gfi_next_fsblk < rec_fsblk)
			info->gfi_next_fsblk = rec_fsblk;
		return EXT4_QUERY_RANGE_CONTINUE;
	}

	/*
	 * If the record starts past the last physical block we saw,
	 * then we've found a gap.  Report the gap as being owned by
	 * whatever the caller specified is the missing owner.
	 */
	if (rec_fsblk > info->gfi_next_fsblk) {
		if (info->gfi_head->fmh_entries >= info->gfi_head->fmh_count)
			return EXT4_QUERY_RANGE_ABORT;

		ext4_get_group_no_and_offset(sb, info->gfi_next_fsblk,
				&agno, &cno);
		trace_ext4_fsmap_mapping(sb, info->gfi_dev, agno,
				EXT4_C2B(sbi, cno),
				rec_fsblk - info->gfi_next_fsblk,
				EXT4_FMR_OWN_UNKNOWN);

		fmr.fmr_device = info->gfi_dev;
		fmr.fmr_physical = info->gfi_next_fsblk;
		fmr.fmr_owner = EXT4_FMR_OWN_UNKNOWN;
		fmr.fmr_length = rec_fsblk - info->gfi_next_fsblk;
		fmr.fmr_flags = FMR_OF_SPECIAL_OWNER;
		error = info->gfi_formatter(&fmr, info->gfi_format_arg);
		if (error)
			return error;
		info->gfi_head->fmh_entries++;
	}

	if (info->gfi_last)
		goto out;

	/* Fill out the extent we found */
	if (info->gfi_head->fmh_entries >= info->gfi_head->fmh_count)
		return EXT4_QUERY_RANGE_ABORT;

	ext4_get_group_no_and_offset(sb, rec_fsblk, &agno, &cno);
	trace_ext4_fsmap_mapping(sb, info->gfi_dev, agno, EXT4_C2B(sbi, cno),
			rec->fmr_length, rec->fmr_owner);

	fmr.fmr_device = info->gfi_dev;
	fmr.fmr_physical = rec_fsblk;
	fmr.fmr_owner = rec->fmr_owner;
	fmr.fmr_flags = FMR_OF_SPECIAL_OWNER;
	fmr.fmr_length = rec->fmr_length;
	error = info->gfi_formatter(&fmr, info->gfi_format_arg);
	if (error)
		return error;
	info->gfi_head->fmh_entries++;

out:
	rec_fsblk += rec->fmr_length;
	if (info->gfi_next_fsblk < rec_fsblk)
		info->gfi_next_fsblk = rec_fsblk;
	return EXT4_QUERY_RANGE_CONTINUE;
}


// Source: fsmap.c
// Lines 84-178
