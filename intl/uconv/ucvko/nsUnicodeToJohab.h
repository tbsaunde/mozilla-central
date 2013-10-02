/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef nsUnicodeToJohab_h___
#define nsUnicodeToJohab_h___

#include "nsID.h"

class nsISupports;

/**
 * A character set converter from Unicode to Johab.
 *
 */
nsresult
nsUnicodeToJohabConstructor(nsISupports *aOuter, REFNSIID aIID,
                            void **aResult);

#endif /* nsUnicodeToJohab_h___ */
