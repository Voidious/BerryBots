/*
  Copyright (C) 2012 - Voidious

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

#import "bbconst.h"
#import "osxcfg.h"

@implementation OsxCfg

@synthesize stagesDir;
@synthesize shipsDir;
@synthesize cacheDir;
@synthesize tmpDir;
@synthesize apidocPath;

bool fileExists(const char *filename) {
  FILE *testFile = fopen(filename, "r");
  bool exists = (testFile != 0);
  if (exists) {
    fclose(testFile);
  }
  return exists;
}

- (id) init {
  self = [super init];
  return self;
}

- (Boolean) hasPlist {
  NSString *plistPath;
  NSString *rootPath = [[NSSearchPathForDirectoriesInDomains(
      NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0]
          stringByAppendingPathComponent:@"BerryBots"];
  plistPath = [rootPath stringByAppendingPathComponent:@"config.plist"];
  if ([[NSFileManager defaultManager] fileExistsAtPath:plistPath]) {
    return true;
  }
  return false;
}

- (bool) loadPlist {
  if (self) {
    NSString *errorDesc = nil;
    NSPropertyListFormat format;
    NSString *plistPath;
    NSString *rootPath = [[NSSearchPathForDirectoriesInDomains(
        NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0]
            stringByAppendingPathComponent:@"BerryBots"];
    plistPath = [rootPath stringByAppendingPathComponent:@"config.plist"];
    bool selectRoot = true;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if ([fileManager fileExistsAtPath:plistPath]) {
      NSData *plistXML = [fileManager contentsAtPath:plistPath];
      NSDictionary *temp =
          (NSDictionary *)[NSPropertyListSerialization
                           propertyListFromData:plistXML
                           mutabilityOption:
                           NSPropertyListMutableContainersAndLeaves
                           format:&format
                           errorDescription:&errorDesc];
      if (!temp) {
        NSLog(@"Error reading plist: %@, format: %ld", errorDesc, format);
      }
      self.stagesDir = [temp objectForKey:@"Stage dir"];
      self.shipsDir = [temp objectForKey:@"Ships dir"];
      self.cacheDir = [temp objectForKey:@"Cache dir"];
      self.tmpDir = [temp objectForKey:@"Tmp dir"];
      self.apidocPath = [temp objectForKey:@"Apidoc Path"];
      if ([fileManager fileExistsAtPath:self.stagesDir]
          && [fileManager fileExistsAtPath:self.shipsDir]) {
        selectRoot = false;
      }
    }
    if (selectRoot) {
      return [self chooseNewRootDir];
    } else {
      return true;
    }
  }
  return false;
}

- (void) setRootDir:(NSString *) newRootDir {
  self.stagesDir = [NSString stringWithFormat:@"%@/%@",
                    newRootDir, @"stages"];
  self.shipsDir = [NSString stringWithFormat:@"%@/%@",
                  newRootDir, @"bots"];
  self.cacheDir = [NSString stringWithFormat:@"%@/%@",
                   newRootDir, @CACHE_SUBDIR];
  self.tmpDir = [NSString stringWithFormat:@"%@/%@",
                 newRootDir, @TMP_SUBDIR];
  self.apidocPath = [NSString stringWithFormat:@"%@/%@",
                     newRootDir, @"apidoc/index.html"];
  [self save];

  NSString *srcPath = [[NSBundle mainBundle] resourcePath];
  NSFileManager *fileManager = [NSFileManager defaultManager];
  if ([fileManager isReadableFileAtPath:srcPath]) {
    NSError *dError;
    if (![fileManager isReadableFileAtPath:self.stagesDir]) {
      NSString *stagesSrc =
          [NSString stringWithFormat:@"%@/stages", srcPath];
      int success = [fileManager
                     copyItemAtPath:stagesSrc toPath:self.stagesDir
                     error:&dError];
      if (success != YES) {
        NSLog(@"Error: %@", dError);
      }
    }

    if (![fileManager isReadableFileAtPath:self.shipsDir]) {
      NSString *shipsSrc = [NSString stringWithFormat:@"%@/bots", srcPath];
      int success = [fileManager
                 copyItemAtPath:shipsSrc toPath:self.shipsDir error:&dError];
      if (success != YES) {
        NSLog(@"Error: %@", dError);
      }
    }

    NSString *apidocDir =
        [NSString stringWithFormat:@"%@/%@", newRootDir, @"apidoc"];
    if (![fileManager isReadableFileAtPath:apidocDir]) {
      NSString *apidocSrc = [NSString stringWithFormat:@"%@/apidoc", srcPath];
      int success =
          [fileManager copyItemAtPath:apidocSrc toPath:apidocDir error:&dError];
      if (success != YES) {
        NSLog(@"Error: %@", dError);
      }
    }
  }
}

- (bool) chooseNewRootDir {
  NSOpenPanel* openDlg = [NSOpenPanel openPanel];
  [openDlg setCanChooseFiles:NO];
  [openDlg setCanChooseDirectories:YES];
  [openDlg setCanCreateDirectories:YES];
  [openDlg setAllowsMultipleSelection:NO];
  [openDlg setTitle:@"Select a BerryBots base directory"];
  if ([openDlg runModal] == NSOKButton) {
    NSArray *files = [openDlg URLs];
    for( int i = 0; i < [files count]; i++ ) {
      NSLog(@"File path: %@", [[files objectAtIndex:i] path]);
    }
    [self setRootDir:[[files objectAtIndex:0] path]];
    return true;
  } else {
    return false;
  }
}

- (void) save {
  NSString *error;
  NSString *rootPath = [[NSSearchPathForDirectoriesInDomains(
      NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0]
          stringByAppendingPathComponent:@"BerryBots"];
  if (!fileExists(rootPath.UTF8String)) {
    NSString *mkdir = @"mkdir \"";
    NSString *mkdirCmd = [[mkdir stringByAppendingPathComponent:rootPath]
                          stringByAppendingPathComponent:@"\""];
    system(mkdirCmd.UTF8String);
  }
  NSString *plistPath =
      [rootPath stringByAppendingPathComponent:@"config.plist"];
  NSDictionary *plistDict =
      [NSDictionary dictionaryWithObjects:
       [NSArray arrayWithObjects:stagesDir, shipsDir, cacheDir, tmpDir,
                                 apidocPath, nil]
        forKeys:[NSArray arrayWithObjects:
                 @"Stage dir", @"Ships dir", @"Cache dir", @"Tmp dir",
                 @"Apidoc Path", nil]];
  NSData *plistData =
      [NSPropertyListSerialization dataFromPropertyList:plistDict
          format:NSPropertyListXMLFormat_v1_0
          errorDescription:&error];
  if(plistData) {
    [plistData writeToFile:plistPath atomically:YES];
  } else {
    NSLog(@"Error saving plist: %@", error);
    [error release];
  }
}

@end
