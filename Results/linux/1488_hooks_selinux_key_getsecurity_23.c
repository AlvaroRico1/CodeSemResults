static int selinux_key_getsecurity(struct key *key, char **_buffer)
{
	struct key_security_struct *ksec = key->security;
	char *context = NULL;
	unsigned len;
	int rc;

	rc = security_sid_to_context(&selinux_state, ksec->sid,
				     &context, &len);
	if (!rc)
		rc = len;
	*_buffer = context;
	return rc;
}


// Source: hooks.c
// Lines 6522-6535
