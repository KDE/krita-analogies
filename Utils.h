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

#ifndef _UTILS_H_
#define _UTILS_H_

inline void deviceToCache(KisHLineIterator& it, float* line, int radius)
{
    // Convert the first pixel of the row to initiliaze it
    int indexInLine = 0;
    {
        const float* pixel = reinterpret_cast<const float*>(it.oldRawData());
        for(indexInLine = 0; indexInLine <= 3*radius; indexInLine += 3)
        {
            line[indexInLine] = pixel[0];
            line[indexInLine+1] = pixel[1];
            line[indexInLine+2] = pixel[2];
        }
    }
    ++it;
    while(not it.isDone())
    {
//         kdDebug() << *it.oldRawData() << " => in L : " << labPixel[0] << endl;
        const float* pixel = reinterpret_cast<const float*>(it.oldRawData());
        line[indexInLine] = pixel[0];
        line[indexInLine+1] = pixel[1];
        line[indexInLine+2] = pixel[2];
        ++it;
        indexInLine+=3;
    }
    for(int i = 0; i < 3 * radius; ++i)
    {
        line[indexInLine + i] =line[indexInLine-3];
        line[indexInLine + i + 1] =line[indexInLine-2];
        line[indexInLine + i + 2] =line[indexInLine-1];
    }
}

inline void swapCache(float** pixels, int dimension)
{
    float* firstLine= pixels[0];
    for(int i =1; i < dimension; i++)
    {
        pixels[i - 1] = pixels[i];
    }
    pixels[dimension-1] = firstLine;
}

#endif
