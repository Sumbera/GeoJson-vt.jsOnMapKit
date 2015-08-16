//
//  MultiPolygon.m
//  SHPonMapKit
//
//  Created by Stanislav Sumbera on 27/05/15.
//  code taken from http://stackoverflow.com/questions/17673410/mkmapview-with-multiple-overlays-memory-issue


#import <Foundation/Foundation.h>
#import "MultiPolygonOverlay.h"
#import "MapKitUtils.h"

//------------------------------------------------------------
NS_INLINE void loadScript(JSContext *jsc,NSString* scriptFile){
    NSError *error;
    NSString *scriptBundle = [[NSBundle mainBundle] resourcePath];
    NSString *scriptFilePath =[scriptBundle stringByAppendingPathComponent:scriptFile];
    NSString *scriptContent= [NSString stringWithContentsOfFile:scriptFilePath encoding:NSUTF8StringEncoding error:&error];
    NSURL    *scriptUrl = [NSURL URLWithString:scriptFile];
    
    [jsc evaluateScript: scriptContent withSourceURL:scriptUrl];
}

//------------------------------------------------------------
NS_INLINE JSValue* callJSFuncByName(JSContext *jsc,NSString* funcName, NSArray *args){
    JSValue *func = jsc[funcName];
    return [func callWithArguments:args];
}




@implementation MultiPolygonOverlay{
    NSString *_filecontent;
    JSValue *_tileIndex;
    
    
  
}

@synthesize polygons = _polygons;
@synthesize jsctx    = _jsctx;



//------------------------------------------------------
- (id)initWithLocalGeoJsonFile:(NSString *)filePath{
   if (self = [super init]) {
       NSError *error;
        NSString *myData= [NSString stringWithContentsOfFile:filePath encoding:NSUTF8StringEncoding error:&error];
       _filecontent = myData;
       _jsctx = [[JSContext alloc ]init];
       _jsctx.name = @"GeoJsonMapkit";
       _jsctx.exceptionHandler = ^(JSContext *context, JSValue *exception) {
           NSLog(@"JS Error: %@", exception);
       };

       
       loadScript(_jsctx,@"geojson-vt-dev.js");
       loadScript(_jsctx,@"index.js");
       
       
       _tileIndex = callJSFuncByName(_jsctx,@"indexGeoJson",@[myData]);
       
       
       
  
       JSValue *json = [_tileIndex[@"getBoundingBox"]  callWithArguments:NULL];
       
    
       NSDictionary *jsonDict = [json toDictionary];
       BBox boundingBBox = BBoxFromArray(@[
                                           jsonDict[@"xMin"] ,
                                           jsonDict[@"yMin"] ,
                                           jsonDict[@"xMax"] ,
                                            jsonDict[@"yMax"]
                                           ]);
        MKMapRect boundingMapRect = MKMapRectFromLatLonBBox (boundingBBox);
       
       
       _boundingMapRect = boundingMapRect;
       
       _boundingMapRect = MKMapRectWorld;
       
       
    }
    return self;
 
}
//-----------------------------------------------------------
-(NSDictionary*) getTileX:(NSUInteger) x Y:(NSUInteger) y Z:(NSUInteger)z{
    
    NSDate *methodStart = [NSDate date];
    
    JSValue *getTile = _jsctx[@"getTile"];
    JSValue *result =  [getTile callWithArguments:@[@(z),@(x),@(y)]];
  

  
    NSDictionary * resDict = [result toDictionary];
    
    NSDate *methodFinish = [NSDate date];
    NSTimeInterval executionTime = [methodFinish timeIntervalSinceDate:methodStart];
    NSLog(@"executionTime = %f", executionTime);
    return resDict;
    
}

//-----------------------------------------------------------
- (MKMapRect)boundingMapRect
{
    return  _boundingMapRect ;
}
//-----------------------------------------------------------

- (CLLocationCoordinate2D)coordinate
{
    return MKCoordinateForMapPoint(MKMapPointMake(MKMapRectGetMidX(_boundingMapRect), MKMapRectGetMidY(_boundingMapRect)));
}

@end