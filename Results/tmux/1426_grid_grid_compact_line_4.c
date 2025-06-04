grid_compact_line(struct grid_line *gl)
{
	int			 new_extdsize = 0;
	struct grid_extd_entry	*new_extddata;
	struct grid_cell_entry	*gce;
	struct grid_extd_entry	*gee;
	u_int			 px, idx;

	if (gl->extdsize == 0)
		return;

	for (px = 0; px < gl->cellsize; px++) {
		gce = &gl->celldata[px];
		if (gce->flags & GRID_FLAG_EXTENDED)
			new_extdsize++;
	}

	if (new_extdsize == 0) {
		free(gl->extddata);
		gl->extddata = NULL;
		gl->extdsize = 0;
		return;
	}
	new_extddata = xreallocarray(NULL, new_extdsize, sizeof *gl->extddata);

	idx = 0;
	for (px = 0; px < gl->cellsize; px++) {
		gce = &gl->celldata[px];
		if (gce->flags & GRID_FLAG_EXTENDED) {
			gee = &gl->extddata[gce->offset];
			memcpy(&new_extddata[idx], gee, sizeof *gee);
			gce->offset = idx++;
		}
	}

	free(gl->extddata);
	gl->extddata = new_extddata;
	gl->extdsize = new_extdsize;
}


// Source: grid.c
// Lines 139-177
