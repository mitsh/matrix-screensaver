// Mac OS X Code
// 
// First version: 09/04/04 Stéphane Sudre

#import "MGLSaverView.h"
#include <sys/time.h>

@implementation MGLSaverView

- (id) initWithFrame:(NSRect)frameRect isPreview:(BOOL) preview
{
    NSString * tIdentifier;
    ScreenSaverDefaults * tDefaults;
    
    tIdentifier = [[NSBundle bundleForClass:[self class]] bundleIdentifier];
    
    tDefaults = [ScreenSaverDefaults defaultsForModuleWithName:tIdentifier];
    
    self = [super initWithFrame:frameRect isPreview:preview];
        
    preview_=preview;
    
    isConfiguring_=NO;
    
    if (preview_==YES)
    {
        mainScreen_=YES;
    }
    else
    {
        mainScreen_= (frameRect.origin.x==0 && frameRect.origin.y==0) ? YES : NO;
    }
    
    mainScreenOnly_=[tDefaults integerForKey:@"MainScreen Only"];
    
    if (self)
    {
        
        if (mainScreenOnly_!=NSOnState || mainScreen_==YES)
        {
            NSOpenGLPixelFormatAttribute attribs[] = 
            {
                NSOpenGLPFADoubleBuffer,
                NSOpenGLPFAMinimumPolicy,
                (NSOpenGLPixelFormatAttribute)0
            };
        
            NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
            
            if (format!=nil)
            {
                _view = [[NSOpenGLView alloc] initWithFrame:NSZeroRect pixelFormat:format];
                
                [self addSubview:_view];
                
                
                [self setAnimationTimeInterval:1.0f/FRAME_RATE_SAMPLES];
            }
        }
    }
    
    return self;
}

- (void) setFrameSize:(NSSize)newSize
{
    [super setFrameSize:newSize];
    
    if (_view!=nil)
    {
        [_view setFrameSize:newSize];
    }
}

#pragma mark -

- (void) keyDown:(NSEvent *) theEvent
{
    NSString * tString;
    unsigned long stringLength;
    unsigned int i;
    unichar tChar;
    
    tString= [theEvent characters];
    
    stringLength=[tString length];
    
    for(i=0;i<stringLength;i++)
    {
        tChar=[tString characterAtIndex:i];
        
        //RESET_TIMER(settings_);
        
        switch(tChar)
        {
            case 'n': /* n - Next picture. */
                if (settings_->classic) break;
                settings_->pic_offset+=rtext_x*text_y;
                settings_->pic_mode=1;
                settings_->timer=10;
                if(settings_->pic_offset>(rtext_x*text_y*(num_pics))) settings_->pic_offset=0; 
                break;
            
            case 'p':
                
                settings_->paused=!settings_->paused;
                
                break;
            case 's':
            
                if (!settings_->classic)
                {
                    settings_->pic_fade=0;
                    settings_->pic_offset=0;
                }
                
                settings_->pic_mode=!settings_->pic_mode;
                settings_->classic=!settings_->classic;

                break;
            case NSPageUpFunctionKey: // move the scene into the distance.
                Z_Off -= 0.3f;
                break;

            case NSPageDownFunctionKey: // move the scene closer.
                Z_Off += 0.3f;
                break;
            case 'e':
                settings_->exit_mode^=1;
                break;
            default:
                [super keyDown:theEvent];
                return;
        }
    }
}

- (void) drawRect:(NSRect) inFrame
{
    [[NSColor blackColor] set];
            
    NSRectFill(inFrame);
    
    if (_view==nil)
    {    
        if (mainScreenOnly_!=NSOnState || mainScreen_==YES)
        {
            NSRect tFrame=[self frame];
            NSRect tStringFrame;
            NSDictionary * tAttributes;
            NSString * tString;
            NSMutableParagraphStyle * tParagraphStyle;
            
            tParagraphStyle=[[NSParagraphStyle defaultParagraphStyle] mutableCopy];
            [tParagraphStyle setAlignment:NSCenterTextAlignment];
            
            tAttributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont systemFontOfSize:[NSFont systemFontSize]],NSFontAttributeName,[NSColor whiteColor],NSForegroundColorAttributeName,tParagraphStyle,NSParagraphStyleAttributeName,nil];
            
            
            tString=NSLocalizedStringFromTableInBundle(@"Minimum OpenGL requirements\rfor this Screen Effect\rnot available\ron your graphic card.",@"Localizable",[NSBundle bundleForClass:[self class]],@"No comment");
            
            tStringFrame.origin=NSZeroPoint;
            tStringFrame.size=[tString sizeWithAttributes:tAttributes];
            
            tStringFrame=SSCenteredRectInRect(tStringFrame,tFrame);
            
            [tString drawInRect:tStringFrame withAttributes:tAttributes];
            
            return;
        }
    }
}

- (void)animateOneFrame
{
    if (isConfiguring_==NO && _view!=nil)
    {
        if (mainScreenOnly_!=NSOnState || mainScreen_==YES)
        {
            [[_view openGLContext] makeCurrentContext];
            
            if (!cbRenderScene(settings_))
            {
                [self performSelector:@selector(doScroll:) withObject:nil afterDelay:0.060f];
            }
            
            [[_view openGLContext] flushBuffer];
        }
    }
}

- (void)startAnimation
{
    [super startAnimation];
    
    if (isConfiguring_==NO && _view!=nil)
    {
        if (mainScreenOnly_!=NSOnState || mainScreen_==YES)
        {
            NSSize tSize;
            NSString * tIdentifier;
            ScreenSaverDefaults * tDefaults;
            id tObject;
            
            tIdentifier = [[NSBundle bundleForClass:[self class]] bundleIdentifier];
            
            tDefaults = [ScreenSaverDefaults defaultsForModuleWithName:tIdentifier];
            
            [self lockFocus];
            [[_view openGLContext] makeCurrentContext];
            
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            [[_view openGLContext] flushBuffer];
            
            tSize=[_view frame].size;
            
            settings_=(MGLSettings *) malloc(sizeof(MGLSettings));
            
            tObject=[tDefaults objectForKey:@"Color"];
    
            if (tObject!=nil)
            {
                settings_->color=[tDefaults integerForKey:@"Color"];
            }
            else
            {
                settings_->color=GL_GREEN;
            }
            
            ourInit((int) tSize.width,(int) tSize.height,settings_);
        
            
            [self readDefaults:tDefaults];
            
            [self unlockFocus];
        }
    }
}

- (void) doScroll:(id) inObject
{
    if (!scroll(settings_->mode2,settings_))
    {
        SEL tSelector;
        
        tSelector=@selector(doScroll:);
        
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:tSelector object:nil];
        
        [self performSelector:tSelector withObject:nil afterDelay:0.060f];
    }
}

- (void)stopAnimation
{
    [super stopAnimation];
    
    if (_view!=nil)
    {
        if (mainScreenOnly_!=NSOnState || mainScreen_==YES)
        {
            [[_view openGLContext] makeCurrentContext];
            
            [NSObject cancelPreviousPerformRequestsWithTarget:self];
            
            free(settings_);
        }
    }
}

- (BOOL) hasConfigureSheet
{
    return (_view!=nil);
}

- (void) readDefaults:(ScreenSaverDefaults *) inDefaults
{
    id tObject;

    // Text Only
    
    tObject=[inDefaults objectForKey:@"Show Images"];
    
    if (tObject!=nil && [inDefaults boolForKey:@"Show Images"]==NO)
    {
        settings_->pic_fade=0;
        settings_->pic_offset=0;
        
        settings_->pic_mode=0;
        settings_->classic=1;
    }
    
    // Color
    
    tObject=[inDefaults objectForKey:@"Color"];
    
    if (tObject!=nil)
    {
        settings_->color=[inDefaults integerForKey:@"Color"];
    }
}

- (void) writeDefaults
{
    NSString * tIdentifier;
    ScreenSaverDefaults * tDefaults;
    
    tIdentifier = [[NSBundle bundleForClass:[self class]] bundleIdentifier];
    
    tDefaults = [ScreenSaverDefaults defaultsForModuleWithName:tIdentifier];
    
    mainScreenOnly_=[IBmainScreen_ state];
    
    
    [tDefaults setInteger:[IBshowImages_ state] forKey:@"Show Images"];
    
    [tDefaults setInteger:[IBmainScreen_ state] forKey:@"MainScreen Only"];
    
    [tDefaults setInteger:[[IBcolor_ selectedItem] tag] forKey:@"Color"];
    
    [tDefaults  synchronize];
}

- (void) setDialogValue
{
    NSString * tIdentifier;
    ScreenSaverDefaults * tDefaults;
    id tObject;
    
    tIdentifier = [[NSBundle bundleForClass:[self class]] bundleIdentifier];
    
    tDefaults = [ScreenSaverDefaults defaultsForModuleWithName:tIdentifier];
    
    // Text Only
    
    tObject=[tDefaults objectForKey:@"Show Images"];
    
    if (tObject!=nil && [tDefaults boolForKey:@"Show Images"]==NO)
    {
        [IBshowImages_ setState:NSOffState];
    }
    else
    {
        [IBshowImages_ setState:NSOnState];
    }



    [IBmainScreen_ setState:mainScreenOnly_];
    
    // Color
    
    tObject=[tDefaults objectForKey:@"Color"];
    
    if (tObject!=nil)
    {
        [IBcolor_ selectItemWithTag:[tDefaults integerForKey:@"Color"]];
    }
    else
    {
        [IBcolor_ selectItemWithTag:GL_GREEN];
    }
}

- (NSWindow*) configureSheet
{
    isConfiguring_=YES;
    
    if (IBconfigureSheet_ == nil)
    {
        [NSBundle loadNibNamed:@"ConfigureSheet" owner:self];
        
        [IBversion_ setStringValue:[[[NSBundle bundleForClass:[self class]] infoDictionary] objectForKey:@"CFBundleShortVersionString"]];
    }
    
    [self setDialogValue];
    
    return IBconfigureSheet_;
}

- (IBAction)closeSheet:(id)sender
{
    if ([sender tag]==NSOKButton)
    {
        [self writeDefaults];
    }
    
    isConfiguring_=NO;
    
    if ([self isAnimating]==YES)
    {
        [self stopAnimation];
        [self startAnimation];
    }
    
    [NSApp endSheet:IBconfigureSheet_];
}

@end
