//
//  ViewController.m
//  SpireiOS
//
//  Created by James Hughes on 12/5/12.
//  Copyright (c) 2012 SCI Institute. All rights reserved.
//

#import "ViewController.h"

#include "Spire/Interface.h"

class SpireContextiOS : public Spire::Context
{
public:
  SpireContextiOS(EAGLContext* context) :
    mContext(context)
  { }
  virtual ~SpireContextiOS() {}

  /// Mandatory overrides
  /// @{
  virtual void makeCurrent()
  {
    [EAGLContext setCurrentContext:mContext];
  }

  virtual void swapBuffers()
  {
    // Not implemented on iOS platforms using GLKit
    // This is automatically performed after draw rect.
  }
  /// @}

protected:

  EAGLContext* mContext;
};

@interface ViewController ()
{
  std::shared_ptr<SpireContextiOS>    mContext;
  Spire::Interface*   mInterface;
}
@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;
@end

@implementation ViewController

- (void)viewDidLoad
{
  [super viewDidLoad];

  self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

  if (!self.context)
  {
    NSLog(@"Failed to create ES context");
  }

  GLKView *view = (GLKView *)self.view;
  view.context = self.context;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat24;

  // Obtain path to shader resources.
  NSString* shaderPath = @"Shaders/UniformColor.fsh";
  NSString* fullPath = [[NSBundle mainBundle] pathForResource:[shaderPath lastPathComponent] ofType:nil inDirectory:[shaderPath stringByDeletingLastPathComponent]];
  NSString* shaderDir = [fullPath stringByDeletingLastPathComponent];
  //std::string shaderDirSTD([shaderDir UTF8String]);
  std::string shaderDirSTD([shaderDir cStringUsingEncoding:NSASCIIStringEncoding]);
    
  mContext = std::shared_ptr<SpireContextiOS>(new SpireContextiOS(self.context));
  try {
    mInterface = new Spire::Interface(mContext, { shaderDirSTD }, false);
  } catch (std::exception& e) {
    NSLog([NSString stringWithUTF8String:e.what()]);
    throw;
  }

  [self setupGL];
}

- (void)dealloc
{
  [self tearDownGL];

  // Remove spire and our thin context.
  delete mInterface;
  mContext.reset();

  if ([EAGLContext currentContext] == self.context)
  {
    [EAGLContext setCurrentContext:nil];
  }
}

- (void)didReceiveMemoryWarning
{
  [super didReceiveMemoryWarning];

  if ([self isViewLoaded] && ([[self view] window] == nil))
  {
    self.view = nil;

    [self tearDownGL];

    if ([EAGLContext currentContext] == self.context)
    {
      [EAGLContext setCurrentContext:nil];
    }
    self.context = nil;
  }

  // Dispose of any resources that can be recreated.
}

- (void)setupGL
{
  [EAGLContext setCurrentContext:self.context];
}

- (void)tearDownGL
{
  [EAGLContext setCurrentContext:self.context];
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
  // This is the 'frame update' operation. Ops such as animation and whatnot.
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
  // Make a call to spire render.
  mInterface->ntsDoFrame();
}

@end
