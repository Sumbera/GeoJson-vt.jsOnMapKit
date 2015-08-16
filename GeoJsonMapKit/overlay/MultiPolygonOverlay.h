//
//  MultiPolygon.h
//  SHPonMapKit
//
//  Created by Stanislav Sumbera on 27/05/15.
//
//

#import <MapKit/MapKit.h> //Add import MapKit
#import <JavaScriptCore/JavaScriptCore.h>



@interface MultiPolygonOverlay : NSObject <MKOverlay> {
    NSArray *_polygons;
    MKMapRect _boundingMapRect;

}

- (id)initWithLocalGeoJsonFile:(NSString *)filePath;
-(NSDictionary*) getTileX:(NSUInteger) x Y:(NSUInteger) y Z:(NSUInteger)z;


@property (nonatomic, readonly) NSArray   *polygons;
@property (nonatomic,readonly)  JSContext *jsctx;

@end
