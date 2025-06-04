static void audit_actions_logged(u32 actions_logged, u32 old_actions_logged,
				 int ret)
{
	char names[sizeof(seccomp_actions_avail)];
	char old_names[sizeof(seccomp_actions_avail)];
	const char *new = names;
	const char *old = old_names;

	if (!audit_enabled)
		return;

	memset(names, 0, sizeof(names));
	memset(old_names, 0, sizeof(old_names));

	if (ret)
		new = "?";
	else if (!actions_logged)
		new = "(none)";
	else if (!seccomp_names_from_actions_logged(names, sizeof(names),
						    actions_logged, ","))
		new = "?";

	if (!old_actions_logged)
		old = "(none)";
	else if (!seccomp_names_from_actions_logged(old_names,
						    sizeof(old_names),
						    old_actions_logged, ","))
		old = "?";

	return audit_seccomp_actions_logged(new, old, !ret);
}


// Source: seccomp.c
// Lines 1718-1748
