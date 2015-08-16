//
//  MultiPolygonView.m
//  SHPonMapKit
//
//  Created by Stanislav Sumbera on 27/05/15.
//  code taken from http://stackoverflow.com/questions/17673410/mkmapview-with-multiple-overlays-memory-issue

#import <CoreGraphics/CGBase.h>
#import "MultiPolygonOverlay.h"
#import "MultiPolygonRenderer.h"

#import "MapKitUtils.h"

//------------------------------------------------------------
NS_INLINE int toID(int z, int x, int y) {
    return (((1 << z) * y + x) * 32) + z;
}



@implementation MultiPolygonRenderer{
    NSMutableDictionary * _tileCache;
}



//--------------------------------------------------------------------------
- (BOOL)canDrawMapRect:(MKMapRect)mapRect  zoomScale:(MKZoomScale)zoomScale{
    MultiPolygonOverlay *multiPolygon = (MultiPolygonOverlay *)self.overlay;
    // Zoom
    NSUInteger z = TileZ(zoomScale);
    NSUInteger x = TileX(mapRect, zoomScale);
    NSUInteger y = TileY(mapRect,zoomScale);

    NSDictionary *tileDict;
    NSString *key = [NSString stringWithFormat:@"%ld",(long)toID(z,x,y) ];
    
    // -- cached tile available ?
    if (_tileCache[key]){
        return YES;
    }
    else{
        // -- no cache, need to load
        @synchronized(self){
         tileDict = [multiPolygon getTileX:x Y:y Z:z];
        _tileCache = _tileCache ? _tileCache:[[NSMutableDictionary alloc] init];
         if (tileDict){
            _tileCache[key] = tileDict;
         }
         else{
             _tileCache[key] = @{};
         }
        }
        return YES;
    }
}

//--------------------------------------------------------------------------
- (void)drawMapRect:(MKMapRect)mapRect
          zoomScale:(MKZoomScale)zoomScale
          inContext:(CGContextRef)context
{
    MultiPolygonOverlay *multiPolygon = (MultiPolygonOverlay *)self.overlay;
    // Zoom
    NSUInteger z = TileZ(zoomScale);
    NSUInteger x = TileX(mapRect, zoomScale);
    NSUInteger y = TileY(mapRect,zoomScale);
    
    NSString *key = [NSString stringWithFormat:@"%ld",(long)toID(z,x,y) ];
    NSDictionary *tileDict =_tileCache[key];
   [_tileCache removeObjectForKey:key];

    
    if (!tileDict || !tileDict[@"features"]){
        return;
    }
  
    /*
    __block NSDictionary *tileDict;
  //  @synchronized(self){
    dispatch_sync(dispatch_get_main_queue(), ^{
        tileDict = [multiPolygon getTileX:x Y:y Z:z];
    });
   
    if (!tileDict)
        return;
    //}
*/
    
    CGRect cgRectRelative = [self rectForMapRect:mapRect];
    
   
    
    NSArray *features = tileDict[@"features"];
    
    double extent = 4096;
    double size = cgRectRelative.size.width;
    
    for (NSDictionary  *geomDict in features) {
        // -- itterate over features
        CGMutablePathRef path = NULL;
        NSArray * geomRingsArray = geomDict[@"geometry"];
        
        // -- itterate over rings
        for (NSArray* geomPoints in geomRingsArray) {
          path = CGPathCreateMutable();
            for (int i = 0; i< [geomPoints count ]; i++) {
                NSArray *xyArray = geomPoints[i];
                double  x =  (double) ([xyArray[0] doubleValue] / extent) * size + cgRectRelative.origin.x;
                double  y =  (double) ([xyArray[1] doubleValue]/ extent ) * size + cgRectRelative.origin.y ;

                if (i == 0) {
                    CGPathMoveToPoint(path, NULL, x, y);
                } else {
                    CGPathAddLineToPoint(path, NULL, x, y);
                }

                
            }
            
       
        if (path) {
            [self applyFillPropertiesToContext:context atZoomScale:zoomScale];
            CGContextBeginPath(context);
            CGContextAddPath(context, path);
            CGContextDrawPath(context, kCGPathEOFill);
            [self applyStrokePropertiesToContext:context atZoomScale:zoomScale];
            CGContextBeginPath(context);
            CGContextAddPath(context, path);
            CGContextStrokePath(context);
            CGPathRelease(path);
        }
     }
       
        
    }

}

/*
//--------------------------------------------------------------------------
- (void)__drawMapRect:(MKMapRect)mapRect
          zoomScale:(MKZoomScale)zoomScale
          inContext:(CGContextRef)context
{
    MultiPolygonOverlay *multiPolygon = (MultiPolygonOverlay *)self.overlay;
    GeoJSONVT *vt = multiPolygon.vt;
    
    // Zoom
    NSUInteger z = TileZ(zoomScale);
    NSUInteger x = TileX(mapRect, zoomScale);
    NSUInteger y = TileY(mapRect,zoomScale);
 
    
    
    NSDate *methodStart = [NSDate date];
    Tile *tile = [vt  getTile:z x:x y:y];
    NSDate *methodFinish = [NSDate date];
    NSTimeInterval executionTime = [methodFinish timeIntervalSinceDate:methodStart];
    NSLog(@"swiftTime = %f", executionTime);

    
    double extent = 4096;
    double size = 256;
    for (TileFeature *feature in tile.features  ) {
        CGMutablePathRef path = NULL;
        for (TileGeometry * geometry in feature.geometry ){
            if (feature.type == ProjectedFeatureTypePoint) {
                ///let radius: CGFloat = 1
                //let point = geometry as! TilePoint
                //let x = CGFloat((Double(point.x) / extent) * Double(size))
                //let y = CGFloat((Double(point.y) / extent) * Double(size))
                //let dot = CGRect(x: (x - radius), y: (y - radius), width: (radius * 2), height: (radius * 2))
                //CGContextAddEllipseInRect(c, dot);
            } else {
                long  pointCount = 0;
                TileRing *ring = (TileRing*) geometry ;
                //----
                path = CGPathCreateMutable();

                for (TilePoint *point in ring.points) {
                    double  x = ( (point.x) / extent) * (size);
                    double  y = ( (point.y) / extent) * (size);
                    if (pointCount == 0) {
                        CGPathMoveToPoint(path, NULL, x, y);
                    } else {
                        CGPathAddLineToPoint(path, NULL, x, y);
                    }
                 pointCount++;
                } // for tile point
            }
            
        } // **end for geometry
        
        if (path) {
            [self applyFillPropertiesToContext:context atZoomScale:zoomScale];
            CGContextBeginPath(context);
            CGContextAddPath(context, path);
            CGContextDrawPath(context, kCGPathEOFill);
            [self applyStrokePropertiesToContext:context atZoomScale:zoomScale];
            CGContextBeginPath(context);
            CGContextAddPath(context, path);
            CGContextStrokePath(context);
            
            CGPathRelease(path);
        }
    }
    
   // for (MKPolygon *polygon in multiPolygon.polygons) {

}
    */

@end
