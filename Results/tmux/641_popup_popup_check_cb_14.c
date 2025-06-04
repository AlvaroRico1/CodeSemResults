popup_check_cb(struct client* c, void *data, u_int px, u_int py, u_int nx,
    struct overlay_ranges *r)
{
	struct popup_data	*pd = data;
	struct overlay_ranges	 or[2];
	u_int			 i, j, k = 0;

	if (pd->md != NULL) {
		/* Check each returned range for the menu against the popup. */
		menu_check_cb(c, pd->md, px, py, nx, r);
		for (i = 0; i < 2; i++) {
			server_client_overlay_range(pd->px, pd->py, pd->sx,
			    pd->sy, r->px[i], py, r->nx[i], &or[i]);
		}

		/*
		 * or has up to OVERLAY_MAX_RANGES non-overlapping ranges,
		 * ordered from left to right. Collect them in the output.
		 */
		for (i = 0; i < 2; i++) {
			/* Each or[i] only has 2 ranges. */
			for (j = 0; j < 2; j++) {
				if (or[i].nx[j] > 0) {
					r->px[k] = or[i].px[j];
					r->nx[k] = or[i].nx[j];
					k++;
				}
			}
		}

		/* Zero remaining ranges if any. */
		for (i = k; i < OVERLAY_MAX_RANGES; i++) {
			r->px[i] = 0;
			r->nx[i] = 0;
		}

		return;
	}

	server_client_overlay_range(pd->px, pd->py, pd->sx, pd->sy, px, py, nx,
	    r);
}


// Source: popup.c
// Lines 163-204
