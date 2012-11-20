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

#import "osxcfg.h"
#import "bbconst.h"

@implementation OsxCfg

@synthesize stageDir;
@synthesize botsDir;
@synthesize cacheDir;
@synthesize tmpDir;

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
  if (self) {
    NSString *errorDesc = nil;
    NSPropertyListFormat format;
    NSString *plistPath;
    NSString *rootPath = [[NSSearchPathForDirectoriesInDomains(
        NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0]
            stringByAppendingPathComponent:@"BerryBots"];
    plistPath = [rootPath stringByAppendingPathComponent:@"config.plist"];
    if (![[NSFileManager defaultManager] fileExistsAtPath:plistPath]) {
      NSOpenPanel* openDlg = [NSOpenPanel openPanel];
      [openDlg setCanChooseFiles:NO];
      [openDlg setCanChooseDirectories:YES];
      [openDlg setCanCreateDirectories:YES];
      [openDlg setAllowsMultipleSelection:NO];
      [openDlg setTitle:
          @"Select a BerryBots root directory, for bots, stages, and cache"];
      if ( [openDlg runModal] == NSOKButton ) {
        NSArray *files = [openDlg URLs];
        for( int i = 0; i < [files count]; i++ ) {
          NSLog(@"File path: %@", [[files objectAtIndex:i] path]);
        }
        self.stageDir = [NSString stringWithFormat:@"%@/%@",
                         [[files objectAtIndex:0] path], @"stages"];
        self.botsDir = [NSString stringWithFormat:@"%@/%@",
                        [[files objectAtIndex:0] path], @"bots"];
        self.cacheDir = [NSString stringWithFormat:@"%@/%@",
                         [[files objectAtIndex:0] path], @CACHE_SUBDIR];
        self.tmpDir = [NSString stringWithFormat:@"%@/%@",
                       [[files objectAtIndex:0] path], @TMP_SUBDIR];
        [self save];
        // TODO: copy sample bots and stages
      } else {
        exit(1);
      }
    } else {
      NSData *plistXML =
          [[NSFileManager defaultManager] contentsAtPath:plistPath];
      NSDictionary *temp =
          (NSDictionary *)[NSPropertyListSerialization
                           propertyListFromData:plistXML
                           mutabilityOption:
                               NSPropertyListMutableContainersAndLeaves
                           format:&format
                           errorDescription:&errorDesc];
      if (!temp) {
        NSLog(@"Error reading plist: %@, format: %d", errorDesc, format);
      }
      self.stageDir = [temp objectForKey:@"Stage dir"];
      self.botsDir = [temp objectForKey:@"Bots dir"];
      self.cacheDir = [temp objectForKey:@"Cache dir"];
      self.tmpDir = [temp objectForKey:@"Tmp dir"];
    }
  }
  return self;
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
       [NSArray arrayWithObjects: stageDir, botsDir, cacheDir, tmpDir, nil]
        forKeys:[NSArray arrayWithObjects:
                 @"Stage dir", @"Bots dir", @"Cache dir", @"Tmp dir", nil]];
  NSData *plistData =
      [NSPropertyListSerialization dataFromPropertyList:plistDict
          format:NSPropertyListXMLFormat_v1_0
          errorDescription:&error];
  if(plistData) {
    [plistData writeToFile:plistPath atomically:YES];
  }
  else {
    NSLog(error);
    [error release];
  }
}

@end
