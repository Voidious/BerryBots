/*
  Copyright (C) 2012-2013 - Voidious

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

@interface OsxCfg : NSObject {
  NSString *stagesDir;
  NSString *shipsDir;
  NSString *runnersDir;
  NSString *cacheDir;
  NSString *tmpDir;
  NSString *replaysDir;
  NSString *apidocPath;
  NSString *samplesVersion;
  bool overwrite;
  bool asked;
  bool aaDisabled;
}

- (Boolean) hasPlist;
- (bool) loadPlist;
- (void) setRootDir:(NSString *) newRootDir;
- (bool) chooseNewRootDir;
- (void) save;
@property (copy, nonatomic) NSString *stagesDir;
@property (copy, nonatomic) NSString *shipsDir;
@property (copy, nonatomic) NSString *runnersDir;
@property (copy, nonatomic) NSString *cacheDir;
@property (copy, nonatomic) NSString *tmpDir;
@property (copy, nonatomic) NSString *replaysDir;
@property (copy, nonatomic) NSString *apidocPath;
@property (copy, nonatomic) NSString *samplesVersion;
@property (nonatomic) bool aaDisabled;

@end
