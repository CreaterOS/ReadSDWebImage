//
//  ViewController.m
//  ReadSDWebImage
//
//  Created by Bryant Reyn on 2020/8/16.
//  Copyright © 2020 Bryant Reyn. All rights reserved.
//

#import "ViewController.h"
#import <SDWebImage/SDWebImage.h>

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.

    /* 加载动图 */
    SDAnimatedImageView *animatedImageView = [[SDAnimatedImageView alloc] initWithFrame:CGRectMake(50, 50, 200, 150)];
    SDAnimatedImage *image = [SDAnimatedImage imageNamed:@"timg.gif"];
    animatedImageView.image = image;
    [self.view addSubview:animatedImageView];
    
    /* 获取网络图片 */
    UIImageView *imageView = [[UIImageView alloc] initWithFrame:CGRectMake(50, 300, 200, 150)];
    [imageView sd_setImageWithURL:[NSURL URLWithString:@"https://timgsa.baidu.com/timg?image&quality=80&size=b9999_10000&sec=1597552851704&di=0ce8df805134d30aabb68c8b3b33e421&imgtype=0&src=http%3A%2F%2Fa1.att.hudong.com%2F62%2F02%2F01300542526392139955025309984.jpg"]];
    [self.view addSubview:imageView];
    
    NSLog(@"UIScreen.mainScreen.traitCollection:%@",UIScreen.mainScreen.traitCollection);
    NSLog(@"UIScreen.mainScreen.scale:%f",UIScreen.mainScreen.scale);
}


@end
