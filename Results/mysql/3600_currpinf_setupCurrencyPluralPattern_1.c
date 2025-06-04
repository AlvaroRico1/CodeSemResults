CurrencyPluralInfo::setupCurrencyPluralPattern(const Locale& loc, UErrorCode& status) {
    if (U_FAILURE(status)) {
        return;
    }

    deleteHash(fPluralCountToCurrencyUnitPattern);
    fPluralCountToCurrencyUnitPattern = initHash(status);
    if (U_FAILURE(status)) {
        return;
    }

    LocalPointer<NumberingSystem> ns(NumberingSystem::createInstance(loc, status), status);
    if (U_FAILURE(status)) {
        return;
    }
    UErrorCode ec = U_ZERO_ERROR;
    LocalUResourceBundlePointer rb(ures_open(nullptr, loc.getName(), &ec));
    LocalUResourceBundlePointer numElements(ures_getByKeyWithFallback(rb.getAlias(), gNumberElementsTag, nullptr, &ec));
    ures_getByKeyWithFallback(numElements.getAlias(), ns->getName(), rb.getAlias(), &ec);
    ures_getByKeyWithFallback(rb.getAlias(), gPatternsTag, rb.getAlias(), &ec);
    int32_t ptnLen;
    const UChar* numberStylePattern = ures_getStringByKeyWithFallback(rb.getAlias(), gDecimalFormatTag, &ptnLen, &ec);
    // Fall back to "latn" if num sys specific pattern isn't there.
    if ( ec == U_MISSING_RESOURCE_ERROR && (uprv_strcmp(ns->getName(), gLatnTag) != 0)) {
        ec = U_ZERO_ERROR;
        ures_getByKeyWithFallback(numElements.getAlias(), gLatnTag, rb.getAlias(), &ec);
        ures_getByKeyWithFallback(rb.getAlias(), gPatternsTag, rb.getAlias(), &ec);
        numberStylePattern = ures_getStringByKeyWithFallback(rb.getAlias(), gDecimalFormatTag, &ptnLen, &ec);
    }
    int32_t numberStylePatternLen = ptnLen;
    const UChar* negNumberStylePattern = nullptr;
    int32_t negNumberStylePatternLen = 0;
    // TODO: Java
    // parse to check whether there is ";" separator in the numberStylePattern
    UBool hasSeparator = false;
    if (U_SUCCESS(ec)) {
        for (int32_t styleCharIndex = 0; styleCharIndex < ptnLen; ++styleCharIndex) {
            if (numberStylePattern[styleCharIndex] == gNumberPatternSeparator) {
                hasSeparator = true;
                // split the number style pattern into positive and negative
                negNumberStylePattern = numberStylePattern + styleCharIndex + 1;
                negNumberStylePatternLen = ptnLen - styleCharIndex - 1;
                numberStylePatternLen = styleCharIndex;
            }
        }
    }

    if (U_FAILURE(ec)) {
        // If OOM occurred during the above code, then we want to report that back to the caller.
        if (ec == U_MEMORY_ALLOCATION_ERROR) {
            status = ec;
        }
        return;
    }

    LocalUResourceBundlePointer currRb(ures_open(U_ICUDATA_CURR, loc.getName(), &ec));
    LocalUResourceBundlePointer currencyRes(ures_getByKeyWithFallback(currRb.getAlias(), gCurrUnitPtnTag, nullptr, &ec));
    
#ifdef CURRENCY_PLURAL_INFO_DEBUG
    std::cout << "in set up\n";
#endif
    LocalPointer<StringEnumeration> keywords(fPluralRules->getKeywords(ec), ec);
    if (U_SUCCESS(ec)) {
        const char* pluralCount;
        while (((pluralCount = keywords->next(nullptr, ec)) != nullptr) && U_SUCCESS(ec)) {
            int32_t ptnLength;
            UErrorCode err = U_ZERO_ERROR;
            const UChar* patternChars = ures_getStringByKeyWithFallback(currencyRes.getAlias(), pluralCount, &ptnLength, &err);
            if (err == U_MEMORY_ALLOCATION_ERROR || patternChars == nullptr) {
                ec = err;
                break;
            }
            if (U_SUCCESS(err) && ptnLength > 0) {
                UnicodeString* pattern = new UnicodeString(patternChars, ptnLength);
                if (pattern == nullptr) {
                    ec = U_MEMORY_ALLOCATION_ERROR;
                    break;
                }
#ifdef CURRENCY_PLURAL_INFO_DEBUG
                char result_1[1000];
                pattern->extract(0, pattern->length(), result_1, "UTF-8");
                std::cout << "pluralCount: " << pluralCount << "; pattern: " << result_1 << "\n";
#endif
                pattern->findAndReplace(UnicodeString(TRUE, gPart0, 3), 
                    UnicodeString(numberStylePattern, numberStylePatternLen));
                pattern->findAndReplace(UnicodeString(TRUE, gPart1, 3), UnicodeString(TRUE, gTripleCurrencySign, 3));

                if (hasSeparator) {
                    UnicodeString negPattern(patternChars, ptnLength);
                    negPattern.findAndReplace(UnicodeString(TRUE, gPart0, 3), 
                        UnicodeString(negNumberStylePattern, negNumberStylePatternLen));
                    negPattern.findAndReplace(UnicodeString(TRUE, gPart1, 3), UnicodeString(TRUE, gTripleCurrencySign, 3));
                    pattern->append(gNumberPatternSeparator);
                    pattern->append(negPattern);
                }
#ifdef CURRENCY_PLURAL_INFO_DEBUG
                pattern->extract(0, pattern->length(), result_1, "UTF-8");
                std::cout << "pluralCount: " << pluralCount << "; pattern: " << result_1 << "\n";
#endif
                // the 'pattern' object allocated above will be owned by the fPluralCountToCurrencyUnitPattern after the call to
                // put(), even if the method returns failure.
                fPluralCountToCurrencyUnitPattern->put(UnicodeString(pluralCount, -1, US_INV), pattern, status);
            }
        }
    }
    // If OOM occurred during the above code, then we want to report that back to the caller.
    if (ec == U_MEMORY_ALLOCATION_ERROR) {
        status = ec;
    }
}


// Source: currpinf.cpp
// Lines 271-380
