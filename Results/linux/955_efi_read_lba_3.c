static size_t read_lba(struct parsed_partitions *state,
		       u64 lba, u8 *buffer, size_t count)
{
	size_t totalreadcount = 0;
	struct block_device *bdev = state->bdev;
	sector_t n = lba * (bdev_logical_block_size(bdev) / 512);

	if (!buffer || lba > last_lba(bdev))
                return 0;

	while (count) {
		int copied = 512;
		Sector sect;
		unsigned char *data = read_part_sector(state, n++, &sect);
		if (!data)
			break;
		if (copied > count)
			copied = count;
		memcpy(buffer, data, copied);
		put_dev_sector(sect);
		buffer += copied;
		totalreadcount +=copied;
		count -= copied;
	}


// Source: efi.c
// Lines 237-260
