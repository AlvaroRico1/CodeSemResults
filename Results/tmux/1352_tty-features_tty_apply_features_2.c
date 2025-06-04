tty_apply_features(struct tty_term *term, int feat)
{
	const struct tty_feature	 *tf;
	const char			**capability;
	u_int				  i;

	if (feat == 0)
		return (0);
	log_debug("applying terminal features: %s", tty_get_features(feat));

	for (i = 0; i < nitems(tty_features); i++) {
		if ((term->features & (1 << i)) || (~feat & (1 << i)))
			continue;
		tf = tty_features[i];

		log_debug("applying terminal feature: %s", tf->name);
		if (tf->capabilities != NULL) {
			capability = tf->capabilities;
			while (*capability != NULL) {
				log_debug("adding capability: %s", *capability);
				tty_term_apply(term, *capability, 1);
				capability++;
			}
		}
		term->flags |= tf->flags;
	}
	if ((term->features | feat) == term->features)
		return (0);
	term->features |= feat;
	return (1);
}


// Source: tty-features.c
// Lines 311-341
