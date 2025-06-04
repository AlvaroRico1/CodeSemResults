size_t ZSTD_compress2(ZSTD_CCtx* cctx,
                      void* dst, size_t dstCapacity,
                      const void* src, size_t srcSize)
{
    ZSTD_CCtx_reset(cctx, ZSTD_reset_session_only);
    {   size_t oPos = 0;
        size_t iPos = 0;
        size_t const result = ZSTD_compressStream2_simpleArgs(cctx,
                                        dst, dstCapacity, &oPos,
                                        src, srcSize, &iPos,
                                        ZSTD_e_end);
        FORWARD_IF_ERROR(result);
        if (result != 0) {  /* compression not completed, due to lack of output space */
            assert(oPos == dstCapacity);
            RETURN_ERROR(dstSize_tooSmall);
        }
        assert(iPos == srcSize);   /* all input is expected consumed */
        return oPos;
    }


// Source: zstd_compress.c
// Lines 3717-3735
