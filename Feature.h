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

#ifndef _FEATURE_H_
#define _FEATURE_H_

#include <ANN/ANN.h>          // ANN declarations

class Feature {
    public:
        Feature();
        Feature(float L, float a, float b);
        static int size();
        void convertToArray(ANNcoord* arr);
        inline void setL(float L) { m_L = L; };
        inline void setA(float a) { m_a = a; };
        inline void setB(float b) { m_b = b; };
    private:
        float m_L, m_a, m_b;
};

#endif
