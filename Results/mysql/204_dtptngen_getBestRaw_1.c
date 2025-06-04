DateTimePatternGenerator::getBestRaw(DateTimeMatcher& source,
                                     int32_t includeMask,
                                     DistanceInfo* missingFields,
                                     UErrorCode &status,
                                     const PtnSkeleton** specifiedSkeletonPtr) {
    int32_t bestDistance = 0x7fffffff;
    DistanceInfo tempInfo;
    const UnicodeString *bestPattern=nullptr;
    const PtnSkeleton* specifiedSkeleton=nullptr;

    PatternMapIterator it(status);
    if (U_FAILURE(status)) { return nullptr; }

    for (it.set(*patternMap); it.hasNext(); ) {
        DateTimeMatcher trial = it.next();
        if (trial.equals(skipMatcher)) {
            continue;
        }
        int32_t distance=source.getDistance(trial, includeMask, tempInfo);
        if (distance<bestDistance) {
            bestDistance=distance;
            bestPattern=patternMap->getPatternFromSkeleton(*trial.getSkeletonPtr(), &specifiedSkeleton);
            missingFields->setTo(tempInfo);
            if (distance==0) {
                break;
            }
        }
    }

    // If the best raw match had a specified skeleton and that skeleton was requested by the caller,
    // then return it too. This generally happens when the caller needs to pass that skeleton
    // through to adjustFieldTypes so the latter can do a better job.
    if (bestPattern && specifiedSkeletonPtr) {
        *specifiedSkeletonPtr = specifiedSkeleton;
    }
    return bestPattern;
}


// Source: dtptngen.cpp
// Lines 1476-1512
