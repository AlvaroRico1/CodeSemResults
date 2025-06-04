ZSTD_decompressSequencesLong_body(
                               ZSTD_DCtx* dctx,
                               void* dst, size_t maxDstSize,
                         const void* seqStart, size_t seqSize, int nbSeq,
                         const ZSTD_longOffset_e isLongOffset)
{
    const BYTE* ip = (const BYTE*)seqStart;
    const BYTE* const iend = ip + seqSize;
    BYTE* const ostart = (BYTE* const)dst;
    BYTE* const oend = ostart + maxDstSize;
    BYTE* op = ostart;
    const BYTE* litPtr = dctx->litPtr;
    const BYTE* const litEnd = litPtr + dctx->litSize;
    const BYTE* const prefixStart = (const BYTE*) (dctx->prefixStart);
    const BYTE* const dictStart = (const BYTE*) (dctx->virtualStart);
    const BYTE* const dictEnd = (const BYTE*) (dctx->dictEnd);

    /* Regen sequences */
    if (nbSeq) {
#define STORED_SEQS 4
#define STORED_SEQS_MASK (STORED_SEQS-1)
#define ADVANCED_SEQS 4
        seq_t sequences[STORED_SEQS];
        int const seqAdvance = MIN(nbSeq, ADVANCED_SEQS);
        seqState_t seqState;
        int seqNb;
        dctx->fseEntropy = 1;
        { int i; for (i=0; i<ZSTD_REP_NUM; i++) seqState.prevOffset[i] = dctx->entropy.rep[i]; }
        seqState.prefixStart = prefixStart;
        seqState.pos = (size_t)(op-prefixStart);
        seqState.dictEnd = dictEnd;
        assert(iend >= ip);
        RETURN_ERROR_IF(
            ERR_isError(BIT_initDStream(&seqState.DStream, ip, iend-ip)),
            corruption_detected);
        ZSTD_initFseState(&seqState.stateLL, &seqState.DStream, dctx->LLTptr);
        ZSTD_initFseState(&seqState.stateOffb, &seqState.DStream, dctx->OFTptr);
        ZSTD_initFseState(&seqState.stateML, &seqState.DStream, dctx->MLTptr);

        /* prepare in advance */
        for (seqNb=0; (BIT_reloadDStream(&seqState.DStream) <= BIT_DStream_completed) && (seqNb<seqAdvance); seqNb++) {
            sequences[seqNb] = ZSTD_decodeSequenceLong(&seqState, isLongOffset);
            PREFETCH_L1(sequences[seqNb].match); PREFETCH_L1(sequences[seqNb].match + sequences[seqNb].matchLength - 1); /* note : it's safe to invoke PREFETCH() on any memory address, including invalid ones */
        }
        RETURN_ERROR_IF(seqNb<seqAdvance, corruption_detected);

        /* decode and decompress */
        for ( ; (BIT_reloadDStream(&(seqState.DStream)) <= BIT_DStream_completed) && (seqNb<nbSeq) ; seqNb++) {
            seq_t const sequence = ZSTD_decodeSequenceLong(&seqState, isLongOffset);
            size_t const oneSeqSize = ZSTD_execSequenceLong(op, oend, sequences[(seqNb-ADVANCED_SEQS) & STORED_SEQS_MASK], &litPtr, litEnd, prefixStart, dictStart, dictEnd);
            if (ZSTD_isError(oneSeqSize)) return oneSeqSize;
            PREFETCH_L1(sequence.match); PREFETCH_L1(sequence.match + sequence.matchLength - 1); /* note : it's safe to invoke PREFETCH() on any memory address, including invalid ones */
            sequences[seqNb & STORED_SEQS_MASK] = sequence;
            op += oneSeqSize;
        }
        RETURN_ERROR_IF(seqNb<nbSeq, corruption_detected);

        /* finish queue */
        seqNb -= seqAdvance;
        for ( ; seqNb<nbSeq ; seqNb++) {
            size_t const oneSeqSize = ZSTD_execSequenceLong(op, oend, sequences[seqNb&STORED_SEQS_MASK], &litPtr, litEnd, prefixStart, dictStart, dictEnd);
            if (ZSTD_isError(oneSeqSize)) return oneSeqSize;
            op += oneSeqSize;
        }

        /* save reps for next block */
        { U32 i; for (i=0; i<ZSTD_REP_NUM; i++) dctx->entropy.rep[i] = (U32)(seqState.prevOffset[i]); }
    }


// Source: zstd_decompress_block.c
// Lines 1052-1119
