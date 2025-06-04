static bool generate_ntlm2_hash(
	unsigned char out[NTLM_NTLM2_HASH_LEN], ntlm_client *ntlm)
{
	unsigned char ntlm_hash[NTLM_NTLM_HASH_LEN] = {0};
	const unsigned char *username = NULL, *target = NULL;
	size_t username_len = 0, target_len = 0, out_len = NTLM_NTLM2_HASH_LEN;

	if (!generate_ntlm_hash(ntlm_hash, ntlm))
		return false;

	if (ntlm->username_upper_utf16) {
		username = (const unsigned char *)ntlm->username_upper_utf16;
		username_len = ntlm->username_upper_utf16_len;
	}

	if (ntlm->target_utf16) {
		target = (const unsigned char *)ntlm->target_utf16;
		target_len = ntlm->target_utf16_len;
	}

	if (!ntlm_hmac_md5_init(ntlm, ntlm_hash, sizeof(ntlm_hash)) ||
		!ntlm_hmac_md5_update(ntlm, username, username_len) ||
		!ntlm_hmac_md5_update(ntlm, target, target_len) ||
		!ntlm_hmac_md5_final(out, &out_len, ntlm)) {
		ntlm_client_set_errmsg(ntlm, "failed to create HMAC-MD5");
		return false;
	}

	NTLM_ASSERT(ntlm, out_len == NTLM_NTLM2_HASH_LEN);
	return true;
}


// Source: ntlm.c
// Lines 1093-1123
