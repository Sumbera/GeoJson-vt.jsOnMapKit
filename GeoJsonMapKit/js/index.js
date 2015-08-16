
var tileIndex ;

function indexGeoJson(stringData){
    var _tileOptions = {
        maxZoom: 20,  // max zoom to preserve detail on
        tolerance: 25, // simplification tolerance (higher means simpler)
        extent: 4096, // tile extent (both width and height)
        buffer: 12,   // tile buffer on each side
        debug: 0,      // logging level (0 to disable, 1 or 2)
            
        indexMaxZoom: 0,        // max zoom in the initial tile index
        indexMaxPoints: 10000, // max number of points per tile in the index
    };
    
    var _data = JSON.parse(stringData);
    
    tileIndex = geojsonvt(_data, _tileOptions);
    //----------------------------------------
    tileIndex.data = function(arg){
        if (!arguments.length)
            return _data;
        _data= arg;
        
        return tileIndex;
    }
    //----------------------------------------
    tileIndex.getBoundingBox = function () {
        var data = _data;
        var bounds = {}, coords, point, latitude, longitude;
        
        // We want to use the “features” key of the FeatureCollection (see above)
        data = data.features;
        
        // Loop through each “feature”
        for (var i = 0; i < data.length; i++) {
            
            // Pull out the coordinates of this feature
            coords = data[i].geometry.coordinates[0];
            
            // For each individual coordinate in this feature's coordinates…
            for (var j = 0; j < coords.length; j++) {
                
                longitude = coords[j][0];
                latitude = coords[j][1];
                
                // Update the bounds recursively by comparing the current
                // xMin/xMax and yMin/yMax with the coordinate
                // we're currently checking
                bounds.xMin = bounds.xMin < longitude ? bounds.xMin : longitude;
                bounds.xMax = bounds.xMax > longitude ? bounds.xMax : longitude;
                bounds.yMin = bounds.yMin < latitude ? bounds.yMin : latitude;
                bounds.yMax = bounds.yMax > latitude ? bounds.yMax : latitude;
            }
            
        }
        
        // Returns an object that contains the bounds of this GeoJSON
        // data. The keys of this object describe a box formed by the
        // northwest (xMin, yMin) and southeast (xMax, yMax) coordinates.
        return bounds;
    }

    return tileIndex;
}


function getTile(z,x,y){
    var result = tileIndex.getTile(z, x, y);
    
    var xres;
    if (result  && result.features)
       xres = { features: result.features};
    
    return xres;
}

function getBoundingBox (data) {
    var bounds = {}, coords, point, latitude, longitude;
    
    // We want to use the “features” key of the FeatureCollection (see above)
    data = data.features;
    
    // Loop through each “feature”
    for (var i = 0; i < data.length; i++) {
        
        // Pull out the coordinates of this feature
        coords = data[i].geometry.coordinates[0];
        
        // For each individual coordinate in this feature's coordinates…
        for (var j = 0; j < coords.length; j++) {
            
            longitude = coords[j][0];
            latitude = coords[j][1];
            
            // Update the bounds recursively by comparing the current
            // xMin/xMax and yMin/yMax with the coordinate
            // we're currently checking
            bounds.xMin = bounds.xMin < longitude ? bounds.xMin : longitude;
            bounds.xMax = bounds.xMax > longitude ? bounds.xMax : longitude;
            bounds.yMin = bounds.yMin < latitude ? bounds.yMin : latitude;
            bounds.yMax = bounds.yMax > latitude ? bounds.yMax : latitude;
        }
        
    }
    
    // Returns an object that contains the bounds of this GeoJSON
    // data. The keys of this object describe a box formed by the
    // northwest (xMin, yMin) and southeast (xMax, yMax) coordinates.
    return bounds;
}

