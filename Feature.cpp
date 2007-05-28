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

#include "Feature.h"

Feature::Feature() : m_L(0.0), m_a(0.0), m_b(0.0)
{
}

Feature::Feature(float L, float a, float b) : m_L(L), m_a(a), m_b(b)
{
    
}

int Feature::size()
{
    return 1;
//     return 3;
}

void Feature::convertToArray(ANNcoord* arr)
{
    arr[0] = m_L;
/*    arr[1] = m_a;
    arr[2] = m_b;*/
}
