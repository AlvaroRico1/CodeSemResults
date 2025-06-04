static int push_additional_extensions(ptls_handshake_properties_t *properties, ptls_buffer_t *sendbuf)
{
    int ret;

    if (properties != NULL && properties->additional_extensions != NULL) {
        ptls_raw_extension_t *ext;
        for (ext = properties->additional_extensions; ext->type != UINT16_MAX; ++ext) {
            buffer_push_extension(sendbuf, ext->type, { ptls_buffer_pushv(sendbuf, ext->data.base, ext->data.len); });
        }
    }


// Source: picotls.c
// Lines 1507-1516
