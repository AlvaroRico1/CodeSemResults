static void color_imageblit(const struct fb_image *image, struct fb_info *p,
			    void *dst1, u32 start_index, u32 pitch_index)
{
	/* Draw the penguin */
	u32 *dst, *dst2;
	u32 color = 0, val, shift;
	int i, n, bpp = p->var.bits_per_pixel;
	u32 null_bits = 32 - bpp;
	u32 *palette = (u32 *) p->pseudo_palette;
	const u8 *src = image->data;

	dst2 = dst1;
	for (i = image->height; i--; ) {
		n = image->width;
		dst = dst1;
		shift = 0;
		val = 0;

		if (start_index) {
			u32 start_mask = ~(FB_SHIFT_HIGH(p, ~(u32)0,
							 start_index));
			val = *dst & start_mask;
			shift = start_index;
		}
		while (n--) {
			if (p->fix.visual == FB_VISUAL_TRUECOLOR ||
			    p->fix.visual == FB_VISUAL_DIRECTCOLOR )
				color = palette[*src];
			else
				color = *src;
			color <<= FB_LEFT_POS(p, bpp);
			val |= FB_SHIFT_HIGH(p, color, shift);
			if (shift >= null_bits) {
				*dst++ = val;

				val = (shift == null_bits) ? 0 :
					FB_SHIFT_LOW(p, color, 32 - shift);
			}
			shift += bpp;
			shift &= (32 - 1);
			src++;
		}
		if (shift) {
			u32 end_mask = FB_SHIFT_HIGH(p, ~(u32)0, shift);

			*dst &= end_mask;
			*dst |= val;
		}
		dst1 += p->fix.line_length;
		if (pitch_index) {
			dst2 += p->fix.line_length;
			dst1 = (u8 *)((long)dst2 & ~(sizeof(u32) - 1));

			start_index += pitch_index;
			start_index &= 32 - 1;
		}
	}
}


// Source: sysimgblt.c
// Lines 52-109
