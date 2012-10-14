//
//  GLView.mm
//  Shadows
//
//  Created by Ilya Kryukov on 28.09.12.
//  Copyright (c) 2012 Ilya Kryukov. All rights reserved.
//

#import <OpenGLES/EAGLDrawable.h>
#import "GLView.h"
#import "mach/mach_time.h"
#import <OpenGLES/ES2/gl.h> // <-- for GL_RENDERBUFFER only
#import <string>

@implementation GLView
const bool ForceES1 = false;

+ (Class) layerClass
{
	return [CAEAGLLayer class];
}

std::string getPath()
{
	NSString* bundlePath =[[NSBundle mainBundle] resourcePath];
	return [bundlePath UTF8String];
}
- (id)initWithFrame:(CGRect)frame
{
	if (self = [super initWithFrame:frame]) {
		CAEAGLLayer* eaglLayer = (CAEAGLLayer*) super.layer;
		eaglLayer.opaque = YES;
		eaglLayer.contentsScale = [[UIScreen mainScreen] scale];
		
		EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
		m_context = [[EAGLContext alloc] initWithAPI:api];
		if (!m_context || ForceES1) {
			api = kEAGLRenderingAPIOpenGLES1;
			m_context = [[EAGLContext alloc] initWithAPI:api]; }
		if (!m_context || ![EAGLContext setCurrentContext:m_context]) { 
			return nil; }
		if (api == kEAGLRenderingAPIOpenGLES1) {
			NSLog(@"Using OpenGL ES 1.1");
			NSLog(@"unsupported device");
		} else {
			NSLog(@"Using OpenGL ES 2.0");
			m_renderingEngine = CreateRenderer2();
		}
		
		[m_context renderbufferStorage:GL_RENDERBUFFER fromDrawable: eaglLayer];
		self.contentScaleFactor = [[UIScreen mainScreen] scale];
		NSLog(@"Width = %d, Height = %d", (int) (CGRectGetWidth(frame) * self.contentScaleFactor), (int) (CGRectGetHeight(frame) * self.contentScaleFactor));
		std::string resourcePath = getPath();
		m_renderingEngine->SetResourcePath(resourcePath);
		m_renderingEngine->SetPivotPoint(CGRectGetWidth(frame) / 2.0, CGRectGetHeight(frame) / 2.0);
		m_renderingEngine->Initialize(CGRectGetWidth(frame) * self.contentScaleFactor, CGRectGetHeight(frame) * self.contentScaleFactor);
		[self drawView: nil];
		m_timestamp = CACurrentMediaTime();
		CADisplayLink* displayLink;
		displayLink = [CADisplayLink displayLinkWithTarget:self
												  selector:@selector(drawView:)];
		[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		[[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
		[[NSNotificationCenter defaultCenter] addObserver:self
												 selector:@selector(didRotate:) name:UIDeviceOrientationDidChangeNotification object:nil];
	}
	return self;
}

- (void) didRotate: (NSNotification*) notification
{
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	m_renderingEngine->OnRotate((DeviceOrientation) orientation);
	[self drawView: nil];
}

- (void) drawView: (CADisplayLink*) displayLink
{
	if (displayLink != nil) {
		float elapsedSeconds = displayLink.timestamp - m_timestamp;
		m_timestamp = displayLink.timestamp;
		m_renderingEngine->UpdateAnimation(elapsedSeconds);
	}
	m_renderingEngine->Render();
	[m_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void) touchesBegan: (NSSet*) touches withEvent: (UIEvent*) event
{
    UITouch* touch = [touches anyObject];
    CGPoint location  = [touch locationInView: self];
    m_renderingEngine->OnFingerDown(vec2(location.x, location.y));
}

- (void) touchesEnded: (NSSet*) touches withEvent: (UIEvent*) event
{
    UITouch* touch = [touches anyObject];
    CGPoint location  = [touch locationInView: self];
    m_renderingEngine->OnFingerUp(vec2(location.x, location.y));
}

- (void) touchesMoved: (NSSet*) touches withEvent: (UIEvent*) event
{
    UITouch* touch = [touches anyObject];
    CGPoint previous  = [touch previousLocationInView: self];
    CGPoint current = [touch locationInView: self];
    m_renderingEngine->OnFingerMove(vec2(previous.x, previous.y),
                                    vec2(current.x, current.y));
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/

@end
