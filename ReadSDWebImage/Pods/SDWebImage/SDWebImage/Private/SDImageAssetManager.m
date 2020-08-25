/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import "SDImageAssetManager.h"
#import "SDInternalMacros.h"

/**
 * 绑定缩放比例,返回数组
 */
static NSArray *SDBundlePreferredScales() {
    static NSArray *scales;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
#if SD_WATCH
        CGFloat screenScale = [WKInterfaceDevice currentDevice].screenScale;
#elif SD_UIKIT
        //UIScreen.mainScreen.scale:2.000000
        CGFloat screenScale = [UIScreen mainScreen].scale;
#elif SD_MAC
        CGFloat screenScale = [NSScreen mainScreen].backingScaleFactor;
#endif
        if (screenScale <= 1) {
            scales = @[@1,@2,@3];
        } else if (screenScale <= 2) {
            scales = @[@2,@3,@1];
        } else {
            scales = @[@3,@2,@1];
        }
    });
    return scales;
}

@implementation SDImageAssetManager {
    dispatch_semaphore_t _lock;
}

+ (instancetype)sharedAssetManager {
    static dispatch_once_t onceToken;
    static SDImageAssetManager *assetManager;
    dispatch_once(&onceToken, ^{
        assetManager = [[SDImageAssetManager alloc] init];
    });
    return assetManager;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        /*
         NSPointerFunctionsOptions 是内存管理策略的枚举值，主要有NSPointerFunctionsWeakMemory，NSPointerFunctionsStrongMemory两个值，代表弱引用和强引用。而NSPointerFunctionsWeakMemory就是我们想要的。不仅如此，NSMapTable的key和value都可以分别设置弱引用或强引用，非常强大。
         如果弱引用的对象被释放了会怎么样？
         如果key对象被释放, key=nil，那么key和value都会被移除，至少遍历的时候不会遍历掉。
         如果value对象被释放，那么key存在，value=nil
         */
        NSPointerFunctionsOptions valueOptions;
#if SD_MAC
        // Apple says that NSImage use a weak reference to value
        valueOptions = NSPointerFunctionsWeakMemory;
#else
        // Apple says that UIImage use a strong reference to value
        valueOptions = NSPointerFunctionsStrongMemory;
#endif
        _imageTable = [NSMapTable mapTableWithKeyOptions:NSPointerFunctionsCopyIn valueOptions:valueOptions];
        _lock = dispatch_semaphore_create(1);
#if SD_UIKIT
        //初始化注册通知
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didReceiveMemoryWarning:) name:UIApplicationDidReceiveMemoryWarningNotification object:nil];
#endif
    }
    return self;
}

- (void)dealloc {
#if SD_UIKIT
    //销毁时取消通知
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidReceiveMemoryWarningNotification object:nil];
#endif
}

- (void)didReceiveMemoryWarning:(NSNotification *)notification {
    SD_LOCK(_lock);
    [self.imageTable removeAllObjects];
    SD_UNLOCK(_lock);
}

/**
 * name: 图片名称
 * bundle: 绑定
 * scale: 缩放比例
 */
- (NSString *)getPathForName:(NSString *)name bundle:(NSBundle *)bundle preferredScale:(CGFloat *)scale {
    NSParameterAssert(name);
    NSParameterAssert(bundle);
    NSString *path;
    if (name.length == 0) {
        return path;
    }
    if ([name hasSuffix:@"/"]) {
        return path;
    }
    
    //拓展名
    NSString *extension = name.pathExtension;
    if (extension.length == 0) {
        // If no extension, follow Apple's doc, check PNG format
        // 没有拓展名则,增加拓展名png
        // 可以根据这个操作,进一步修改Paintinglite框架初始化部分
        extension = @"png";
    }
    name = [name stringByDeletingPathExtension];
    
    //获得缩放比例scale
    CGFloat providedScale = *scale;
    NSArray *scales = SDBundlePreferredScales();
    
    // Check if file name contains scale
    // 检查文件名称包含缩放比例
    for (size_t i = 0; i < scales.count; i++) {
        NSNumber *scaleValue = scales[i];
        if ([name hasSuffix:[NSString stringWithFormat:@"@%@x", scaleValue]]) {
            path = [bundle pathForResource:name ofType:extension];
            if (path) {
                *scale = scaleValue.doubleValue; // override
                return path;
            }
        }
    }
    
    // Search with provided scale first
    if (providedScale != 0) {
        NSString *scaledName = [name stringByAppendingFormat:@"@%@x", @(providedScale)];
        path = [bundle pathForResource:scaledName ofType:extension];
        if (path) {
            return path;
        }
    }
    
    // Search with preferred scale
    for (size_t i = 0; i < scales.count; i++) {
        NSNumber *scaleValue = scales[i];
        if (scaleValue.doubleValue == providedScale) {
            // Ignore provided scale
            continue;
        }
        NSString *scaledName = [name stringByAppendingFormat:@"@%@x", scaleValue];
        path = [bundle pathForResource:scaledName ofType:extension];
        if (path) {
            *scale = scaleValue.doubleValue; // override
            return path;
        }
    }
    
    // Search without scale
    //[[NSBundle mainBundle] pathForResource:@"" ofType:@""]
    path = [bundle pathForResource:name ofType:extension];
    
    return path;
}

- (UIImage *)imageForName:(NSString *)name {
    NSParameterAssert(name);
    UIImage *image;
    SD_LOCK(_lock);
    image = [self.imageTable objectForKey:name];
    SD_UNLOCK(_lock);
    return image;
}

- (void)storeImage:(UIImage *)image forName:(NSString *)name {
    NSParameterAssert(image);
    NSParameterAssert(name);
    SD_LOCK(_lock);
    [self.imageTable setObject:image forKey:name];
    SD_UNLOCK(_lock);
}

@end
