PatternMap::add(const UnicodeString& basePattern,
                const PtnSkeleton& skeleton,
                const UnicodeString& value,// mapped pattern value
                UBool skeletonWasSpecified,
                UErrorCode &status) {
    UChar baseChar = basePattern.charAt(0);
    PtnElem *curElem, *baseElem;
    status = U_ZERO_ERROR;

    // the baseChar must be A-Z or a-z
    if ((baseChar >= CAP_A) && (baseChar <= CAP_Z)) {
        baseElem = boot[baseChar-CAP_A];
    }
    else {
        if ((baseChar >=LOW_A) && (baseChar <= LOW_Z)) {
            baseElem = boot[26+baseChar-LOW_A];
         }
         else {
             status = U_ILLEGAL_CHARACTER;
             return;
         }
    }

    if (baseElem == nullptr) {
        LocalPointer<PtnElem> newElem(new PtnElem(basePattern, value), status);
        if (U_FAILURE(status)) {
            return; // out of memory
        }
        newElem->skeleton.adoptInsteadAndCheckErrorCode(new PtnSkeleton(skeleton), status);
        if (U_FAILURE(status)) {
            return; // out of memory
        }
        newElem->skeletonWasSpecified = skeletonWasSpecified;
        if (baseChar >= LOW_A) {
            boot[26 + (baseChar - LOW_A)] = newElem.orphan(); // the boot array now owns the PtnElem.
        }
        else {
            boot[baseChar - CAP_A] = newElem.orphan(); // the boot array now owns the PtnElem.
        }
    }
    if ( baseElem != nullptr ) {
        curElem = getDuplicateElem(basePattern, skeleton, baseElem);

        if (curElem == nullptr) {
            // add new element to the list.
            curElem = baseElem;
            while( curElem -> next != nullptr )
            {
                curElem = curElem->next.getAlias();
            }

            LocalPointer<PtnElem> newElem(new PtnElem(basePattern, value), status);
            if (U_FAILURE(status)) {
                return; // out of memory
            }
            newElem->skeleton.adoptInsteadAndCheckErrorCode(new PtnSkeleton(skeleton), status);
            if (U_FAILURE(status)) {
                return; // out of memory
            }
            newElem->skeletonWasSpecified = skeletonWasSpecified;
            curElem->next.adoptInstead(newElem.orphan());
            curElem = curElem->next.getAlias();
        }
        else {
            // Pattern exists in the list already.
            if ( !isDupAllowed ) {
                return;
            }
            // Overwrite the value.
            curElem->pattern = value;
            // It was a bug that we were not doing the following previously,
            // though that bug hid other problems by making things partly work.
            curElem->skeletonWasSpecified = skeletonWasSpecified;
        }
    }
}  // PatternMap::add


// Source: dtptngen.cpp
// Lines 1903-1978
