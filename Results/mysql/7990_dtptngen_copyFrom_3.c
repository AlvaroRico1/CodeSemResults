PatternMap::copyFrom(const PatternMap& other, UErrorCode& status) {
    if (U_FAILURE(status)) {
        return;
    }
    this->isDupAllowed = other.isDupAllowed;
    for (int32_t bootIndex = 0; bootIndex < MAX_PATTERN_ENTRIES; ++bootIndex) {
        PtnElem *curElem, *otherElem, *prevElem=nullptr;
        otherElem = other.boot[bootIndex];
        while (otherElem != nullptr) {
            LocalPointer<PtnElem> newElem(new PtnElem(otherElem->basePattern, otherElem->pattern), status);
            if (U_FAILURE(status)) {
                return; // out of memory
            }
            newElem->skeleton.adoptInsteadAndCheckErrorCode(new PtnSkeleton(*(otherElem->skeleton)), status);
            if (U_FAILURE(status)) {
                return; // out of memory
            }
            newElem->skeletonWasSpecified = otherElem->skeletonWasSpecified;

            // Release ownership from the LocalPointer of the PtnElem object.
            // The PtnElem will now be owned by either the boot (for the first entry in the linked-list)
            // or owned by the previous PtnElem object in the linked-list.
            curElem = newElem.orphan();

            if (this->boot[bootIndex] == nullptr) {
                this->boot[bootIndex] = curElem;
            } else {
                if (prevElem != nullptr) {
                    prevElem->next.adoptInstead(curElem);
                } else {
                    UPRV_UNREACHABLE;
                }
            }
            prevElem = curElem;
            otherElem = otherElem->next.getAlias();
        }

    }


// Source: dtptngen.cpp
// Lines 1835-1872
