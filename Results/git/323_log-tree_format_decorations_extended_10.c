void format_decorations_extended(struct strbuf *sb,
			const struct commit *commit,
			int use_color,
			const char *prefix,
			const char *separator,
			const char *suffix)
{
	const struct name_decoration *decoration;
	const struct name_decoration *current_and_HEAD;
	const char *color_commit =
		diff_get_color(use_color, DIFF_COMMIT);
	const char *color_reset =
		decorate_get_color(use_color, DECORATION_NONE);

	decoration = get_name_decoration(&commit->object);
	if (!decoration)
		return;

	current_and_HEAD = current_pointed_by_HEAD(decoration);
	while (decoration) {
		/*
		 * When both current and HEAD are there, only
		 * show HEAD->current where HEAD would have
		 * appeared, skipping the entry for current.
		 */
		if (decoration != current_and_HEAD) {
			strbuf_addstr(sb, color_commit);
			strbuf_addstr(sb, prefix);
			strbuf_addstr(sb, color_reset);
			strbuf_addstr(sb, decorate_get_color(use_color, decoration->type));
			if (decoration->type == DECORATION_REF_TAG)
				strbuf_addstr(sb, "tag: ");

			show_name(sb, decoration);

			if (current_and_HEAD &&
			    decoration->type == DECORATION_REF_HEAD) {
				strbuf_addstr(sb, " -> ");
				strbuf_addstr(sb, color_reset);
				strbuf_addstr(sb, decorate_get_color(use_color, current_and_HEAD->type));
				show_name(sb, current_and_HEAD);
			}
			strbuf_addstr(sb, color_reset);

			prefix = separator;
		}
		decoration = decoration->next;
	}
	strbuf_addstr(sb, color_commit);
	strbuf_addstr(sb, suffix);
	strbuf_addstr(sb, color_reset);
}


// Source: log-tree.c
// Lines 285-336
