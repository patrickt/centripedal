// -*- mode: c; c-basic-offset: 4; compile-command: "xcodebuild"-*-
//
//  main.c
//  centripedal
//
//  Created by Patrick Thomson on 11/12/21.
//

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <getopt.h>

static int debug, dry_run, help;

const struct option options[] = {
    {
        .name = "debug",
        .has_arg = no_argument,
        .flag = &debug,
        .val = 1,
    },
    {
        .name = "dry-run",
        .has_arg = no_argument,
        .flag = &dry_run,
        .val = 1,
    },
    {
        .name = "help",
        .has_arg = no_argument,
        .flag = &help,
        .val = 1,
    }
};

void describe(char *msg, CGEventFlags state) {
    fprintf(stderr, "%s: %d, %d, %d\n", msg,
            !!(state & kCGEventFlagMaskControl),
            !!(state & kCGEventFlagMaskAlternate),
            !!(state & kCGEventFlagMaskCommand));
}

CGEventRef pedalCallback(CGEventTapProxy proxy __attribute__((unused)), CGEventType type, CGEventRef event, void *info) {
    CGEventFlags *state = (CGEventFlags *)info;
    const CGEventFlags flags = CGEventGetFlags(event);
    if (type == kCGEventFlagsChanged) {
        if (debug) describe("before", *state);
        *state = flags & (kCGEventFlagMaskControl | kCGEventFlagMaskAlternate | kCGEventFlagMaskCommand);
        if (debug) describe("after", *state);

    } else if (type == kCGEventKeyDown) {
        const CFTimeInterval cooldown = 10.0;
        CFTimeInterval since = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateCombinedSessionState,
                                                                      kCGEventFlagsChanged);
        if (since > cooldown) {
            if (debug) fputs("Resetting flags", stderr);
            *state = 0;
        } else if (*state) {
            if (debug) fputs("Modifying flags!", stderr);
            if (!dry_run) CGEventSetFlags(event, flags | *state);
        }

    }
    return event;
}

int main(int argc, char * const * argv) {
    while (getopt_long(argc, argv, "", options, NULL) != -1);
    if (help) {
        puts("Usage: centripedal [--debug] [--dry-run]");
        return 0;
    }

    if (debug) fputs("Booting up…", stderr);

    CGEventFlags pressedModifierKeys = 0;
    CFMachPortRef tap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap,
                                         kCGEventTapOptionDefault,
                                         CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged),
                                         pedalCallback, &pressedModifierKeys);
    if (tap == NULL) {
        fputs("couldn't create tap, may need accessibility privileges?", stderr);
        exit(EXIT_FAILURE);
    } else if (debug) {
        fputs("tap created", stderr);
    }

    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(NULL, tap, 0);
    if (source == NULL) {
        fputs("Couldn't create runloop source", stderr);
        exit(EXIT_FAILURE);
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
    CFRunLoopRun();
    CFRelease(source);
    return 0;
}
