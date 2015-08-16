//
//  TileUtils.h

//
//  Created by Stanislav Sumbera on 10/24/11.

//

#import <Foundation/Foundation.h>
#import <MapKit/MapKit.h>
#import "WebLoaderUtils.h"

#define TILE_SIZE 256
#define MINIMUM_ZOOM 0
#define MAXIMUM_ZOOM 25


// center point
typedef struct XYPoint{
    double x;
	double y;
} XYPoint;

// range of map or layer
typedef struct BBox{
    double  left;
    double  bottom;
    double  right;
    double  top;
} BBox;


//------------------------------------------------------------
NS_INLINE CGRect cgRectForMapRect(MKMapRect mapRect){
    CGRect rect = CGRectMake(mapRect.origin.x, mapRect.origin.y, mapRect.size.width, mapRect.size.height);
    return rect;
}
//------------------------------------------------------------
NS_INLINE  MKMapRect mapRectForCGRect(CGRect cgRect){
    MKMapRect mapRect = MKMapRectMake(cgRect.origin.x,cgRect.origin.y,cgRect.size.width,cgRect.size.height);
    return mapRect;
}
//------------------------------------------------------------
NS_INLINE NSUInteger TileZ(MKZoomScale zoomScale){
    double numTilesAt1_0 = MKMapSizeWorld.width / TILE_SIZE; // 256 is a tile size
    NSInteger zoomLevelAt1_0 = log2(numTilesAt1_0);  // add 1 because the convention skips a virtual level with 1 tile.
    NSInteger zoomLevel = MAX(0, zoomLevelAt1_0 + floor(log2f(zoomScale) + 0.5));
    return zoomLevel;
}

// -- calculates zoom scale form mapview taking in account scale of the main screen
//------------------------------------------------------------
NS_INLINE float mkMapViewZoomScale(MKMapView *mapView){
    float viewWidth =(mapView.frame.size.width*[[UIScreen mainScreen] scale]);
    float mapWidth =mapView.visibleMapRect.size.width;
    float zoomScale =(float) viewWidth/mapWidth;
   // NSLog(@"viewWidth %f  mapWidth %f scale %f",viewWidth,mapWidth,[[UIScreen mainScreen] scale]);
   // NSLog(@"returning zoomScale %f",zoomScale);
    return (float) zoomScale;
    //return (mapView.frame.size.width*[[UIScreen mainScreen] scale]) / mapView.visibleMapRect.size.width;
 }

//------------------------------------------------------------
NS_INLINE NSUInteger TileX(MKMapRect mapRect,MKZoomScale zoomScale){
   return  floor((MKMapRectGetMinX(mapRect) * zoomScale) / TILE_SIZE);
}

//------------------------------------------------------------
NS_INLINE NSUInteger TileY(MKMapRect mapRect,MKZoomScale zoomScale){
    return  floor((MKMapRectGetMinY(mapRect) * zoomScale) / TILE_SIZE);
}

//------------------------------------------------------------
NS_INLINE NSString* selectUrl(NSArray* urls,NSString* params){
    
    if ([urls count]== 1){
        return urls[0];
    } 
    else{
        double golden_ratio = (sqrt(5)-1)/2;
        double product = 1;
        long  length = [params length];
        for (int i=0; i<length; i++) { 
            product *= [params characterAtIndex:i] * golden_ratio; 
            product -= floor(product); 
        }
        int index = floor(product * [urls count]);
        return urls[index];
    }
}

//-------------------------------------------------------------------------------------
NS_INLINE  double MercatorXofLongitude(double lon){
    return  lon * 20037508.34 / 180;
}

//------------------------------------------------------------
NS_INLINE double MercatorYofLatitude(double lat){
    double y = log(tan((90 + lat) * M_PI / 360)) / (M_PI / 180);
    y = y * 20037508.34 / 180;
    
    return y;
}

//----------------------------------------------------------------------------
NS_INLINE double xOfColumn(NSInteger column,NSInteger zoom){

	double x = column;
	double z = zoom;
    
	return x / pow(2.0, z) * 360.0 - 180;
}
//----------------------------------------------------------------------------

NS_INLINE  double yOfRow(NSInteger row,NSInteger zoom){

	double y = row;
	double z = zoom;
    
	double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
	return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

//----------------------------------------------------------------------------
NS_INLINE NSString* TileXYToQuadKeyforTile(NSInteger  tileX,NSInteger tileY,NSInteger levelOfDetail){
	NSMutableString *quadKey = [NSMutableString stringWithCapacity:levelOfDetail];
	
	for (NSInteger i = levelOfDetail; i > 0; --i)
	{
		char digit = '0';
		int mask = 1 << (i - 1);
		if ((tileX & mask) != 0)
		{
			digit++;
		}
		if ((tileY & mask) != 0)
		{
			digit++;
			digit++;
		}
	
       [quadKey appendString:[NSString stringWithFormat:@"%c",digit]];
     //   NSLog(@"x= %d, y= %d, z=%d digit = %c quadkey = %@",tileX,tileY, levelOfDetail, digit, quadKey); 
        
    }
    
   // NSLog(@"-------- quadkey = %@", quadKey); 
     
	return quadKey;
}


//----------------------------------------------------------------------------
NS_INLINE NSString* convertDec2Hex(NSInteger decNumber){
	NSInteger decimal = decNumber;
	NSString* hD=@"0123456789abcdef";
	NSInteger startPos = decimal&15;
	NSString* hex = [hD substringWithRange:NSMakeRange(startPos, 1)];
	
	while (decimal > 15)
	{
		decimal>>=4;
		startPos = decimal&15;
		hex = [NSString stringWithFormat:@"%@%@", [hD substringWithRange:NSMakeRange(startPos, 1)], hex];
	}
	
	while ([hex length] <4)
	{
		hex = [NSString stringWithFormat:@"%@%@", hex, @"0"];
	}
	
	return hex;
}

//----------------------------------------------------------------------------
NS_INLINE NSString * convertDec2Bin(NSInteger input){
	if (input == 1 || input == 0)
		return [NSString stringWithFormat:@"%ld", (long)input];
	
	return [NSString stringWithFormat:@"%@%ld", convertDec2Bin(input / 2), input % 2];
}



NS_INLINE MKMapRect MKMapRectDoubled(MKMapRect mapRect) {
    
    return MKMapRectMake(mapRect.origin.x - (float) (mapRect.size.width /2.0) ,
                         mapRect.origin.y - (float) (mapRect.size.height /2.0),
                         mapRect.size.width *2.0 , // 2.09 is OK. 2.08 is not OK
                         mapRect.size.height *2.0);
    
}

//-------------------------------------------------------------------------------------
NS_INLINE UIImage* TileLoad(NSString* url,BOOL online){ 
    UIImage  *image = nil;
    
    NSString * filePath = filePathFromUrl(url,TILE_CACHE);
    // -- file is cached ?
    if ([[NSFileManager defaultManager] fileExistsAtPath: filePath]){
        image = [UIImage imageWithData:[NSData dataWithContentsOfFile:filePath]];
    }
    else if (online){
        NSData *imgData = [NSData dataWithContentsOfURL:[NSURL URLWithString: url]];
        [imgData writeToFile: filePath atomically:YES];
        image =[UIImage imageWithData:imgData];
    }
    return image;
}


//------------------------------------------------------------
NS_INLINE void TileDraw(UIImage* image, 
                        MKMapRect mapRect, 
                           double contextScale, 
                          CGFloat opacity, 
                     CGContextRef context ){
    
    if (image == nil){
        NSLog(@"Image is nil !");
        return;
    }
    CGRect rect = cgRectForMapRect(mapRect);
    CGContextSaveGState(context);
    {
        CGContextSetAlpha(context, opacity);
        CGContextTranslateCTM(context, CGRectGetMinX(rect), CGRectGetMinY(rect));
        CGContextScaleCTM(context, contextScale, contextScale);
        CGContextTranslateCTM(context, 0, image.size.height);
        CGContextScaleCTM(context, 1, -1);
        CGContextDrawImage(context, CGRectMake(0, 0, image.size.width, image.size.height), [image CGImage]);
    };
    CGContextRestoreGState(context);
    
    // debug
    //CGContextSetRGBStrokeColor(context, 1, 0, 0, 1.0);
    //CGContextSetLineWidth(context, 6.0 / zoomScale);
    //CGContextStrokeRect(context, rect);
    //NSString *countStr = [NSString stringWithFormat:@"+"];
    //CGContextShowTextAtPoint(context, 10.0 , 100.0, [countStr UTF8String] , 2);
    
}
//------------------------------------------------------------
NS_INLINE void TileDrawInRect(UIImage* image, 
                        MKMapRect mapRect, 
                        double contextScale, 
                        CGFloat opacity, 
                        CGContextRef context ){
    
        if (!image)
            NSLog(@"image is nil");
        
        // Perform the image render on the current UI context
        UIGraphicsPushContext(context);
        [image drawInRect:cgRectForMapRect(mapRect) blendMode:kCGBlendModeNormal alpha:opacity];
        UIGraphicsPopContext();
        
}



//-----------------------------------------------------------------
NS_INLINE BBox BBoxFromMKCoordinateRegion(MKCoordinateRegion region){
    BBox bbox;
    
    
    bbox.left =  region.center.longitude - region.span.longitudeDelta / 2.0f;
    bbox.right = region.center.longitude + region.span.longitudeDelta / 2.0f;
    
    bbox.bottom = region.center.latitude - region.span.latitudeDelta /2.0f;
    bbox.top    = region.center.latitude + region.span.latitudeDelta /2.0f;
    
    
   // NSLog (@"left: %0.15f ,right: %0.15f, bottom: %0.15f, top:%0.15f",bbox.left,bbox.right, bbox.bottom, bbox.top);
    return bbox;
}

//-----------------------------------------------------------------
NS_INLINE  MKCoordinateRegion MKCoordinateRegionfromBBox (BBox bbox){
    MKCoordinateRegion region;
    
    region.span.latitudeDelta  = bbox.top - bbox.bottom;
    region.span.longitudeDelta =  bbox.right - bbox.left ;
    region.center.latitude     = bbox.bottom + region.span.latitudeDelta /2; 
    region.center.longitude    = bbox.left   + region.span.longitudeDelta /2; 
    return region;
}


//-----------------------------------------------------------------
NS_INLINE  MKMapRect MKMapRectFromLatLonBBox (BBox bbox){
    MKMapRect mapRect;
    
    // -- get origin (left, top)
    MKMapPoint origin = MKMapPointForCoordinate(CLLocationCoordinate2DMake( bbox.top,bbox.left)) ;
    
    // -- get bottom, right corner
    MKMapPoint  corner = MKMapPointForCoordinate(CLLocationCoordinate2DMake(bbox.bottom,bbox.right)) ;
    
    //-- get MKMapRect origin and size
    mapRect = MKMapRectMake(origin.x,origin.y, fabs(corner.x-origin.x),fabs(corner.y - origin.y));
    
    return mapRect;
}

//-----------------------------------------------------------------
NS_INLINE BBox PlanarBBoxFromMKMapRect (MKMapRect mapRect){
    return  (BBox)
            {.left   = mapRect.origin.x ,
             .bottom = fabs(mapRect.origin.y - mapRect.size.height),
             .right  = mapRect.origin.x + mapRect.size.width,
             .top =  mapRect.origin.y};
    
}
//-----------------------------------------------------------------
NS_INLINE BBox planarBBoxFromLatLonBBox (BBox latlonBBox){
    return PlanarBBoxFromMKMapRect(MKMapRectFromLatLonBBox(latlonBBox));
}

// BBox as [xmin,ymin, xmax, ymax]
//--------------------------------------------------------------------------------------------------
NS_INLINE BBox BBoxFromArray(NSArray* bboxArray){
    // default bbox is whole world
    MKCoordinateRegion worldCoord =  MKCoordinateRegionForMapRect(MKMapRectWorld);
    BBox bbox =  BBoxFromMKCoordinateRegion (worldCoord) ; //{0.0,0.0,0.0,0.0};
   
    if ( bboxArray && ([bboxArray isKindOfClass:[NSArray class]])  && [bboxArray count] ==4){
        bbox.left   = [bboxArray[0] doubleValue];
        bbox.bottom = [bboxArray[1] doubleValue];
        bbox.right  = [bboxArray[2] doubleValue];
        bbox.top    = [bboxArray[3] doubleValue];
    }
    return bbox;
}

//--------------------------------------------------------------------------------------------------
NS_INLINE BOOL BBoxIsValid (BBox bbox){
    return (bbox.left != bbox.right != bbox.top != bbox.bottom ? YES: NO);
}
//--------------------------------------------------------------------------------------------------
NS_INLINE double LongitudeCenterFromBBOX(BBox bbox){
  return  bbox.left + ((bbox.right - bbox.left)/2 );
}

//--------------------------------------------------------------------------------------------------
NS_INLINE double LatitudeCenterFromBBOX(BBox bbox){
    return  bbox.bottom + ((bbox.top - bbox.bottom)/2 );
}



// -- calculates MKMapRect bounds from array
//----------------------------------------------------------------------

NS_INLINE MKMapRect  MKMapRectFromArray(NSArray* bboxArray){
    //-- TODO this should be max bounds rather than 0 MKMapRectWorld
    
    BBox bbox = BBoxFromArray(bboxArray);
    return MKMapRectFromLatLonBBox (bbox);
}
// -- gets min zoom from zoom span array
//--------------------------------------------------------------------------
NS_INLINE int MinZoomFromZoomSpan(NSArray* zoomSpan){
    int minZoom = MINIMUM_ZOOM;
    if([zoomSpan count] > 0){
        minZoom = [((NSNumber*)zoomSpan[0]) intValue];
    }
    return minZoom;
}

// -- gets max zoom from zoom span array
//--------------------------------------------------------------------------
NS_INLINE int  MaxZoomFromZoomSpan(NSArray* zoomSpan){
    int maxZoom = MAXIMUM_ZOOM;
    if([zoomSpan count] > 0){
        maxZoom = [((NSNumber*)zoomSpan[1]) intValue];
    }
    return maxZoom;
}


// -- checks whther overlay is in zoom range
//--------------------------------------------------------------------------
NS_INLINE  BOOL  isZoomLevelInZoomRange (NSUInteger z, NSArray*  zoomSpan){
    if (zoomSpan == nil){
        return YES;
    }
    
    int minZoom  = MinZoomFromZoomSpan(zoomSpan);
    int maxZoom  = MaxZoomFromZoomSpan(zoomSpan);
    
    return (  (z >= minZoom) && (z<= maxZoom) ? YES:NO );

}

// -- checks whther overlay is in zoom range
//--------------------------------------------------------------------------
NS_INLINE BOOL isMapRectInRange(MKMapRect mapRect,NSArray*bboxArray){
   
    if (bboxArray == nil){
        return YES;
    }
    
    BOOL retVal = YES;
    MKMapRect overlayBounds = MKMapRectFromArray(bboxArray);
    //2 -- check bounds
    if (! MKMapRectIsEmpty(overlayBounds)){
        retVal = MKMapRectIntersectsRect(overlayBounds,mapRect);
       
    }
    return retVal;
}
// -- check if overlay is in map  and zoom range
//--------------------------------------------------------------------------
NS_INLINE BOOL isMapAreaInRange(MKMapRect mapRect,MKZoomScale mapZoomScale,NSArray* zoomSpan,NSArray* bboxArray) {
    
    BOOL retValZoom  = isZoomLevelInZoomRange(TileZ(mapZoomScale), zoomSpan);
    BOOL retValRange = isMapRectInRange(mapRect,bboxArray);
    
    return retValRange && retValZoom;
}

//--------------------------------------------------------------------------
NS_INLINE BOOL isBBoxInRange(BBox mapBBox, int mapZoom,NSArray* zoomSpan,NSArray* bboxArray) {
    
    BOOL retValZoom  = isZoomLevelInZoomRange(mapZoom, zoomSpan);
    BOOL retValRange = isMapRectInRange(MKMapRectFromLatLonBBox (mapBBox),bboxArray);
    
    return retValRange && retValZoom;
}


//---------------------------------------------------
NS_INLINE UIImage * tintImage(UIImage* image, CGColorRef tintColor){
    UIGraphicsBeginImageContext(image.size);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    CGContextTranslateCTM(context, 0, image.size.height);
    CGContextScaleCTM(context, 1.0, -1.0);
    
    CGRect rect = CGRectMake(0, 0, image.size.width, image.size.height);
    
    // image drawing code here
    
    CGContextSetBlendMode (context, kCGBlendModeMultiply);
    
    CGContextDrawImage(context, rect, image.CGImage);
    
    CGContextClipToMask(context, rect, image.CGImage);
    
    CGContextSetFillColorWithColor(context, tintColor);
    
    CGContextFillRect(context, rect);
    
    // -- end drawing code
    UIImage *coloredImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return coloredImage;
}
    



