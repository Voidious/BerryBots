/*
  Copyright (C) 2012-2015 - Voidious

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

@interface OsxCfg()
- (void) copyAllAppFiles:(NSString *) srcDir forceAll:(bool) forceAll;
- (void) copyAppFiles:(NSString *) srcDir toDir:(NSString *) targetDir
         forceOverwrite:(bool) force;
@end

@implementation OsxCfg

@synthesize stagesDir;
@synthesize shipsDir;
@synthesize runnersDir;
@synthesize cacheDir;
@synthesize tmpDir;
@synthesize replaysDir;
@synthesize apidocPath;
@synthesize samplesVersion;
@synthesize aaDisabled;

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

- (void) copyAllAppFiles:(NSString *) srcDir forceAll:(bool) forceAll {
  overwrite = asked = false;
  NSString *stagesSrc = [NSString stringWithFormat:@"%@/stages", srcDir];
  [self copyAppFiles:stagesSrc toDir:self.stagesDir forceOverwrite:forceAll];

  NSString *shipsSrc = [NSString stringWithFormat:@"%@/bots", srcDir];
  [self copyAppFiles:shipsSrc toDir:self.shipsDir forceOverwrite:forceAll];

  NSString *runnersSrc = [NSString stringWithFormat:@"%@/runners", srcDir];
  [self copyAppFiles:runnersSrc toDir:self.runnersDir forceOverwrite:forceAll];

  NSString *apidocSrc = [NSString stringWithFormat:@"%@/apidoc", srcDir];
  NSString *apidocDest = [self.apidocPath stringByDeletingLastPathComponent];
  [self copyAppFiles:apidocSrc toDir:apidocDest forceOverwrite:true];
}

- (void) copyAppFiles:(NSString *) srcDir toDir:(NSString *) targetDir
         forceOverwrite:(bool) force {
  NSFileManager *fileManager = [NSFileManager defaultManager];
  if (![fileManager isReadableFileAtPath:targetDir]) {
    if (![fileManager createDirectoryAtPath:targetDir
              withIntermediateDirectories:YES attributes:nil error:NULL]) {
      NSLog(@"Error creating directory: %@", targetDir);
    }
  }

  NSArray *files = [fileManager subpathsAtPath:srcDir];
  NSError *dError;
  for (NSString *file in files) {
    NSString *srcFile = [NSString stringWithFormat:@"%@/%@", srcDir, file];
    NSString *destFile = [NSString stringWithFormat:@"%@/%@", targetDir, file];

    BOOL isDir;
    if ([fileManager fileExistsAtPath:srcFile isDirectory:&isDir] && !isDir) {
      bool destFileExists = [fileManager isReadableFileAtPath:destFile];
      if (destFileExists && !force && !asked) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"OK"];
        [alert addButtonWithTitle:@"Skip"];
        [alert setMessageText:@"Overwrite samples?"];
        [alert setInformativeText:
             @"Sample ships and stages already exist in this directory. OK to overwrite them?"];
        [alert setAlertStyle:NSInformationalAlertStyle];
        if ([alert runModal] == NSAlertFirstButtonReturn) {
          overwrite = true;
        }
        asked = true;
      }

      bool doCopy = (force || overwrite || !destFileExists);
      if (destFileExists && doCopy) {
        NSLog(@"Deleting %@", destFile);
        int success = [fileManager removeItemAtPath:destFile error:&dError];
        if (success != YES) {
          NSLog(@"Error: %@", dError);
        }
      }

      if (doCopy) {
        NSLog(@"Copying %@ to %@", srcFile, destFile);
        NSString *parentDir = [destFile stringByDeletingLastPathComponent];
        if (![fileManager isReadableFileAtPath:parentDir]) {
          if (![fileManager createDirectoryAtPath:parentDir
                  withIntermediateDirectories:YES attributes:nil error:NULL]) {
            NSLog(@"Error creating directory: %@", parentDir);
          }
        }

        int success =
            [fileManager copyItemAtPath:srcFile toPath:destFile error:&dError];
        if (success != YES) {
          NSLog(@"Error: %@", dError);
        }
      }
    }
  }
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
    NSString *plistPath;
    NSString *rootPath = [[NSSearchPathForDirectoriesInDomains(
        NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0]
            stringByAppendingPathComponent:@"BerryBots"];
    plistPath = [rootPath stringByAppendingPathComponent:@"config.plist"];
    bool selectRoot = true;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    if ([fileManager fileExistsAtPath:plistPath]) {
      NSData *plistXML = [fileManager contentsAtPath:plistPath];
      NSError *plistError;
      NSPropertyListFormat plistFormat;
      NSDictionary *temp =
          (NSDictionary *)[NSPropertyListSerialization
                           propertyListWithData:plistXML
                           options:NSPropertyListImmutable
                           format:&plistFormat
                           error:&plistError];
      if (!temp) {
        NSLog(@"Error reading plist: %@, format: %ld", plistError, plistFormat);
      }
      self.stagesDir = [temp objectForKey:@"Stage dir"];
      self.shipsDir = [temp objectForKey:@"Ships dir"];
      self.runnersDir = [temp objectForKey:@"Runners dir"];
      self.cacheDir = [temp objectForKey:@"Cache dir"];
      self.tmpDir = [temp objectForKey:@"Tmp dir"];
      self.replaysDir = [temp objectForKey:@"Replays dir"];
      self.apidocPath = [temp objectForKey:@"Apidoc Path"];
      self.samplesVersion = [temp objectForKey:@"Samples Version"];
      if (self.samplesVersion == nil) {
        self.samplesVersion = @"1.1.0";
      }
      self.aaDisabled =
          [[temp objectForKey:@"Disable Anti-aliasing"] boolValue];

      if ([fileManager fileExistsAtPath:self.stagesDir]
          && [fileManager fileExistsAtPath:self.shipsDir]) {
        selectRoot = false;
        bool needToSave = false;
        if (self.runnersDir == nil) {
          self.runnersDir = [NSString stringWithFormat:@"%@/runners",
                             [self.shipsDir stringByDeletingLastPathComponent]];
          needToSave = true;
        }
        if (self.replaysDir == nil) {
          self.replaysDir = [NSString stringWithFormat:@"%@/replays",
                             [self.shipsDir stringByDeletingLastPathComponent]];
          needToSave = true;
        }
        if (needToSave) {
          [self save];
        }
      }
    }
    bool loaded;
    if (selectRoot) {
      loaded = [self chooseNewRootDir];
    } else {
      loaded = true;
    }

    if (loaded && ![self.samplesVersion isEqualToString:@SAMPLES_VERSION]) {
      NSString *srcDir = [[NSBundle mainBundle] resourcePath];
      if ([fileManager isReadableFileAtPath:srcDir]) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"OK"];
        [alert addButtonWithTitle:@"Skip"];
        [alert setMessageText:@"Update samples and docs?"];
        [alert setInformativeText:[NSString stringWithFormat:
            @"BerryBots samples and API docs have been updated to version %@. OK to update the files in your base directory?", @SAMPLES_VERSION ]];
        [alert setAlertStyle:NSInformationalAlertStyle];
        if ([alert runModal] == NSAlertFirstButtonReturn) {
          [self copyAllAppFiles:srcDir forceAll:true];
          
          NSAlert *alert2 = [[NSAlert alloc] init];
          [alert2 addButtonWithTitle:@"OK"];
          [alert2 setMessageText:@"Samples and docs updated"];
          [alert2 setInformativeText:[NSString stringWithFormat:
              @"BerryBots samples and API docs have been updated to v%@.",
              @SAMPLES_VERSION]];
          [alert2 setAlertStyle:NSInformationalAlertStyle];
          [alert2 runModal];
        }

        self.samplesVersion = @SAMPLES_VERSION;
        [self save];
      }
    }

    return loaded;
  }
  return false;
}

- (void) setRootDir:(NSString *) newRootDir {
  self.stagesDir = [NSString stringWithFormat:@"%@/%@",
                    newRootDir, @"stages"];
  self.shipsDir = [NSString stringWithFormat:@"%@/%@",
                  newRootDir, @"bots"];
  self.runnersDir = [NSString stringWithFormat:@"%@/%@",
                     newRootDir, @"runners"];
  self.cacheDir = [NSString stringWithFormat:@"%@/%@",
                   newRootDir, @CACHE_SUBDIR];
  self.tmpDir = [NSString stringWithFormat:@"%@/%@",
                 newRootDir, @TMP_SUBDIR];
  self.replaysDir = [NSString stringWithFormat:@"%@/%@",
                     newRootDir, @REPLAYS_SUBDIR];
  self.apidocPath = [NSString stringWithFormat:@"%@/%@",
                     newRootDir, @"apidoc/index.html"];
  self.samplesVersion = @SAMPLES_VERSION;
  [self save];

  NSString *srcDir = [[NSBundle mainBundle] resourcePath];
  NSFileManager *fileManager = [NSFileManager defaultManager];
  if ([fileManager isReadableFileAtPath:srcDir]) {
    [self copyAllAppFiles:srcDir forceAll:false];
  }
}

- (bool) chooseNewRootDir {
  NSOpenPanel* openDlg = [NSOpenPanel openPanel];
  [openDlg setCanChooseFiles:NO];
  [openDlg setCanChooseDirectories:YES];
  [openDlg setCanCreateDirectories:YES];
  [openDlg setAllowsMultipleSelection:NO];
  [openDlg setTitle:@"Select a BerryBots base directory"];
  if ([openDlg runModal] == NSModalResponseOK) {
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
  NSError *plistError;
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
       [NSArray arrayWithObjects:stagesDir, shipsDir, runnersDir, cacheDir,
        tmpDir, replaysDir, apidocPath, samplesVersion,
        [NSNumber numberWithBool:aaDisabled], nil]
            forKeys:[NSArray arrayWithObjects:
                     @"Stage dir", @"Ships dir", @"Runners dir", @"Cache dir",
                     @"Tmp dir", @"Replays dir", @"Apidoc Path",
                     @"Samples Version", @"Disable Anti-aliasing", nil]];
  NSData *plistData =
      [NSPropertyListSerialization dataWithPropertyList:plistDict
          format:NSPropertyListXMLFormat_v1_0
          options:NSPropertyListImmutable
          error:&plistError];
  if(plistData) {
    [plistData writeToFile:plistPath atomically:YES];
  } else {
    NSLog(@"Error saving plist: %@", plistError);
  }
}

@end
