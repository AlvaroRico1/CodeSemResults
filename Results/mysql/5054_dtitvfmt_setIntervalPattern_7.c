DateIntervalFormat::setIntervalPattern(UCalendarDateFields field,
                                       const UnicodeString* skeleton,
                                       const UnicodeString* bestSkeleton,
                                       int8_t differenceInfo,
                                       UnicodeString* extendedSkeleton,
                                       UnicodeString* extendedBestSkeleton) {
    UErrorCode status = U_ZERO_ERROR;
    // following getIntervalPattern() should not generate error status
    UnicodeString pattern;
    fInfo->getIntervalPattern(*bestSkeleton, field, pattern, status);
    if ( pattern.isEmpty() ) {
        // single date
        if ( SimpleDateFormat::isFieldUnitIgnored(*bestSkeleton, field) ) {
            // do nothing, format will handle it
            return false;
        }

        // for 24 hour system, interval patterns in resource file
        // might not include pattern when am_pm differ,
        // which should be the same as hour differ.
        // add it here for simplicity
        if ( field == UCAL_AM_PM ) {
            fInfo->getIntervalPattern(*bestSkeleton, UCAL_HOUR, pattern,status);
            if ( !pattern.isEmpty() ) {
                setIntervalPattern(field, pattern);
            }
            return false;
        }
        // else, looking for pattern when 'y' differ for 'dMMMM' skeleton,
        // first, get best match pattern "MMMd",
        // since there is no pattern for 'y' differs for skeleton 'MMMd',
        // need to look for it from skeleton 'yMMMd',
        // if found, adjust field width in interval pattern from
        // "MMM" to "MMMM".
        UChar fieldLetter = fgCalendarFieldToPatternLetter[field];
        if ( extendedSkeleton ) {
            *extendedSkeleton = *skeleton;
            *extendedBestSkeleton = *bestSkeleton;
            extendedSkeleton->insert(0, fieldLetter);
            extendedBestSkeleton->insert(0, fieldLetter);
            // for example, looking for patterns when 'y' differ for
            // skeleton "MMMM".
            fInfo->getIntervalPattern(*extendedBestSkeleton,field,pattern,status);
            if ( pattern.isEmpty() && differenceInfo == 0 ) {
                // if there is no skeleton "yMMMM" defined,
                // look for the best match skeleton, for example: "yMMM"
                const UnicodeString* tmpBest = fInfo->getBestSkeleton(
                                        *extendedBestSkeleton, differenceInfo);
                if ( tmpBest != 0 && differenceInfo != -1 ) {
                    fInfo->getIntervalPattern(*tmpBest, field, pattern, status);
                    bestSkeleton = tmpBest;
                }
            }


// Source: dtitvfmt.cpp
// Lines 1258-1310
