int fb_parse_edid(unsigned char *edid, struct fb_var_screeninfo *var)
{
	int i;
	unsigned char *block;

	if (edid == NULL || var == NULL)
		return 1;

	if (!(edid_checksum(edid)))
		return 1;

	if (!(edid_check_header(edid)))
		return 1;

	block = edid + DETAILED_TIMING_DESCRIPTIONS_START;

	for (i = 0; i < 4; i++, block += DETAILED_TIMING_DESCRIPTION_SIZE) {
		if (edid_is_timing_block(block)) {
			var->xres = var->xres_virtual = H_ACTIVE;
			var->yres = var->yres_virtual = V_ACTIVE;
			var->height = var->width = 0;
			var->right_margin = H_SYNC_OFFSET;
			var->left_margin = (H_ACTIVE + H_BLANKING) -
				(H_ACTIVE + H_SYNC_OFFSET + H_SYNC_WIDTH);
			var->upper_margin = V_BLANKING - V_SYNC_OFFSET -
				V_SYNC_WIDTH;
			var->lower_margin = V_SYNC_OFFSET;
			var->hsync_len = H_SYNC_WIDTH;
			var->vsync_len = V_SYNC_WIDTH;
			var->pixclock = PIXEL_CLOCK;
			var->pixclock /= 1000;
			var->pixclock = KHZ2PICOS(var->pixclock);

			if (HSYNC_POSITIVE)
				var->sync |= FB_SYNC_HOR_HIGH_ACT;
			if (VSYNC_POSITIVE)
				var->sync |= FB_SYNC_VERT_HIGH_ACT;
			return 0;
		}
	}
	return 1;
}


// Source: fbmon.c
// Lines 893-934
