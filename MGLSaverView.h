#import <AppKit/AppKit.h>
#import <ScreenSaver/ScreenSaver.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#include "matrix.h"

@interface MGLSaverView : ScreenSaverView
{
	IBOutlet id IBconfigureSheet_;
	
	IBOutlet id IBversion_;
	
	IBOutlet id IBshowImages_;
	
	IBOutlet id IBmainScreen_;
    
	IBOutlet id IBcolor_;
	
	// Data
	  
	NSOpenGLView *_view;
    
    NSInteger mainScreenOnly_;
    
    BOOL isConfiguring_;
	
    BOOL preview_;
	
    BOOL mainScreen_;
    
    
    
    MGLSettings * settings_;
}

- (void) doScroll:(id) inObject;

- (IBAction) closeSheet:(id) sender;

- (void) readDefaults:(ScreenSaverDefaults *) inDefaults;
- (void) writeDefaults;

@end
