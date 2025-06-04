static bool generate_lm2_response(ntlm_client *ntlm,
	unsigned char ntlm2_hash[NTLM_NTLM2_HASH_LEN])
{
	unsigned char lm2_challengehash[16] = {0};
	size_t lm2_len = 16;
	uint64_t local_nonce;

	local_nonce = ntlm_htonll(ntlm->nonce);

	if (!ntlm_hmac_md5_init(ntlm, ntlm2_hash, NTLM_NTLM2_HASH_LEN) ||
		!ntlm_hmac_md5_update(ntlm, (const unsigned char *)&ntlm->challenge.nonce, 8) ||
		!ntlm_hmac_md5_update(ntlm, (const unsigned char *)&local_nonce, 8) ||
		!ntlm_hmac_md5_final(lm2_challengehash, &lm2_len, ntlm)) {
		ntlm_client_set_errmsg(ntlm, "failed to create HMAC-MD5");
		return false;
	}

	NTLM_ASSERT(ntlm, lm2_len == 16);

	memcpy(&ntlm->lm_response[0], lm2_challengehash, 16);
	memcpy(&ntlm->lm_response[16], &local_nonce, 8);

	ntlm->lm_response_len = 24;
	return true;
}


// Source: ntlm.c
// Lines 1146-1170
