DateIntervalFormat::operator==(const Format& other) const {
    if (typeid(*this) != typeid(other)) {return FALSE;}
    const DateIntervalFormat* fmt = (DateIntervalFormat*)&other;
    if (this == fmt) {return TRUE;}
    if (!Format::operator==(other)) {return FALSE;}
    if ((fInfo != fmt->fInfo) && (fInfo == NULL || fmt->fInfo == NULL)) {return FALSE;}
    if (fInfo && fmt->fInfo && (*fInfo != *fmt->fInfo )) {return FALSE;}
    {
        Mutex lock(&gFormatterMutex);
        if (fDateFormat != fmt->fDateFormat && (fDateFormat == NULL || fmt->fDateFormat == NULL)) {return FALSE;}
        if (fDateFormat && fmt->fDateFormat && (*fDateFormat != *fmt->fDateFormat)) {return FALSE;}
    }
    // note: fFromCalendar and fToCalendar hold no persistent state, and therefore do not participate in operator ==.
    //       fDateFormat has the master calendar for the DateIntervalFormat.
    if (fSkeleton != fmt->fSkeleton) {return FALSE;}
    if (fDatePattern != fmt->fDatePattern && (fDatePattern == NULL || fmt->fDatePattern == NULL)) {return FALSE;}
    if (fDatePattern && fmt->fDatePattern && (*fDatePattern != *fmt->fDatePattern)) {return FALSE;}
    if (fTimePattern != fmt->fTimePattern && (fTimePattern == NULL || fmt->fTimePattern == NULL)) {return FALSE;}
    if (fTimePattern && fmt->fTimePattern && (*fTimePattern != *fmt->fTimePattern)) {return FALSE;}
    if (fDateTimeFormat != fmt->fDateTimeFormat && (fDateTimeFormat == NULL || fmt->fDateTimeFormat == NULL)) {return FALSE;}
    if (fDateTimeFormat && fmt->fDateTimeFormat && (*fDateTimeFormat != *fmt->fDateTimeFormat)) {return FALSE;}
    if (fLocale != fmt->fLocale) {return FALSE;}

    for (int32_t i = 0; i< DateIntervalInfo::kIPI_MAX_INDEX; ++i ) {
        if (fIntervalPatterns[i].firstPart != fmt->fIntervalPatterns[i].firstPart) {return FALSE;}
        if (fIntervalPatterns[i].secondPart != fmt->fIntervalPatterns[i].secondPart ) {return FALSE;}
        if (fIntervalPatterns[i].laterDateFirst != fmt->fIntervalPatterns[i].laterDateFirst) {return FALSE;}
    }
    return TRUE;
}


// Source: dtitvfmt.cpp
// Lines 225-254
