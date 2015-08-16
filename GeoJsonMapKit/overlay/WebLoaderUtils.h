//
//  LoaderUtils.h

//
//  Created by Stanislav Sumbera on 3/3/12.

//
#import <CommonCrypto/CommonDigest.h>
#import <Foundation/Foundation.h>
#import <netdb.h>
#import <SystemConfiguration/SCNetworkReachability.h>


#define TILE_CACHE   @"TILE_CACHE"
#define FEED_CACHE   @"MAP_FEEDS"
#define QUERY_CACHE  @"QUERY_CACHE"
#define IMAGE_CACHE  @"IMAGE_CACHE"
#define OTHER_CACHE  @"OTHER_CACHE"
#define DATABASE_DIR @"DATABASE"
#define EXPORT_DIR   @"EXPORTS"

//--------------------------------------------------------------------------------------------------
NS_INLINE NSString* md5Hash (NSString* stringData) {
    NSData *data = [stringData dataUsingEncoding:NSUTF8StringEncoding];
    unsigned char result[CC_MD5_DIGEST_LENGTH];
    CC_MD5([data bytes], [data length], result);
    
    return [NSString stringWithFormat:
            @"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
            result[0], result[1], result[2], result[3], result[4], result[5], result[6], result[7],
            result[8], result[9], result[10], result[11], result[12], result[13], result[14], result[15]
            ];
}

//--------------------------------------------------------------------------------------------------
NS_INLINE BOOL createPathIfNecessary (NSString* path) {
    BOOL succeeded = YES;
    
    NSFileManager* fm = [NSFileManager defaultManager];
    if (![fm fileExistsAtPath:path]) {
        succeeded = [fm createDirectoryAtPath: path
                  withIntermediateDirectories: YES
                                   attributes: nil
                                        error: nil];
    }
    
    return succeeded;
}

//--------------------------------------------------------------------------------------------------
NS_INLINE  NSString*  pathToFolderInDocuments(NSString* folderName) {
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* documentDirPath     = paths[0];
    NSString* fullPathWithFolder  = [documentDirPath stringByAppendingPathComponent:folderName];
    
    createPathIfNecessary(documentDirPath);
    createPathIfNecessary(fullPathWithFolder);
    
    return fullPathWithFolder;
}


//--------------------------------------------------------------------------------------------------

NS_INLINE  NSString*  filePathFromUrl( NSString* url,NSString* folderName){
    return [pathToFolderInDocuments(folderName) stringByAppendingPathComponent:md5Hash(url)];
}


//------------------------------------------------------------
NS_INLINE void cacheUrlToLocalFolder(NSString* url,NSData* data, NSString* folderName){
    NSString *localFilePath =   filePathFromUrl(url,folderName);
    [data writeToFile: localFilePath atomically:YES];
    
}

//------------------------------------------------------------
NS_INLINE void cacheUrlToLocalPath(NSString* localFilePath,NSData* data){
    [data writeToFile: localFilePath atomically:YES];
    
}
// -- gets filename from url
//------------------------------------------------------------
NS_INLINE NSString* fileNameFromUrlWithoutFragment(NSString* url){
    NSArray  *parts     = [url componentsSeparatedByString:@"/"];
    NSString *fileName  = parts[[parts count]-1];
    
    // -- get only file name without the #fragment to find it in bundle
    NSRange fragmentRange = [fileName rangeOfString:@"#"];
    if (fragmentRange.location != NSNotFound) {
        fileName= [fileName substringToIndex:fragmentRange.location];
    }
    return fileName;
    
}


// -- retrieve file location from main bundle or bundle specified by bundleName
//-------------------------------------------------------------------------------------
NS_INLINE NSString* filePathFromMainOrSpecificBundle(NSString* fileName,NSString* bundleName){
   
    // -- search in main bundle first
    NSString *bundleFileLocation = [[NSBundle mainBundle] pathForResource:fileName ofType:nil];
    
    
    // -- search in  bundle
    if ((!bundleFileLocation) && (bundleName)){
        NSString *mapBundlePath = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent: bundleName];
        bundleFileLocation = [[NSBundle bundleWithPath: mapBundlePath] pathForResource:fileName ofType:nil];
    }
    return bundleFileLocation;
}

//------------------------------------------------------------
NS_INLINE BOOL connectedToNetwork()
{
    // Create zero addy
    struct sockaddr_in zeroAddress;
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.sin_len = sizeof(zeroAddress);
    zeroAddress.sin_family = AF_INET;
    
    // Recover reachability flags
    SCNetworkReachabilityRef defaultRouteReachability = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&zeroAddress);
    SCNetworkReachabilityFlags flags;
    
    BOOL didRetrieveFlags = SCNetworkReachabilityGetFlags(defaultRouteReachability, &flags);
    CFRelease(defaultRouteReachability);
    
    if (!didRetrieveFlags)
    {
        printf("Error. Could not recover network reachability flags\n");
        return 0;
    }
    
    BOOL isReachable = flags & kSCNetworkFlagsReachable;
    BOOL needsConnection = flags & kSCNetworkFlagsConnectionRequired;
    return (isReachable && !needsConnection) ? YES : NO;
}
