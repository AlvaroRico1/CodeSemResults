charsetMatchComparator(const void * /*context*/, const void *left, const void *right)
{
    U_NAMESPACE_USE

    const CharsetMatch **csm_l = (const CharsetMatch **) left;
    const CharsetMatch **csm_r = (const CharsetMatch **) right;

    // NOTE: compare is backwards to sort from highest to lowest.
    return (*csm_r)->getConfidence() - (*csm_l)->getConfidence();
}


// Source: csdetect.cpp
// Lines 73-82
