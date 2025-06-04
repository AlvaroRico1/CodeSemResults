enumCnERange(const void *context, UChar32 start, UChar32 end, uint32_t ce32) {
    ContractionsAndExpansions *cne = (ContractionsAndExpansions *)context;
    if(cne->checkTailored == 0) {
        // There is no tailoring.
        // No need to collect nor check the tailored set.
    } else if(cne->checkTailored < 0) {
        // Collect the set of code points with mappings in the tailoring data.
        if(ce32 == Collation::FALLBACK_CE32) {
            return TRUE;  // fallback to base, not tailored
        } else {
            cne->tailored.add(start, end);
        }
        // checkTailored > 0: Exclude tailored ranges from the base data enumeration.
    } else if(start == end) {
        if(cne->tailored.contains(start)) {
            return TRUE;
        }
    } else if(cne->tailored.containsSome(start, end)) {
        cne->ranges.set(start, end).removeAll(cne->tailored);
        int32_t count = cne->ranges.getRangeCount();
        for(int32_t i = 0; i < count; ++i) {
            cne->handleCE32(cne->ranges.getRangeStart(i), cne->ranges.getRangeEnd(i), ce32);
        }
        return U_SUCCESS(cne->errorCode);
    }
    cne->handleCE32(start, end, ce32);
    return U_SUCCESS(cne->errorCode);
}


// Source: collationsets.cpp
// Lines 360-387
