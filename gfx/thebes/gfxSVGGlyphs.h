/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GFX_SVG_GLYPHS_WRAPPER_H
#define GFX_SVG_GLYPHS_WRAPPER_H

#include "gfxFontUtils.h"
#include "nsString.h"
#include "nsIDocument.h"
#include "nsAutoPtr.h"
#include "nsIContentViewer.h"
#include "nsIPresShell.h"
#include "nsClassHashtable.h"
#include "nsBaseHashtable.h"
#include "nsHashKeys.h"
#include "gfxPattern.h"
#include "gfxFont.h"
#include "mozilla/gfx/UserData.h"
#include "nsRefreshDriver.h"
 
class gfxSVGGlyphs;


/**
 * Wraps an SVG document contained in the SVG table of an OpenType font.
 * There may be multiple SVG documents in an SVG table which we lazily parse
 *   so we have an instance of this class for every document in the SVG table
 *   which contains a glyph ID which has been used
 * Finds and looks up elements contained in the SVG document which have glyph
 *   mappings to be drawn by gfxSVGGlyphs
 */
class gfxSVGGlyphsDocument MOZ_FINAL : public nsAPostRefreshObserver
{
    typedef mozilla::dom::Element Element;

public:
    gfxSVGGlyphsDocument(const uint8_t *aBuffer, uint32_t aBufLen,
                         gfxSVGGlyphs *aSVGGlyphs);

    Element *GetGlyphElement(uint32_t aGlyphId);

    ~gfxSVGGlyphsDocument();

    virtual void DidRefresh() MOZ_OVERRIDE;

private:
    nsresult ParseDocument(const uint8_t *aBuffer, uint32_t aBufLen);

    nsresult SetupPresentation();

    void FindGlyphElements(Element *aElement);

    void InsertGlyphId(Element *aGlyphElement);

    // Weak so as not to create a cycle. mOwner owns us so this can't dangle.
    gfxSVGGlyphs* mOwner;
    nsCOMPtr<nsIDocument> mDocument;
    nsCOMPtr<nsIContentViewer> mViewer;
    nsCOMPtr<nsIPresShell> mPresShell;

    nsBaseHashtable<nsUint32HashKey, Element*, Element*> mGlyphIdMap;

    nsAutoCString mSVGGlyphsDocumentURI;
};

/**
 * Used by |gfxFontEntry| to represent the SVG table of an OpenType font.
 * Handles lazy parsing of the SVG documents in the table, looking up SVG glyphs
 *   and rendering SVG glyphs.
 * Each |gfxFontEntry| owns at most one |gfxSVGGlyphs| instance.
 */
class gfxSVGGlyphs
{
private:
    typedef mozilla::dom::Element Element;

public:
    /**
     * @param aSVGTable The SVG table from the OpenType font
     *
     * The gfxSVGGlyphs object takes over ownership of the blob references
     * that are passed in, and will hb_blob_destroy() them when finished;
     * the caller should -not- destroy these references.
     */
    gfxSVGGlyphs(hb_blob_t *aSVGTable, gfxFontEntry *aFontEntry);

    /**
     * Releases our references to the SVG table and cleans up everything else.
     */
    ~gfxSVGGlyphs();

    /**
     * This is called when the refresh driver has ticked.
     */
    void DidRefresh();

    /**
     * Find the |gfxSVGGlyphsDocument| containing an SVG glyph for |aGlyphId|.
     * If |aGlyphId| does not map to an SVG document, return null.
     * If a |gfxSVGGlyphsDocument| has not been created for the document, create one.
     */
    gfxSVGGlyphsDocument *FindOrCreateGlyphsDocument(uint32_t aGlyphId);

    /**
     * Return true iff there is an SVG glyph for |aGlyphId|
     */
    bool HasSVGGlyph(uint32_t aGlyphId);

    /**
     * Render the SVG glyph for |aGlyphId|
     * @param aDrawMode Whether to fill or stroke or both; see DrawMode
     * @param aContextPaint Information on text context paints.
     *   See |gfxTextContextPaint|.
     */
    bool RenderGlyph(gfxContext *aContext, uint32_t aGlyphId, DrawMode aDrawMode,
                     gfxTextContextPaint *aContextPaint);

    /**
     * Get the extents for the SVG glyph associated with |aGlyphId|
     * @param aSVGToAppSpace The matrix mapping the SVG glyph space to the
     *   target context space
     */
    bool GetGlyphExtents(uint32_t aGlyphId, const gfxMatrix& aSVGToAppSpace,
                         gfxRect *aResult);

private:
    Element *GetGlyphElement(uint32_t aGlyphId);

    nsClassHashtable<nsUint32HashKey, gfxSVGGlyphsDocument> mGlyphDocs;
    nsBaseHashtable<nsUint32HashKey, Element*, Element*> mGlyphIdMap;

    hb_blob_t *mSVGData;
    gfxFontEntry *mFontEntry;

    const struct Header {
        mozilla::AutoSwap_PRUint16 mVersion;
        mozilla::AutoSwap_PRUint32 mDocIndexOffset;
        mozilla::AutoSwap_PRUint32 mColorPalettesOffset;
    } *mHeader;

    struct IndexEntry {
        mozilla::AutoSwap_PRUint16 mStartGlyph;
        mozilla::AutoSwap_PRUint16 mEndGlyph;
        mozilla::AutoSwap_PRUint32 mDocOffset;
        mozilla::AutoSwap_PRUint32 mDocLength;
    };

    const struct DocIndex {
      mozilla::AutoSwap_PRUint16 mNumEntries;
      IndexEntry mEntries[1]; /* actual length = mNumEntries */
    } *mDocIndex;

    static int CompareIndexEntries(const void *_a, const void *_b);
};

/**
 * Used for trickling down paint information through to SVG glyphs.
 */
class gfxTextContextPaint
{
protected:
    gfxTextContextPaint() { }

public:
    static mozilla::gfx::UserDataKey sUserDataKey;

    /*
     * Get text context pattern with the specified opacity value.
     * This lets us inherit paints and paint opacities (i.e. fill/stroke and
     * fill-opacity/stroke-opacity) separately.
     */
    virtual already_AddRefed<gfxPattern> GetFillPattern(float aOpacity,
                                                        const gfxMatrix& aCTM) = 0;
    virtual already_AddRefed<gfxPattern> GetStrokePattern(float aOpacity,
                                                          const gfxMatrix& aCTM) = 0;

    virtual float GetFillOpacity() { return 1.0f; }
    virtual float GetStrokeOpacity() { return 1.0f; }

    void InitStrokeGeometry(gfxContext *aContext,
                            float devUnitsPerSVGUnit);

    FallibleTArray<gfxFloat>& GetStrokeDashArray() {
        return mDashes;
    }

    gfxFloat GetStrokeDashOffset() {
        return mDashOffset;
    }

    gfxFloat GetStrokeWidth() {
        return mStrokeWidth;
    }

    already_AddRefed<gfxPattern> GetFillPattern(const gfxMatrix& aCTM) {
        return GetFillPattern(GetFillOpacity(), aCTM);
    }

    already_AddRefed<gfxPattern> GetStrokePattern(const gfxMatrix& aCTM) {
        return GetStrokePattern(GetStrokeOpacity(), aCTM);
    }

    virtual ~gfxTextContextPaint() { }

private:
    FallibleTArray<gfxFloat> mDashes;
    gfxFloat mDashOffset;
    gfxFloat mStrokeWidth;
};

/**
 * For passing in patterns where the text context has no separate pattern
 * opacity value.
 */
class SimpleTextContextPaint : public gfxTextContextPaint
{
private:
    static const gfxRGBA sZero;

public:
    static gfxMatrix SetupDeviceToPatternMatrix(gfxPattern *aPattern,
                                                const gfxMatrix& aCTM)
    {
        if (!aPattern) {
            return gfxMatrix();
        }
        gfxMatrix deviceToUser = aCTM;
        deviceToUser.Invert();
        return deviceToUser * aPattern->GetMatrix();
    }

    SimpleTextContextPaint(gfxPattern *aFillPattern, gfxPattern *aStrokePattern,
                          const gfxMatrix& aCTM) :
        mFillPattern(aFillPattern ? aFillPattern : new gfxPattern(sZero)),
        mStrokePattern(aStrokePattern ? aStrokePattern : new gfxPattern(sZero))
    {
        mFillMatrix = SetupDeviceToPatternMatrix(aFillPattern, aCTM);
        mStrokeMatrix = SetupDeviceToPatternMatrix(aStrokePattern, aCTM);
    }

    already_AddRefed<gfxPattern> GetFillPattern(float aOpacity,
                                                const gfxMatrix& aCTM) {
        if (mFillPattern) {
            mFillPattern->SetMatrix(aCTM * mFillMatrix);
        }
        nsRefPtr<gfxPattern> fillPattern = mFillPattern;
        return fillPattern.forget();
    }

    already_AddRefed<gfxPattern> GetStrokePattern(float aOpacity,
                                                  const gfxMatrix& aCTM) {
        if (mStrokePattern) {
            mStrokePattern->SetMatrix(aCTM * mStrokeMatrix);
        }
        nsRefPtr<gfxPattern> strokePattern = mStrokePattern;
        return strokePattern.forget();
    }

    float GetFillOpacity() {
        return mFillPattern ? 1.0f : 0.0f;
    }

    float GetStrokeOpacity() {
        return mStrokePattern ? 1.0f : 0.0f;
    }

private:
    nsRefPtr<gfxPattern> mFillPattern;
    nsRefPtr<gfxPattern> mStrokePattern;

    // Device space to pattern space transforms
    gfxMatrix mFillMatrix;
    gfxMatrix mStrokeMatrix;
};

#endif
