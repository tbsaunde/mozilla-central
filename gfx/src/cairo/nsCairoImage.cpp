/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is mozilla.org.
 * Portions created by the Initial Developer are
 * Copyright (C) 2004 the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *    Stuart Parmenter <pavlov@pavlov.net>
 *    Vladimir Vukicevic <vladimir@pobox.com>
 *    Joe Hewitt <hewitt@netscape.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 */

#include "nsMemory.h"
#include "nsCairoImage.h"
#include "nsCairoRenderingContext.h"
#include "nsCairoDrawingSurface.h"

#include "nsIServiceManager.h"

nsCairoImage::nsCairoImage()
 : mWidth(0),
   mHeight(0),
   mImageSurface(nsnull),
   mImageSurfaceData(nsnull)
{
}

nsCairoImage::~nsCairoImage()
{
    if (mImageSurface)
        cairo_surface_destroy(mImageSurface);
    // XXXX - totally not safe. mImageSurface should own data,
    // but it doesn't.
    if (mImageSurfaceData)
        nsMemory::Free(mImageSurfaceData);
    if (mImageSurfaceAlpha)
        nsMemory::Free(mImageSurfaceAlpha);
}

NS_IMPL_ISUPPORTS1(nsCairoImage, nsIImage);

//////////////////////////////////////////////
//// nsIImage

nsresult
nsCairoImage::Init(PRInt32 aWidth, PRInt32 aHeight, PRInt32 aDepth, nsMaskRequirements aMaskRequirements)
{
    mWidth = aWidth;
    mHeight = aHeight;

    switch(aMaskRequirements)
    {
    case nsMaskRequirements_kNeeds1Bit:
        break;
    case nsMaskRequirements_kNeeds8Bit:
        break;
    default:
        break;
    }

    if (aDepth == 24 || aDepth == 32) {
        mImageSurfaceData = (char *) nsMemory::Alloc(aWidth * aHeight * 4);
        mImageSurfaceAlpha = (char *) nsMemory::Alloc(aWidth * aHeight);
        mImageSurface = cairo_image_surface_create_for_data
            (mImageSurfaceData,
             CAIRO_FORMAT_ARGB32,
             mWidth,
             mHeight,
             aWidth * 4);
    }

    return NS_OK;
}

PRInt32
nsCairoImage::GetBytesPix()
{
    return 4;
}

PRBool
nsCairoImage::GetIsRowOrderTopToBottom()
{
    return PR_FALSE;
}

PRInt32
nsCairoImage::GetWidth()
{
    return mWidth;
}

PRInt32
nsCairoImage::GetHeight()
{
    return mHeight;
}

PRUint8 *
nsCairoImage::GetBits()
{
    return (PRUint8 *) mImageSurfaceData;
}

PRInt32
nsCairoImage::GetLineStride()
{
    return mWidth * 4;
}

PRBool
nsCairoImage::GetHasAlphaMask()
{
    /* imglib is such a piece of crap! */
    return PR_TRUE;
}

PRUint8 *
nsCairoImage::GetAlphaBits()
{
    /* eugh */
    /* imglib is such a piece of crap! */
    return (PRUint8 *) mImageSurfaceAlpha;
}

PRInt32
nsCairoImage::GetAlphaLineStride()
{
    /* imglib is such a piece of crap! */
    return mWidth;
}

void
nsCairoImage::ImageUpdated(nsIDeviceContext *aContext, PRUint8 aFlags, nsRect *aUpdateRect)
{
}

nsresult
nsCairoImage::Optimize(nsIDeviceContext* aContext)
{
    return NS_OK;
}

nsColorMap *
nsCairoImage::GetColorMap()
{
    return NULL;
}

/* WHY ARE THESE HERE?!?!?!?!?!? */
/* INSTEAD OF RENDERINGCONTEXT, WHICH ONLY TAKES IMGICONTAINER?!?!?!? */
NS_IMETHODIMP
nsCairoImage::Draw(nsIRenderingContext &aContext, nsIDrawingSurface *aSurface,
                   PRInt32 aX, PRInt32 aY, PRInt32 aWidth, PRInt32 aHeight)
{
    return Draw(aContext, aSurface, 0, 0, mWidth, mHeight, aX, aY, aWidth, aHeight);
}

NS_IMETHODIMP
nsCairoImage::Draw(nsIRenderingContext &aContext, nsIDrawingSurface *aSurface,
                   PRInt32 aSX, PRInt32 aSY, PRInt32 aSWidth, PRInt32 aSHeight,
                   PRInt32 aDX, PRInt32 aDY, PRInt32 aDWidth, PRInt32 aDHeight)
{
#if 1
    fprintf (stderr, "****** nsCairoImage::Draw: (%d,%d,%d,%d) -> (%d,%d,%d,%d)\n",
             aSX, aSY, aSWidth, aSHeight,
             aDX, aDY, aDWidth, aDHeight);
#endif

    nsCairoDrawingSurface *dstSurf = NS_STATIC_CAST(nsCairoDrawingSurface*, aSurface);
    nsCairoRenderingContext *cairoContext = NS_STATIC_CAST(nsCairoRenderingContext*, &aContext);

    cairo_t *dstCairo = cairoContext->GetCairo();

    cairo_save(dstCairo);

    cairo_set_rgb_color (dstCairo, 1.0, 0.0, 0.0);
    cairo_rectangle (dstCairo, double(aDX), double(aDY), double(aDWidth), double(aDHeight));
    cairo_fill (dstCairo);

#if 0
    cairo_pattern_t *imgpat = cairo_pattern_create_for_surface (mImageSurface);
    cairo_matrix_t *mat = cairo_matrix_create();
    cairo_matrix_translate (mat, double(aSX), double(aSY));
    cairo_matrix_scale (mat, double(aDWidth)/double(aSWidth), double(aDHeight)/double(aSHeight));
    cairo_pattern_set_matrix (imgpat, mat);

    cairo_set_pattern (dstCairo, imgpat);

    cairo_new_path (dstCairo);
    cairo_rectangle (dstCairo, double(aDX), double(aDY), double(aDWidth), double(aDHeight));
    cairo_fill (dstCairo);

    cairo_set_pattern (dstCairo, nsnull);
    cairo_pattern_destroy (imgpat);
    cairo_matrix_destroy(mat);
#endif

    cairo_restore(dstCairo);

    return NS_OK;
}

NS_IMETHODIMP
nsCairoImage::DrawTile(nsIRenderingContext &aContext,
                       nsIDrawingSurface *aSurface,
                       PRInt32 aSXOffset, PRInt32 aSYOffset,
                       PRInt32 aPadX, PRInt32 aPadY,
                       const nsRect &aTileRect)
{
    NS_WARNING("not implemented");
    return NS_OK;
}

NS_IMETHODIMP
nsCairoImage::DrawToImage(nsIImage* aDstImage, PRInt32 aDX, PRInt32 aDY, PRInt32 aDWidth, PRInt32 aDHeight)
{
    nsCairoImage *dstCairoImage = NS_STATIC_CAST(nsCairoImage*, aDstImage);

    cairo_t *cairo = cairo_create ();

    cairo_set_target_surface (cairo, dstCairoImage->mImageSurface);

    cairo_pattern_t *pat = cairo_pattern_create_for_surface (mImageSurface);
    cairo_matrix_t *mat = cairo_matrix_create ();
    cairo_matrix_scale (mat, double(aDWidth)/double(mWidth), double(aDHeight)/double(mHeight));
    cairo_pattern_set_matrix (pat, mat);

    cairo_set_pattern (cairo, pat);

    cairo_new_path (cairo);
    cairo_rectangle (cairo, double(aDX), double(aDY), double(aDWidth), double(aDHeight));
    cairo_fill (cairo);

    cairo_destroy (cairo);
    cairo_pattern_destroy (pat);
    cairo_matrix_destroy (mat);

    return NS_OK;
}

PRInt8
nsCairoImage::GetAlphaDepth()
{
    return 0;
}

void *
nsCairoImage::GetBitInfo()
{
    return NULL;
}

NS_IMETHODIMP
nsCairoImage::LockImagePixels(PRBool aMaskPixels)
{
    return NS_OK;
}

NS_IMETHODIMP
nsCairoImage::UnlockImagePixels(PRBool aMaskPixels)
{
    return NS_OK;
}
