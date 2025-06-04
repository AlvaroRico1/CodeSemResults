static void avc_audit_post_callback(struct audit_buffer *ab, void *a)
{
	struct common_audit_data *ad = a;
	struct selinux_audit_data *sad = ad->selinux_audit_data;
	char *scontext;
	u32 scontext_len;
	int rc;

	rc = security_sid_to_context(sad->state, sad->ssid, &scontext,
				     &scontext_len);
	if (rc)
		audit_log_format(ab, " ssid=%d", sad->ssid);
	else {
		audit_log_format(ab, " scontext=%s", scontext);
		kfree(scontext);
	}

	rc = security_sid_to_context(sad->state, sad->tsid, &scontext,
				     &scontext_len);
	if (rc)
		audit_log_format(ab, " tsid=%d", sad->tsid);
	else {
		audit_log_format(ab, " tcontext=%s", scontext);
		kfree(scontext);
	}

	audit_log_format(ab, " tclass=%s", secclass_map[sad->tclass-1].name);

	if (sad->denied)
		audit_log_format(ab, " permissive=%u", sad->result ? 0 : 1);

	/* in case of invalid context report also the actual context string */
	rc = security_sid_to_context_inval(sad->state, sad->ssid, &scontext,
					   &scontext_len);
	if (!rc && scontext) {
		if (scontext_len && scontext[scontext_len - 1] == '\0')
			scontext_len--;
		audit_log_format(ab, " srawcon=");
		audit_log_n_untrustedstring(ab, scontext, scontext_len);
		kfree(scontext);
	}

	rc = security_sid_to_context_inval(sad->state, sad->tsid, &scontext,
					   &scontext_len);
	if (!rc && scontext) {
		if (scontext_len && scontext[scontext_len - 1] == '\0')
			scontext_len--;
		audit_log_format(ab, " trawcon=");
		audit_log_n_untrustedstring(ab, scontext, scontext_len);
		kfree(scontext);
	}
}


// Source: avc.c
// Lines 704-755
