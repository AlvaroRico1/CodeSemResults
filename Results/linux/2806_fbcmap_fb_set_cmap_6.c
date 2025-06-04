int fb_set_cmap(struct fb_cmap *cmap, struct fb_info *info)
{
	int i, start, rc = 0;
	u16 *red, *green, *blue, *transp;
	u_int hred, hgreen, hblue, htransp = 0xffff;

	red = cmap->red;
	green = cmap->green;
	blue = cmap->blue;
	transp = cmap->transp;
	start = cmap->start;

	if (start < 0 || (!info->fbops->fb_setcolreg &&
			  !info->fbops->fb_setcmap))
		return -EINVAL;
	if (info->fbops->fb_setcmap) {
		rc = info->fbops->fb_setcmap(cmap, info);
	} else {
		for (i = 0; i < cmap->len; i++) {
			hred = *red++;
			hgreen = *green++;
			hblue = *blue++;
			if (transp)
				htransp = *transp++;
			if (info->fbops->fb_setcolreg(start++,
						      hred, hgreen, hblue,
						      htransp, info))
				break;
		}
	}
	if (rc == 0)
		fb_copy_cmap(cmap, &info->cmap);

	return rc;
}


// Source: fbcmap.c
// Lines 231-265
