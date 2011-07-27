// tiledCanvas.cpp
// this file is part of Context Free
// ---------------------
// Copyright (C) 2006 John Horigan - john@glyphic.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// John Horigan can be contacted at john@glyphic.com or at
// John Horigan, 1209 Villa St., Mountain View, CA 94041-1123, USA
//
//

#include "tiledCanvas.h"
#include <math.h>
#include "primShape.h"
#include "bounds.h"
#include <cstdlib>
#include <stdlib.h>

void tiledCanvas::start(bool clear, const agg::rgba& bk, int w, int h)
{
    mWidth = w;
    mHeight = h;
	mTile->start(clear, bk, w, h);
}

void tiledCanvas::end() 
{
	mTile->end();
}

void tiledCanvas::circle(RGBA8 c, agg::trans_affine tr)
{
    for (unsigned int i = 0; i < mTileList.size(); ++i) {
        agg::trans_affine t(tr);
        t.tx += mTileList[i].x;
        t.ty += mTileList[i].y;
        mTile->circle(c, t);
    }
}

void tiledCanvas::square(RGBA8 c, agg::trans_affine tr)
{
    for (unsigned int i = 0; i < mTileList.size(); ++i) {
        agg::trans_affine t(tr);
        t.tx += mTileList[i].x;
        t.ty += mTileList[i].y;
        mTile->square(c, t);
    }
}

void tiledCanvas::triangle(RGBA8 c, agg::trans_affine tr)
{
    for (unsigned int i = 0; i < mTileList.size(); ++i) {
        agg::trans_affine t(tr);
        t.tx += mTileList[i].x;
        t.ty += mTileList[i].y;
        mTile->triangle(c, t);
    }
}

void tiledCanvas::fill(RGBA8 c)
{
    mTile->fill(c);
}

void tiledCanvas::path(RGBA8 c, agg::trans_affine tr, const AST::CommandInfo& attr)
{
    for (unsigned int i = 0; i < mTileList.size(); ++i) {
        agg::trans_affine t(tr);
        t.tx += mTileList[i].x;
        t.ty += mTileList[i].y;
        mTile->path(c, t, attr);
    }
}

static const double tileBuffer = 1.05;

void
tiledCanvas::tileTransform(const Bounds& b)
// Adjust the translation part of the transform so that it falls within the 
// tile parallelogram at the origin. 
//
// Returns whether the shape is close to the edge of the canvas 
// (true=not close, false=close/overlapping).
{
    double centx = (b.mMin_X + b.mMax_X) * 0.5;
    double centy = (b.mMin_Y + b.mMax_Y) * 0.5;
    mInvert.transform(&centx, &centy);          // transform to unit square tesselation
    centx = floor(centx);
    centy = floor(centy);
	
    mTileList.clear();
    mTileList.push_back(agg::point_d(-centx, -centy));
    mOffset.transform(&(mTileList.back().x), &(mTileList.back().y));
    agg::rect_d canvas(0, 0, (double)(mWidth - 1), (double)(mHeight - 1));
    
    if (mFrieze) {
        centx += centy;
        for (int offset = 1; ; ++offset) {
            bool hit = false;
            for (int side = -1; side <= 1; side += 2) {
                double dx = offset * side - centx;
                double dy = dx;
                mOffset.transform(&dx, &dy);
                
                // If the tile might touch the canvas then record it
                agg::rect_d shape(b.mMin_X + dx, b.mMin_Y + dy, b.mMax_X + dx, b.mMax_Y + dy);
                if (shape.overlaps(canvas)) {
                    hit = true;
                    mTileList.push_back(agg::point_d(dx, dy));
                }
            }
            if (!hit) return;
        }
    }
                
    for (int ring = 1; ; ring++) {
        bool hit = false;
        for (int y = -ring; y <= ring; y++) {
            for (int x = -ring; x <= ring; x++) {
                // These loops enumerate all tile units on and within the ring.
                // Skip tile units that are within (not on) the ring.
                if (abs(x) < ring && abs(y) < ring) continue;
                
                // Find where this tile is on the canvas
                double dx = x - centx;
                double dy = y - centy;
                mOffset.transform(&dx, &dy);
                
                // If the tile might touch the canvas then record it
                agg::rect_d shape(b.mMin_X + dx, b.mMin_Y + dy, b.mMax_X + dx, b.mMax_Y + dy);
                if (shape.overlaps(canvas)) {
                    hit = true;
                    mTileList.push_back(agg::point_d(dx, dy));
                }
            }
        }
        
        if (!hit) return;
    }
}

tiledCanvas::tiledCanvas(Canvas* tile, const agg::trans_affine& tr, CFDG::frieze_t f) 
:   Canvas(tile->mWidth, tile->mHeight), 
    mTile(tile), 
    mTileTransform(tr),
    mFrieze(f)
{
}

void tiledCanvas::scale(double scaleFactor)
{
    agg::trans_affine_scaling scale(scaleFactor);
    
    // Generate the tiling transform in pixel units
    mOffset = mTileTransform * scale;
    
    // The mInvert transform can transform coordinates from the pixel unit tiling
    // to the unit square tiling.
    if (mFrieze) {
        mInvert.reset();
        mInvert.sx = mOffset.sx == 0.0 ? 0.0 : 1/mOffset.sx;
        mInvert.sy = mOffset.sy == 0.0 ? 0.0 : 1/mOffset.sy;
    } else {
        mInvert = ~mOffset;
    }
}

tileList tiledCanvas::getTesselation(int w, int h, int x, int y, bool flipY)
{
    // Produce an integer version of mOffset that is centered in the w x h screen
    agg::trans_affine tess(mWidth, floor(mOffset.shy + 0.5), floor(mOffset.shx + 0.5),
        flipY ? -mHeight : mHeight, x, y);
    agg::rect_i screen(0, 0, w - 1, h - 1);
    if (mFrieze == CFDG::frieze_x)
        tess.sy = 0.0;
    if (mFrieze == CFDG::frieze_y)
        tess.sx = 0.0;
    
    tileList tessPoints;
    tessPoints.push_back(agg::point_i(x, y));   // always include the center tile
    
    if (mFrieze) {
        for (int offset = 1; ; ++offset) {
            bool hit = false;
            for (int side = -1; side <= 1; side += 2) {
                double dx = offset * side;
                double dy = dx;
                tess.transform(&dx, &dy);
                int px = (int)floor(dx + 0.5);
                int py = (int)floor(dy + 0.5);
                
                // If the tile is visible then record it
                agg::rect_i tile(px, py, px + mWidth - 1, py + mHeight - 1);
                if (tile.overlaps(screen)) {
                    hit = true;
                    tessPoints.push_back(agg::point_i(px, py));
                }
            }
            if (!hit) return tessPoints;
        }
    }
    
    // examine rings of tile units around the center unit until you encounter a
    // ring that doesn't have any tile units that intersect the screen. Then stop.
    for (int ring = 1; ; ring++) {
        bool hit = false;
        for (int y = -ring; y <= ring; y++) {
            for (int x = -ring; x <= ring; x++) {
                // These loops enumerate all tile units on and within the ring.
                // Skip tile units that are within (not on) the ring.
                if (abs(x) < ring && abs(y) < ring) continue;
                
                // Find where this tile is on the screen
                double dx = x;
                double dy = y;
                tess.transform(&dx, &dy);
                int px = (int)floor(dx + 0.5);
                int py = (int)floor(dy + 0.5);
                
                // If the tile is visible then record it
                agg::rect_i tile(px, py, px + mWidth - 1, py + mHeight - 1);
                if (tile.overlaps(screen)) {
                    hit = true;
                    tessPoints.push_back(agg::point_i(px, py));
                }
            }
        }
        
        if (!hit) break;
    }
    
    return tessPoints;
}

bool tiledCanvas::isRectangular(int* x_factor, int* y_factor)
{
    if (mFrieze) return false;
    int shx = (int)floor(mOffset.shx + 0.5);
    int shy = (int)floor(mOffset.shy + 0.5);
    
    if (shx == 0 && shy == 0) {
        if (x_factor && y_factor) *x_factor = *y_factor = 1;
        return true;
    }
    
    if (shx && mWidth % abs(shx) == 0) {
        if (x_factor && y_factor) {
            *x_factor = 1;
            *y_factor = mWidth / abs(shx);
        }
        return true;
    }
    
    if (shx && mWidth % (mWidth - abs(shx)) == 0) {
        if (x_factor && y_factor) {
            *x_factor = 1;
            *y_factor = mWidth / (mWidth - abs(shx));
        }
        return true;
    }
    
    if (shy && mHeight % shy == 0) {
        if (x_factor && y_factor) {
            *x_factor = mHeight / abs(shy);
            *y_factor = 1;
        }
        return true;
    }
    
    if (shy && mHeight % (mHeight - abs(shy)) == 0) {
        if (x_factor && y_factor) {
            *x_factor = mHeight / (mHeight - abs(shy));
            *y_factor = 1;
        }
        return true;
    }
    
    return false;
}
