mode_tree_key(struct mode_tree_data *mtd, struct client *c, key_code *key,
    struct mouse_event *m, u_int *xp, u_int *yp)
{
	struct mode_tree_line	*line;
	struct mode_tree_item	*current, *parent, *mti;
	u_int			 i, x, y;
	int			 choice;

	if (KEYC_IS_MOUSE(*key) && m != NULL) {
		if (cmd_mouse_at(mtd->wp, m, &x, &y, 0) != 0) {
			*key = KEYC_NONE;
			return (0);
		}
		if (xp != NULL)
			*xp = x;
		if (yp != NULL)
			*yp = y;
		if (x > mtd->width || y > mtd->height) {
			if (*key == KEYC_MOUSEDOWN3_PANE)
				mode_tree_display_menu(mtd, c, x, y, 1);
			if (!mtd->preview)
				*key = KEYC_NONE;
			return (0);
		}
		if (mtd->offset + y < mtd->line_size) {
			if (*key == KEYC_MOUSEDOWN1_PANE ||
			    *key == KEYC_MOUSEDOWN3_PANE ||
			    *key == KEYC_DOUBLECLICK1_PANE)
				mtd->current = mtd->offset + y;
			if (*key == KEYC_DOUBLECLICK1_PANE)
				*key = '\r';
			else {
				if (*key == KEYC_MOUSEDOWN3_PANE)
					mode_tree_display_menu(mtd, c, x, y, 0);
				*key = KEYC_NONE;
			}
		} else {
			if (*key == KEYC_MOUSEDOWN3_PANE)
				mode_tree_display_menu(mtd, c, x, y, 0);
			*key = KEYC_NONE;
		}
		return (0);
	}

	line = &mtd->line_list[mtd->current];
	current = line->item;

	choice = -1;
	for (i = 0; i < mtd->line_size; i++) {
		if (*key == mtd->line_list[i].item->key) {
			choice = i;
			break;
		}
	}
	if (choice != -1) {
		if ((u_int)choice > mtd->line_size - 1) {
			*key = KEYC_NONE;
			return (0);
		}
		mtd->current = choice;
		*key = '\r';
		return (0);
	}

	switch (*key) {
	case 'q':
	case '\033': /* Escape */
	case '\007': /* C-g */
		return (1);
	case KEYC_UP:
	case 'k':
	case KEYC_WHEELUP_PANE:
	case '\020': /* C-p */
		mode_tree_up(mtd, 1);
		break;
	case KEYC_DOWN:
	case 'j':
	case KEYC_WHEELDOWN_PANE:
	case '\016': /* C-n */
		mode_tree_down(mtd, 1);
		break;
	case 'g':
	case KEYC_PPAGE:
	case '\002': /* C-b */
		for (i = 0; i < mtd->height; i++) {
			if (mtd->current == 0)
				break;
			mode_tree_up(mtd, 1);
		}
		break;
	case 'G':
	case KEYC_NPAGE:
	case '\006': /* C-f */
		for (i = 0; i < mtd->height; i++) {
			if (mtd->current == mtd->line_size - 1)
				break;
			mode_tree_down(mtd, 1);
		}
		break;
	case KEYC_HOME:
		mtd->current = 0;
		mtd->offset = 0;
		break;
	case KEYC_END:
		mtd->current = mtd->line_size - 1;
		if (mtd->current > mtd->height - 1)
			mtd->offset = mtd->current - mtd->height + 1;
		else
			mtd->offset = 0;
		break;
	case 't':
		/*
		 * Do not allow parents and children to both be tagged: untag
		 * all parents and children of current.
		 */
		if (current->no_tag)
			break;
		if (!current->tagged) {
			parent = current->parent;
			while (parent != NULL) {
				parent->tagged = 0;
				parent = parent->parent;
			}
			mode_tree_clear_tagged(&current->children);
			current->tagged = 1;
		} else
			current->tagged = 0;
		if (m != NULL)
			mode_tree_down(mtd, 0);
		break;
	case 'T':
		for (i = 0; i < mtd->line_size; i++)
			mtd->line_list[i].item->tagged = 0;
		break;
	case '\024': /* C-t */
		for (i = 0; i < mtd->line_size; i++) {
			if ((mtd->line_list[i].item->parent == NULL &&
			    !mtd->line_list[i].item->no_tag) ||
			    (mtd->line_list[i].item->parent != NULL &&
			    mtd->line_list[i].item->parent->no_tag))
				mtd->line_list[i].item->tagged = 1;
			else
				mtd->line_list[i].item->tagged = 0;
		}
		break;
	case 'O':
		mtd->sort_crit.field++;
		if (mtd->sort_crit.field >= mtd->sort_size)
			mtd->sort_crit.field = 0;
		mode_tree_build(mtd);
		break;
	case 'r':
		mtd->sort_crit.reversed = !mtd->sort_crit.reversed;
		mode_tree_build(mtd);
		break;
	case KEYC_LEFT:
	case 'h':
	case '-':
		if (line->flat || !current->expanded)
			current = current->parent;
		if (current == NULL)
			mode_tree_up(mtd, 0);
		else {
			current->expanded = 0;
			mtd->current = current->line;
			mode_tree_build(mtd);
		}
		break;
	case KEYC_RIGHT:
	case 'l':
	case '+':
		if (line->flat || current->expanded)
			mode_tree_down(mtd, 0);
		else if (!line->flat) {
			current->expanded = 1;
			mode_tree_build(mtd);
		}
		break;
	case '-'|KEYC_META:
		TAILQ_FOREACH(mti, &mtd->children, entry)
			mti->expanded = 0;
		mode_tree_build(mtd);
		break;
	case '+'|KEYC_META:
		TAILQ_FOREACH(mti, &mtd->children, entry)
			mti->expanded = 1;
		mode_tree_build(mtd);
		break;
	case '?':
	case '/':
	case '\023': /* C-s */
		mtd->references++;
		status_prompt_set(c, NULL, "(search) ", "",
		    mode_tree_search_callback, mode_tree_search_free, mtd,
		    PROMPT_NOFORMAT, PROMPT_TYPE_SEARCH);
		break;
	case 'n':
		mode_tree_search_set(mtd);
		break;
	case 'f':
		mtd->references++;
		status_prompt_set(c, NULL, "(filter) ", mtd->filter,
		    mode_tree_filter_callback, mode_tree_filter_free, mtd,
		    PROMPT_NOFORMAT, PROMPT_TYPE_SEARCH);
		break;
	case 'v':
		mtd->preview = !mtd->preview;
		mode_tree_build(mtd);
		if (mtd->preview)
			mode_tree_check_selected(mtd);
		break;
	}
	return (0);
}


// Source: mode-tree.c
// Lines 977-1190
