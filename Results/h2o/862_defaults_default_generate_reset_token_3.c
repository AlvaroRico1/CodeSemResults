static int default_generate_reset_token(quicly_cid_encryptor_t *_self, void *token, const void *cid)
{
    struct st_quicly_default_encrypt_cid_t *self = (void *)_self;
    generate_reset_token(self, token, cid);
    return 1;
}


// Source: defaults.c
// Lines 180-185
