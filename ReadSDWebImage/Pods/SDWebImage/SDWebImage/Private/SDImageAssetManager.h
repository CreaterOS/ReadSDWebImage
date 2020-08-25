/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import <Foundation/Foundation.h>
#import "SDWebImageCompat.h"

// Apple parse the Asset Catalog compiled file(`Assets.car`) by CoreUI.framework, however it's a private framework and there are no other ways to directly get the data. So we just process the normal bundle files :)
/**
 苹果公司通过CoreUI.framework解析了Asset Catalog编译的文件（Assets.car`），但是它是一个私有框架，没有其他直接获取数据的方法。所以我们只处理普通的捆绑文件:)
 SDImageAssetManager用来解析捆绑文件
 */
@interface SDImageAssetManager : NSObject

@property (nonatomic, strong, nonnull) NSMapTable<NSString *, UIImage *> *imageTable;
//解析文件,整个程序共用一个AssetManager即可
+ (nonnull instancetype)sharedAssetManager;
- (nullable NSString *)getPathForName:(nonnull NSString *)name bundle:(nonnull NSBundle *)bundle preferredScale:(nonnull CGFloat *)scale;
- (nullable UIImage *)imageForName:(nonnull NSString *)name;
- (void)storeImage:(nonnull UIImage *)image forName:(nonnull NSString *)name;

@end
