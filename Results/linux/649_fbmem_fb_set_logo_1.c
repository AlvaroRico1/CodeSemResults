static void fb_set_logo(struct fb_info *info,
			       const struct linux_logo *logo, u8 *dst,
			       int depth)
{
	int i, j, k;
	const u8 *src = logo->data;
	u8 xor = (info->fix.visual == FB_VISUAL_MONO01) ? 0xff : 0;
	u8 fg = 1, d;

	switch (fb_get_color_depth(&info->var, &info->fix)) {
	case 1:
		fg = 1;
		break;
	case 2:
		fg = 3;
		break;
	default:
		fg = 7;
		break;
	}

	if (info->fix.visual == FB_VISUAL_MONO01 ||
	    info->fix.visual == FB_VISUAL_MONO10)
		fg = ~((u8) (0xfff << info->var.green.length));

	switch (depth) {
	case 4:
		for (i = 0; i < logo->height; i++)
			for (j = 0; j < logo->width; src++) {
				*dst++ = *src >> 4;
				j++;
				if (j < logo->width) {
					*dst++ = *src & 0x0f;
					j++;
				}
			}
		break;
	case 1:
		for (i = 0; i < logo->height; i++) {
			for (j = 0; j < logo->width; src++) {
				d = *src ^ xor;
				for (k = 7; k >= 0 && j < logo->width; k--) {
					*dst++ = ((d >> k) & 1) ? fg : 0;
					j++;
				}
			}
		}
		break;
	}
}


// Source: fbmem.c
// Lines 272-321
