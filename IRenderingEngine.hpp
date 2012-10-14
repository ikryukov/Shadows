//
//  IRenderingEngine.h
//  Shadows
//
//  Created by Ilya Kryukov on 28.09.12.
//  Copyright (c) 2012 Ilya Kryukov. All rights reserved.
//

#ifndef Shadows_IRenderingEngine_hpp
#define Shadows_IRenderingEngine_hpp
#include "Vector.hpp"
#include <string>


// Physical orientation of a handheld device, equivalent to UIDeviceOrientation.
enum DeviceOrientation {
	DeviceOrientationUnknown,
	DeviceOrientationPortrait,
	DeviceOrientationPortraitUpsideDown,
	DeviceOrientationLandscapeLeft,
	DeviceOrientationLandscapeRight,
	DeviceOrientationFaceUp,
	DeviceOrientationFaceDown,
};
// Interface to the OpenGL ES renderer; consumed by GLView.
struct IRenderingEngine {
    virtual void Initialize(int width, int height) = 0;
	virtual void SetResourcePath(std::string& path) = 0;
	virtual void SetPivotPoint(float x, float y) = 0;
    virtual void Render() const = 0;
    virtual void UpdateAnimation(float timeStep) = 0;
    virtual void OnRotate(DeviceOrientation newOrientation) = 0;
    virtual void OnFingerUp(vec2 location) = 0;
    virtual void OnFingerDown(vec2 location) = 0;
    virtual void OnFingerMove(vec2 oldLocation, vec2 newLocation) = 0;
    virtual ~IRenderingEngine() {}
};
// Creates an instance of the renderer and sets up various OpenGL state.
struct std::auto_ptr<IRenderingEngine> CreateRenderer2();

#endif
