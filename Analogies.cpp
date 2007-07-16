/*
 * This file is part of the KDE project
 *
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "Analogies.h"

#include <stdlib.h>
#include <vector>

#include <kdebug.h>
#include <klocale.h>
#include <kgenericfactory.h>

#include <kis_composite_op.h>
#include <kis_doc.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_iterators_pixel.h>
#include <kis_random_accessor.h>
#include <kis_progress_display_interface.h>
#include <kis_meta_registry.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_selection.h>
#include <kis_transaction.h>

#include <kis_convolution_painter.h>

#include <kis_basic_math_toolbox2.h>

#include "FeatureSearch.h"
#include "Utils.h"

typedef KGenericFactory<KritaAnalogies> KritaAnalogiesFactory;
K_EXPORT_COMPONENT_FACTORY( kritaAnalogies, KritaAnalogiesFactory( "krita" ) )

KritaAnalogies::KritaAnalogies(QObject *parent, const char *name, const QStringList &)
: KParts::Plugin(parent, name)
{
    setInstance(KritaAnalogiesFactory::instance());

    kdDebug(41006) << "Fast Gaussian Blur filter plugin. Class: "
    << className()
    << ", Parent: "
    << parent -> className()
    << "\n";

    if(parent->inherits("KisFilterRegistry"))
    {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(new KisAnalogiesFilter());
    }
}

KritaAnalogies::~KritaAnalogies()
{
}

KisAnalogiesFilter::KisAnalogiesFilter() 
    : KisFilter(id(), "Analogies", i18n("&Analogies..."))
{
}

std::list<KisFilterConfiguration*> KisAnalogiesFilter::listOfExamplesConfiguration(KisPaintDeviceSP )
{
    std::list<KisFilterConfiguration*> list;
    list.insert(list.begin(), configuration());
    return list;
}

KisFilterConfiguration* KisAnalogiesFilter::configuration()
{
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(),1);
    config->setProperty("imgA", "/home/cyrille/imgA.png"); // TODO: for test purposes
    config->setProperty("imgAprime", "/home/cyrille/imgAprime.png");
    return config;
};

KisFilterConfigWidget * KisAnalogiesFilter::createConfigurationWidget(QWidget* /*parent*/, KisPaintDeviceSP /*dev*/)
{
    return 0;
}

KisFilterConfiguration* KisAnalogiesFilter::configuration(QWidget* /*nwidget*/)
{
    return configuration();
}

void KisAnalogiesFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, 
                                   KisFilterConfiguration* config, const QRect& rect ) 
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    if(config == 0) return;
    
    QString imgAfilename = config->getString("imgA", "");
    QString imgAprimeFilename = config->getString("imgAprime", "");
    
    if(imgAfilename == "" or imgAprimeFilename == "") return;
    
    kdDebug() << "Opening " << imgAfilename << endl;
    
    KisPaintDeviceSP imgADevice;
    KisDoc dImgA;
    dImgA.import(imgAfilename);
    KisImageSP importedImageImgA = dImgA.currentImage();

    if(importedImageImgA)
    {
        imgADevice = importedImageImgA->projection();
    }
    if(!imgADevice)
    {
        kdDebug() << "An error occured when openening image A." << endl;
        return;
    }
    
    kdDebug() << "Opening " << imgAprimeFilename << endl;
    
    KisPaintDeviceSP imgAPrimeDevice;
    KisDoc dImgAPrime;
    dImgAPrime.import(imgAprimeFilename);
    KisImageSP importedImageImgAPrime = dImgAPrime.currentImage();

    if(importedImageImgAPrime)
    {
        imgAPrimeDevice = importedImageImgAPrime->projection();
    }
    if(!imgAPrimeDevice)
    {
        kdDebug() << "An error occured when openening image A'." << endl;
        return;
    }
    QRect rectA(0,0,importedImageImgAPrime->width(),importedImageImgAPrime->height());
    if(rectA.width() != importedImageImgA->width() or rectA.height() != importedImageImgA->height())
    {
        kdDebug() << "Image A and Image A' must have the same size." << endl;
        return;
    }
    // Compute the number of levels of the pyramids
    int ANbLevels = 0;
    {
        int size = QMIN(rectA.width(), rectA.height());
        while( (size /= 2) > 10)
        {
            ANbLevels++;
        }
    }
    int BNbLevels = 0;
    {
        int size = QMIN(rect.width(), rect.height());
        while( (size /= 2) > 10)
        {
            BNbLevels++;
        }
    }
    // Convert A and B to LAB
    KisColorSpace* labCS = KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("LABA"),"");
    imgADevice->convertTo(labCS);
    KisPaintDeviceSP srcLAB = new KisPaintDevice(*src);
    srcLAB->convertTo(labCS);
    // Convert A' to B' 's colorspace
    imgAPrimeDevice->convertTo( labCS );
    
    int NbLevels = QMIN(ANbLevels, BNbLevels);
    NbLevels = 1;
    
    
    // Compute pyramids
    KisBasicMathToolbox2 tlb2;
    kdDebug() << "Computing the simple pyramid for image A with " << NbLevels << " levels" << endl;
    KisBasicMathToolbox2::Pyramid* gaussianPyramidImgA = tlb2.toSimplePyramid(imgADevice, NbLevels, rectA);
    kdDebug() << "Computing the simple pyramid for image A' with " << NbLevels << " levels" << endl;
    KisBasicMathToolbox2::Pyramid* gaussianPyramidImgAPrime = tlb2.toSimplePyramid(imgAPrimeDevice, NbLevels, rectA);
    kdDebug() << "Computing the simple pyramid for image B with " << BNbLevels << " levels" << endl;
    KisBasicMathToolbox2::Pyramid* gaussianPyramidImgB = tlb2.toSimplePyramid(srcLAB, NbLevels, rect);
    kdDebug() << gaussianPyramidImgA->levels.last().size << " " << gaussianPyramidImgAPrime->levels.last().size << " " << gaussianPyramidImgB->levels.last().size << endl;
    
    int radius = 2;
    int diameter = radius * 2 + 1;
    int dimension = Feature::size() * diameter * diameter;
    
    int totalCacheLineCount = (diameter + rect.width());
    
    Feature** pixels = new Feature*[diameter];
    for(int i = 0; i < diameter; i++)
    {
        pixels[i] = new Feature[ totalCacheLineCount ];
    }
    
//     KisColorSpace* cs = dst->colorSpace();
//     Q_UINT16 labPixel[4];
//     Q_UINT8* labPixelU8 = reinterpret_cast<Q_UINT8*>(labPixel);
    
    KisPaintDeviceSP devBPrime = 0;//new KisPaintDevice(cs,"B'");
    
    ANNpoint queryPoint = annAllocPt( dimension );
    
    // Go throught the levels of the pyramid
    for(int i = NbLevels; i >=0; i--)
    {
        kdDebug() << "Level " << i << " of the pyramid:" << endl;
        QRect rectA(QPoint(0,0),gaussianPyramidImgA->levels[0].size);
        
        kdDebug() << " - Initialization of the FeatureSearch" << endl;
        FeatureSearch aSearch(radius);
        aSearch.addPair( gaussianPyramidImgA->levels[0].device, QRect(QPoint(0,0), gaussianPyramidImgA->levels[0].size), gaussianPyramidImgAPrime->levels[0].device );
        aSearch.initSearch();
        
        
        kdDebug() << " - Search features for image B" << endl;
        devBPrime = new KisPaintDevice(* gaussianPyramidImgB->levels[i].device);
        
        // Read the first 'radius' line to initialize the cache
        QRect area = QRect(QPoint(0,0), gaussianPyramidImgB->levels[i].size);
        KisHLineIterator it = gaussianPyramidImgB->levels[i].device->createHLineIterator(area.x(), area.y(), area.width(), false);
//         KisRandomAccessor radAcc = /*gaussianPyramidImgAPrime->levels[i].device*/ imgAPrimeDevice->createRandomAccessor(0,0,true);
//         KisHLineIterator itAcc = imgAPrimeDevice->createHLineIterator(xAcc, yAcc, area.width(), true); //< if I don't do that, for some reason the imgA' isn't read correctly
        KisHLineIterator itDst = devBPrime ->createHLineIterator(area.x(), area.y(), area.width(), true);
        
        // Initialize cache constants
        int cacheLineCount = (area.width() + diameter); // countains the local count of the cache
        int cacheLineSize = cacheLineCount * sizeof(Feature); // countains the local size (in bytes) of the cache
        
        // Intialize the first row of the cache, as the cache needs to be full before it is possible to start creating descriptors for the key points
        for(int indexInPixels = radius; indexInPixels < diameter - 1; ++indexInPixels)
        {
            deviceToCache(it, pixels[indexInPixels], radius);
            it.nextRow();
        }
        
        // Copy the first line, as the first 'radius' columns are initialized with the same pixel value
        for(int k = 0; k < radius; ++k)
        {
            memcpy(pixels[k], pixels[radius], cacheLineSize);
        }
        
        // Main loop
        for(int y = 0; y < area.height(); y++)
        {
            // Fill the last line of the cache
            if( it.y() <= area.bottom())
            {
                deviceToCache(it, pixels[diameter - 1], radius);
            } else { // No more line in the device, so copy the last line
                memcpy(pixels[diameter - 1], pixels[diameter - 2], cacheLineSize);
            }
            it.nextRow();
            // Use the cache to search for the destination point
            for(int x = 0; x < area.width(); x++)
            {
                // Create the search point
                int subPos = 0;
//                 kdDebug() << " New search " << x << endl;
                for(int u = 0; u < diameter; u++)
                {
                    for(int v = 0; v < diameter; v++)
                    {
                        pixels[u][v + x].convertToArray(queryPoint + subPos );
//                         kdDebug() << subPos << " "<< pixels[i][j + x] << endl;
                     subPos++;
                    }
                }
                // Search
#if 0
                int index = aSearch.search(queryPoint);
                int xAcc = (index % rectA.width() );
                int yAcc = (index / rectA.width() );
//                 kdDebug() << "Index: " << index << " xAcc = " << xAcc  << " yAcc = "  << yAcc << endl;
//                 radAcc.moveTo( xAcc , yAcc );
//                 kdDebug() << (int)itAcc.oldRawData()[0] << " " << (int)itAcc.oldRawData()[1] << " " << (int)itAcc.oldRawData()[2] << " " << (int)itAcc.oldRawData()[3] << endl;
//                 kdDebug() << (int)radAcc.rawData()[0] << " " << (int)radAcc.rawData()[1] << " " << (int)radAcc.rawData()[2] << " " << (int)radAcc.rawData()[3] << endl;
//                 kdDebug() <<  reinterpret_cast<Q_UINT16*>(radAcc.rawData())[0] << " " << reinterpret_cast<Q_UINT16*>(itAcc.rawData())[0] << endl;
                KisHLineIterator itAcc = gaussianPyramidImgAPrime->levels[i].device->createHLineIterator(xAcc, yAcc, area.width(), true); //< the random accessor don't work correctly for cs != rgb8..
//                 labPixel[0] = reinterpret_cast<float*>(radAcc.rawData())[0];
                reinterpret_cast<float*>(itDst.rawData())[0] =  reinterpret_cast<float*>(itAcc.rawData())[0];
#endif
                reinterpret_cast<float*>(itDst.rawData())[0] = aSearch.search(queryPoint);
                ++itDst;
            }
            itDst.nextRow();
            swapCache(pixels, diameter);
        }
    }
    // Transform to 16bit integer LAB
    KisPaintDeviceSP dstLAB = new KisPaintDevice(*dst);
    dstLAB->convertTo(labCS);
    tlb2.fromFloatDevice( devBPrime, dstLAB, rect);
    
    // Transform to final color space and bitblt it on the destination
    dstLAB->convertTo(dst->colorSpace());
    KisPainter p(dst);
    p.bitBlt( rect.x(), rect.y(), COMPOSITE_OVER, dstLAB, rect.x(), rect.y(), rect.width(), rect.height());
    
    // delete the luminosity cache
    for(int i = 0; i < diameter; i++)
    {
        delete[] pixels[i];
    }
    delete[] pixels;
    
    delete gaussianPyramidImgA;
    delete gaussianPyramidImgAPrime;
    delete gaussianPyramidImgB;
    setProgressDone(); // Must be called even if you don't really support progression
}
