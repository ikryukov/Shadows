//
//  AppDelegate.h
//  Shadows
//
//  Created by Ilya Kryukov on 28.09.12.
//  Copyright (c) 2012 Ilya Kryukov. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GLView.h"

@interface AppDelegate : NSObject <UIApplicationDelegate> {
	UIWindow* m_window;
	GLView* m_view;
}

@property (nonatomic, retain) IBOutlet UIWindow *m_window;

@end
