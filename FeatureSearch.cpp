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

#include "FeatureSearch.h"

#include <kis_colorspace.h>
#include <kis_iterators_pixel.h>
#include <kis_paint_device.h>

#include "Utils.h"

FeatureSearch::FeatureSearch( int r) : m_countFeatures(0), m_radius(r), m_diameter(2*r+1), m_tree(0)
{
    // Compute the size of the descriptors
    m_spaceDimension = Feature::size() * diameter() * diameter();
}

void FeatureSearch::addPair( KisPaintDeviceSP devA, QRect area, KisPaintDeviceSP devAPrime)
{
    SearchPairInfo pi;
    pi.devA = devA;
    pi.area = area;
    pi.devAPrime = devAPrime;
    m_infos.push_back(pi);
}

void FeatureSearch::initSearch()
{
    m_countFeatures = 0;
    for(std::vector<SearchPairInfo>::iterator itSPI = m_infos.begin(); itSPI != m_infos.end(); itSPI++)
    {
        m_countFeatures += itSPI->area.width() * itSPI->area.height();
    }
    // Allocate the array of points
    m_pointArray = annAllocPts( m_countFeatures, m_spaceDimension );
    
    for(std::vector<SearchPairInfo>::iterator itSPI = m_infos.begin(); itSPI != m_infos.end(); itSPI++)
    {
        // Initialize the descriptors
        int cacheLineCount = (itSPI->area.width() + diameter());
        size_t cacheLineSize = cacheLineCount * sizeof(Feature);
        // pixels will hold a cache of the luminosity to feed the descriptors
        Feature** pixels = new Feature*[diameter()];
        for(int i = 0; i < diameter(); i++)
        {
            pixels[i] = new Feature[ cacheLineCount ];
        }
    //     KisColorSpace* cs = dev->colorSpace();
        // Read the first 'radius' line to initialize the cache
        KisHLineIterator itDevA = itSPI->devA->createHLineIterator(itSPI->area.x(), itSPI->area.y(), itSPI->area.width(), false);
        KisHLineIterator itDevAPrime = itSPI->devAPrime->createHLineIterator(itSPI->area.x(), itSPI->area.y(), itSPI->area.width(), false);
        // Intialize the first row of the cache, as the cache needs to be full before it is possible to start creating descriptors for the key points
        for(int indexInPixels = radius(); indexInPixels < diameter() - 1; ++indexInPixels)
        {
            deviceToCache(itDevA, pixels[indexInPixels], radius());
            itDevA.nextRow();
        }
        // Copy the first line, as the first 'radius' columns are initialized with the same pixel value
        for(int i = 0; i < radius(); ++i)
        {
            memcpy(pixels[i], pixels[radius()], cacheLineSize);
        }
        // Main loop
        int posInAP = 0;
        for(int y = 0; y < itSPI->area.height(); y++)
        {
            // Fill the last line of the cache
            if( itDevA.y() <= itSPI->area.bottom())
            {
                deviceToCache(itDevA, pixels[diameter() - 1], radius());
            } else { // No more line in the device, so copy the last line
                memcpy(pixels[diameter() - 1], pixels[diameter() - 2], cacheLineSize);
            }
            itDevA.nextRow();
            // Use the cache to fill the array of points
            for(int x = 0; x < itSPI->area.width(); x++)
            {
                int subPos = 0;
    //             kdDebug() << " Feature : " << posInAP << endl;
                for(int i = 0; i < diameter(); i++)
                {
                    for(int j = 0; j < diameter(); j++)
                    {
                        pixels[i][j + x].convertToArray((m_pointArray[ posInAP ]) + subPos );
    //                     kdDebug() << subPos << " "<< pixels[i][j + x] << endl;
                        subPos++;
                    }
                }
                m_values.push_back(*reinterpret_cast<float*>(itDevAPrime.rawData()));
                ++itDevAPrime;
                ++posInAP;
            }
            itDevAPrime.nextRow();
            swapCache(pixels, diameter());
        }
        // delete the luminosity cache
        for(int i = 0; i < diameter(); i++)
        {
            delete[] pixels[i];
        }
        delete[] pixels;
    }

    // Initialize the search tree
    m_tree = new ANNkd_tree( m_pointArray, m_countFeatures, m_spaceDimension);
}

double FeatureSearch::search(ANNpoint point)
{
    int countSamples = 5;
    ANNidx resultIndex[5];
    ANNdist resultDist[5];
    float epsilon = 1.0;
    m_tree->annkSearch( point, countSamples, resultIndex, resultDist, 0.0/*epsilon*/);
    
//     for(int i = 0; i < countSamples; i++)
//     {
//         kdDebug() << i << "th search result: " << resultIndex[0] << " Distance = " << resultDist[0] << endl;
/*        double dist = 0.0;
        for(int k = 0; k < m_spaceDimension; k++)
        {
            double v =(point[k] - m_pointArray[ resultIndex[0] ][k]);
            dist += v*v;
        }*/
//         kdDebug() << dist << endl; 
//     }
    return m_values[resultIndex[0]];
}

FeatureSearch::~FeatureSearch()
{
    annDeallocPts( m_pointArray );
    delete m_tree;
}
