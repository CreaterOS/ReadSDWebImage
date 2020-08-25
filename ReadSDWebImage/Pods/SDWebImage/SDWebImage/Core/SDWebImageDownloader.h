/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import <Foundation/Foundation.h>
#import "SDWebImageCompat.h"
#import "SDWebImageDefine.h"
#import "SDWebImageOperation.h"
#import "SDWebImageDownloaderConfig.h"
#import "SDWebImageDownloaderRequestModifier.h"
#import "SDWebImageDownloaderResponseModifier.h"
#import "SDWebImageDownloaderDecryptor.h"
#import "SDImageLoader.h"

/// Downloader options
// SDWebImage下载策略
typedef NS_OPTIONS(NSUInteger, SDWebImageDownloaderOptions) {
    /**
     * Put the download in the low queue priority and task priority.
     *  下载任务在最低的线程和任务优先级
     */
    SDWebImageDownloaderLowPriority = 1 << 0,
    
    /**
     * This flag enables progressive download, the image is displayed progressively during download as a browser would do.
     * 该标志启用渐进式下载，在下载过程中图像会像浏览器一样逐步显示。
     */
    SDWebImageDownloaderProgressiveLoad = 1 << 1,

    /**
     * By default, request prevent the use of NSURLCache. With this flag, NSURLCache
     * is used with default policies.
     * 默认情况下，请求阻止使用NSURLCache。带有此标志，NSURLCache与默认策略一起使用。
     */
    SDWebImageDownloaderUseNSURLCache = 1 << 2,

    /**
     * Call completion block with nil image/imageData if the image was read from NSURLCache
     * And the error code is `SDWebImageErrorCacheNotModified`
     * This flag should be combined with `SDWebImageDownloaderUseNSURLCache`.
     * 如果从NSURLCache读取了图像，则用nil image / imageData调用完成块，错误代码为SDWebImageErrorCacheNotModified。
     * 此标志应与SDWebImageDownloaderUseNSURLCache结合使用。
     */
    SDWebImageDownloaderIgnoreCachedResponse = 1 << 3,
    
    /**
     * In iOS 4+, continue the download of the image if the app goes to background. This is achieved by asking the system for
     * extra time in background to let the request finish. If the background task expires the operation will be cancelled.
     */
    SDWebImageDownloaderContinueInBackground = 1 << 4,

    /**
     * Handles cookies stored in NSHTTPCookieStore by setting 
     * NSMutableURLRequest.HTTPShouldHandleCookies = YES;
     */
    SDWebImageDownloaderHandleCookies = 1 << 5,

    /**
     * Enable to allow untrusted SSL certificates.
     * Useful for testing purposes. Use with caution in production.
     */
    SDWebImageDownloaderAllowInvalidSSLCertificates = 1 << 6,

    /**
     * Put the download in the high queue priority and task priority.
     * 下载线程和任务优先级最高
     */
    SDWebImageDownloaderHighPriority = 1 << 7,
    
    /**
     * By default, images are decoded respecting their original size. On iOS, this flag will scale down the
     * images to a size compatible with the constrained memory of devices.
     * This flag take no effect if `SDWebImageDownloaderAvoidDecodeImage` is set. And it will be ignored if `SDWebImageDownloaderProgressiveLoad` is set.
     * 默认情况下，图像会根据其原始大小进行解码。
     * 在iOS上，此标志会将图像缩小到与设备的受限内存兼容的大小。
     * 设置了SDWebImageDownloaderAvoidDecodeImage，则此标志无效。
     * 设置了SDWebImageDownloaderProgressiveLoad，它将被忽略。
     */
    SDWebImageDownloaderScaleDownLargeImages = 1 << 8,
    
    /**
     * By default, we will decode the image in the background during cache query and download from the network. This can help to improve performance because when rendering image on the screen, it need to be firstly decoded. But this happen on the main queue by Core Animation.
     * However, this process may increase the memory usage as well. If you are experiencing a issue due to excessive memory consumption, This flag can prevent decode the image.
     * 默认情况下，我们将在缓存查询和从网络下载期间在后台解码图像。
     * 这有助于提高性能，因为在屏幕上渲染图像时，首先需要对其进行解码。但这是由Core Animation在主队列上发生的。
     * 此过程也可能会增加内存使用率。
     * 由于过多的内存消耗而遇到问题，则此标志可以防止对图像进行解码。
     */
    SDWebImageDownloaderAvoidDecodeImage = 1 << 9,
    
    /**
     * By default, we decode the animated image. This flag can force decode the first frame only and produece the static image.
     * 默认情况下，我们对动画图像进行解码。该标志只能强制解码第一帧并生成静态图像。
     */
    SDWebImageDownloaderDecodeFirstFrameOnly = 1 << 10,
    
    /**
     * By default, for `SDAnimatedImage`, we decode the animated image frame during rendering to reduce memory usage. This flag actually trigger `preloadAllAnimatedImageFrames = YES` after image load from network
     * 默认情况下，对于“ SDAnimatedImage”，我们在渲染过程中对动画图像帧进行解码以减少内存使用量。
     * 从网络加载图像后，该标志实际上触发了“ preloadAllAnimatedImageFrames = YES”
     */
    SDWebImageDownloaderPreloadAllFrames = 1 << 11,
    
    /**
     * By default, when you use `SDWebImageContextAnimatedImageClass` context option (like using `SDAnimatedImageView` which designed to use `SDAnimatedImage`), we may still use `UIImage` when the memory cache hit, or image decoder is not available, to behave as a fallback solution.
     * Using this option, can ensure we always produce image with your provided class. If failed, a error with code `SDWebImageErrorBadImageData` will been used.
     * Note this options is not compatible with `SDWebImageDownloaderDecodeFirstFrameOnly`, which always produce a UIImage/NSImage.
     * 默认情况下，当您使用SDWebImageContextAnimatedImageClass上下文选项（例如使用设计为使用SDAnimatedImage的SDAnimatedImageView）时，当内存缓存命中或图像解码器不可用时，我们仍可以使用UIImage来表现后备解决方案。
     * 使用此选项，可以确保我们始终使用您提供的类生成图像。如果失败，将使用代码“ SDWebImageErrorBadImageData”的错误。
     * 请注意，此选项与总是生成UIImage / NSImage的SDWebImageDownloaderDecodeFirstFrameOnly不兼容。
     */
    SDWebImageDownloaderMatchAnimatedImageClass = 1 << 12,
};

FOUNDATION_EXPORT NSNotificationName _Nonnull const SDWebImageDownloadStartNotification;
FOUNDATION_EXPORT NSNotificationName _Nonnull const SDWebImageDownloadReceiveResponseNotification;
FOUNDATION_EXPORT NSNotificationName _Nonnull const SDWebImageDownloadStopNotification;
FOUNDATION_EXPORT NSNotificationName _Nonnull const SDWebImageDownloadFinishNotification;

typedef SDImageLoaderProgressBlock SDWebImageDownloaderProgressBlock;
typedef SDImageLoaderCompletedBlock SDWebImageDownloaderCompletedBlock;

/**
 *  A token associated with each download. Can be used to cancel a download
 */
@interface SDWebImageDownloadToken : NSObject <SDWebImageOperation>

/**
 Cancel the current download.
 */
- (void)cancel;

/**
 The download's URL.
 */
@property (nonatomic, strong, nullable, readonly) NSURL *url;

/**
 The download's request.
 */
@property (nonatomic, strong, nullable, readonly) NSURLRequest *request;

/**
 The download's response.
 */
@property (nonatomic, strong, nullable, readonly) NSURLResponse *response;

@end


/**
 * Asynchronous downloader dedicated and optimized for image loading.
 */
@interface SDWebImageDownloader : NSObject

/**
 * Downloader Config object - storing all kind of settings.
 * Most config properties support dynamic changes during download, except something like `sessionConfiguration`, see `SDWebImageDownloaderConfig` for more detail.
 */
@property (nonatomic, copy, readonly, nonnull) SDWebImageDownloaderConfig *config;

/**
 * Set the request modifier to modify the original download request before image load.
 * This request modifier method will be called for each downloading image request. Return the original request means no modification. Return nil will cancel the download request.
 * Defaults to nil, means does not modify the original download request.
 * @note If you want to modify single request, consider using `SDWebImageContextDownloadRequestModifier` context option.
 */
@property (nonatomic, strong, nullable) id<SDWebImageDownloaderRequestModifier> requestModifier;

/**
 * Set the response modifier to modify the original download response during image load.
 * This request modifier method will be called for each downloading image response. Return the original response means no modification. Return nil will mark current download as cancelled.
 * Defaults to nil, means does not modify the original download response.
 * @note If you want to modify single response, consider using `SDWebImageContextDownloadResponseModifier` context option.
 */
@property (nonatomic, strong, nullable) id<SDWebImageDownloaderResponseModifier> responseModifier;

/**
 * Set the decryptor to decrypt the original download data before image decoding. This can be used for encrypted image data, like Base64.
 * This decryptor method will be called for each downloading image data. Return the original data means no modification. Return nil will mark this download failed.
 * Defaults to nil, means does not modify the original download data.
 * @note When using decryptor, progressive decoding will be disabled, to avoid data corrupt issue.
 * @note If you want to decrypt single download data, consider using `SDWebImageContextDownloadDecryptor` context option.
 */
@property (nonatomic, strong, nullable) id<SDWebImageDownloaderDecryptor> decryptor;

/**
 * The configuration in use by the internal NSURLSession. If you want to provide a custom sessionConfiguration, use `SDWebImageDownloaderConfig.sessionConfiguration` and create a new downloader instance.
 @note This is immutable according to NSURLSession's documentation. Mutating this object directly has no effect.
 */
@property (nonatomic, readonly, nonnull) NSURLSessionConfiguration *sessionConfiguration;

/**
 * Gets/Sets the download queue suspension state.
 */
@property (nonatomic, assign, getter=isSuspended) BOOL suspended;

/**
 * Shows the current amount of downloads that still need to be downloaded
 */
@property (nonatomic, assign, readonly) NSUInteger currentDownloadCount;

/**
 *  Returns the global shared downloader instance. Which use the `SDWebImageDownloaderConfig.defaultDownloaderConfig` config.
 */
@property (nonatomic, class, readonly, nonnull) SDWebImageDownloader *sharedDownloader;

/**
 Creates an instance of a downloader with specified downloader config.
 You can specify session configuration, timeout or operation class through downloader config.

 @param config The downloader config. If you specify nil, the `defaultDownloaderConfig` will be used.
 @return new instance of downloader class
 */
- (nonnull instancetype)initWithConfig:(nullable SDWebImageDownloaderConfig *)config NS_DESIGNATED_INITIALIZER;

/**
 * Set a value for a HTTP header to be appended to each download HTTP request.
 *
 * @param value The value for the header field. Use `nil` value to remove the header field.
 * @param field The name of the header field to set.
 */
- (void)setValue:(nullable NSString *)value forHTTPHeaderField:(nullable NSString *)field;

/**
 * Returns the value of the specified HTTP header field.
 *
 * @return The value associated with the header field field, or `nil` if there is no corresponding header field.
 */
- (nullable NSString *)valueForHTTPHeaderField:(nullable NSString *)field;

/**
 * Creates a SDWebImageDownloader async downloader instance with a given URL
 *
 * The delegate will be informed when the image is finish downloaded or an error has happen.
 *
 * @see SDWebImageDownloaderDelegate
 *
 * @param url            The URL to the image to download
 * @param completedBlock A block called once the download is completed.
 *                       If the download succeeded, the image parameter is set, in case of error,
 *                       error parameter is set with the error. The last parameter is always YES
 *                       if SDWebImageDownloaderProgressiveDownload isn't use. With the
 *                       SDWebImageDownloaderProgressiveDownload option, this block is called
 *                       repeatedly with the partial image object and the finished argument set to NO
 *                       before to be called a last time with the full image and finished argument
 *                       set to YES. In case of error, the finished argument is always YES.
 *
 * @return A token (SDWebImageDownloadToken) that can be used to cancel this operation
 */
- (nullable SDWebImageDownloadToken *)downloadImageWithURL:(nullable NSURL *)url
                                                 completed:(nullable SDWebImageDownloaderCompletedBlock)completedBlock;

/**
 * Creates a SDWebImageDownloader async downloader instance with a given URL
 *
 * The delegate will be informed when the image is finish downloaded or an error has happen.
 *
 * @see SDWebImageDownloaderDelegate
 *
 * @param url            The URL to the image to download
 * @param options        The options to be used for this download
 * @param progressBlock  A block called repeatedly while the image is downloading
 *                       @note the progress block is executed on a background queue
 * @param completedBlock A block called once the download is completed.
 *                       If the download succeeded, the image parameter is set, in case of error,
 *                       error parameter is set with the error. The last parameter is always YES
 *                       if SDWebImageDownloaderProgressiveLoad isn't use. With the
 *                       SDWebImageDownloaderProgressiveLoad option, this block is called
 *                       repeatedly with the partial image object and the finished argument set to NO
 *                       before to be called a last time with the full image and finished argument
 *                       set to YES. In case of error, the finished argument is always YES.
 *
 * @return A token (SDWebImageDownloadToken) that can be used to cancel this operation
 */
- (nullable SDWebImageDownloadToken *)downloadImageWithURL:(nullable NSURL *)url
                                                   options:(SDWebImageDownloaderOptions)options
                                                  progress:(nullable SDWebImageDownloaderProgressBlock)progressBlock
                                                 completed:(nullable SDWebImageDownloaderCompletedBlock)completedBlock;

/**
 * Creates a SDWebImageDownloader async downloader instance with a given URL
 *
 * The delegate will be informed when the image is finish downloaded or an error has happen.
 *
 * @see SDWebImageDownloaderDelegate
 *
 * @param url            The URL to the image to download
 * @param options        The options to be used for this download
 * @param context        A context contains different options to perform specify changes or processes, see `SDWebImageContextOption`. This hold the extra objects which `options` enum can not hold.
 * @param progressBlock  A block called repeatedly while the image is downloading
 *                       @note the progress block is executed on a background queue
 * @param completedBlock A block called once the download is completed.
 *
 * @return A token (SDWebImageDownloadToken) that can be used to cancel this operation
 */
- (nullable SDWebImageDownloadToken *)downloadImageWithURL:(nullable NSURL *)url
                                                   options:(SDWebImageDownloaderOptions)options
                                                   context:(nullable SDWebImageContext *)context
                                                  progress:(nullable SDWebImageDownloaderProgressBlock)progressBlock
                                                 completed:(nullable SDWebImageDownloaderCompletedBlock)completedBlock;

/**
 * Cancels all download operations in the queue
 */
- (void)cancelAllDownloads;

/**
 * Invalidates the managed session, optionally canceling pending operations.
 * @note If you use custom downloader instead of the shared downloader, you need call this method when you do not use it to avoid memory leak
 * @param cancelPendingOperations Whether or not to cancel pending operations.
 * @note Calling this method on the shared downloader has no effect.
 */
- (void)invalidateSessionAndCancel:(BOOL)cancelPendingOperations;

@end


/**
 SDWebImageDownloader is the built-in image loader conform to `SDImageLoader`. Which provide the HTTP/HTTPS/FTP download, or local file URL using NSURLSession.
 However, this downloader class itself also support customization for advanced users. You can specify `operationClass` in download config to custom download operation, See `SDWebImageDownloaderOperation`.
 If you want to provide some image loader which beyond network or local file, consider to create your own custom class conform to `SDImageLoader`.
 */
@interface SDWebImageDownloader (SDImageLoader) <SDImageLoader>

@end
