TailoredSet::addPrefix(const CollationData *d, const UnicodeString &pfx, UChar32 c, uint32_t ce32) {
    setPrefix(pfx);
    ce32 = d->getFinalCE32(ce32);
    if(Collation::isContractionCE32(ce32)) {
        const UChar *p = d->contexts + Collation::indexFromCE32(ce32);
        addContractions(c, p + 2);
    }


// Source: collationsets.cpp
// Lines 317-323
