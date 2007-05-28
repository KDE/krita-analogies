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

#ifndef _FEATURE_SEARCH_H_
#define _FEATURE_SEARCH_H_

#include <ANN/ANN.h>          // ANN declarations

// TODO: remove that
#define LCMS_HEADER <lcms.h>
// TODO: remove it !

#include <qrect.h>
#include <kis_types.h>

/**
 */
class FeatureSearch {
    public:
        FeatureSearch( KisPaintDeviceSP dev, QRect area, int r);
        ~FeatureSearch();
        inline int radius() { return m_radius; }
        inline int diameter() { return m_diameter; }
        int search(ANNpoint point);
    private:
        ANNpointArray m_pointArray;
        int m_countFeatures; ///< the number of features in the search space
        int m_radius; ///< the radius of the descriptor of each features
        int m_diameter; ///< the diameter
        int m_spaceDimension; ///< the size of the descriptor
        ANNkd_tree* m_tree; ///< the search tree
        QRect m_area;
};

#endif
