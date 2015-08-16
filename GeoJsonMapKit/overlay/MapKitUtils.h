//
//  TileUtils.h

//
//  Created by Stanislav Sumbera on 10/24/11.

//

#import <Foundation/Foundation.h>
#import <MapKit/MapKit.h>


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

//------------------------------------------------------------
NS_INLINE NSUInteger TileX(MKMapRect mapRect,MKZoomScale zoomScale){
   return  floor((MKMapRectGetMinX(mapRect) * zoomScale) / TILE_SIZE);
}

//------------------------------------------------------------
NS_INLINE NSUInteger TileY(MKMapRect mapRect,MKZoomScale zoomScale){
    return  floor((MKMapRectGetMinY(mapRect) * zoomScale) / TILE_SIZE);
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






