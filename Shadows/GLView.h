//
//  GLView.h
//  Shadows
//
//  Created by Ilya Kryukov on 28.09.12.
//  Copyright (c) 2012 Ilya Kryukov. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "IRenderingEngine.hpp"
#import <OpenGLES/EAGL.h>
#import <QuartzCore/QuartzCore.h>

@interface GLView : UIView {
@private
	EAGLContext* m_context;
	std::auto_ptr<IRenderingEngine> m_renderingEngine;
	float m_timestamp;
}
- (void) drawView: (CADisplayLink*) displayLink;
- (void) didRotate: (NSNotification*) notification;

@end
