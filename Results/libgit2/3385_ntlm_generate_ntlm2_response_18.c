static bool generate_ntlm2_response(ntlm_client *ntlm)
{
	size_t blob_len, ntlm2_response_len;
	uint32_t signature;
	uint64_t timestamp, nonce;
	unsigned char ntlm2_hash[NTLM_NTLM2_HASH_LEN];
	unsigned char challengehash[16] = {0};
	unsigned char *blob;

	if (!generate_timestamp(ntlm) ||
		!generate_nonce(ntlm) ||
		!generate_ntlm2_hash(ntlm2_hash, ntlm))
		return false;

	blob_len = ntlm->challenge.target_info_len + 32;
	ntlm2_response_len = blob_len + 16;

	if ((ntlm->ntlm2_response = malloc(ntlm2_response_len)) == NULL) {
		ntlm_client_set_errmsg(ntlm, "out of memory");
		return false;
	}

	/* position the blob in the response; we'll use it then return it */
	blob = ntlm->ntlm2_response + 16;

	/* the blob's integer values are in network byte order */
	signature = htonl(0x01010000);
	timestamp = ntlm_htonll(ntlm->timestamp);
	nonce = ntlm_htonll(ntlm->nonce);

	/* construct the blob */
	memcpy(&blob[0], &signature, 4);
	memset(&blob[4], 0, 4);
	memcpy(&blob[8], &timestamp, 8);
	memcpy(&blob[16], &nonce, 8);
	memset(&blob[24], 0, 4);
	memcpy(&blob[28], ntlm->challenge.target_info, ntlm->challenge.target_info_len);
	memset(&blob[28 + ntlm->challenge.target_info_len], 0, 4);

	if (!generate_ntlm2_challengehash(challengehash, ntlm, ntlm2_hash, blob, blob_len))
		return false;

	memcpy(ntlm->ntlm2_response, challengehash, 16);
	ntlm->ntlm2_response_len = ntlm2_response_len;

	if (!generate_lm2_response(ntlm, ntlm2_hash))
		return false;

	return true;
}


// Source: ntlm.c
// Lines 1194-1243
