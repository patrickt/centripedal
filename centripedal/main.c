// -*- mode: c; c-basic-offset: 4; compilation-scroll-output: t; compile-command: "xcodebuild"-*-
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

static int debug, dry_run;

// TODO: we could just use CGEventFlags here.
// also TODO: there should be a CFRunLoopTimer here
struct kb_state {
    _Bool pedal_ctrl_down : 1;
    _Bool pedal_alt_down : 1;
    _Bool pedal_cmd_down : 1;
} __attribute__((packed));

#define TOGGLE(state, it) if (it && !state) state = true

CGEventRef pedalCallback(CGEventTapProxy proxy __attribute__((unused)), CGEventType type, CGEventRef event, void *info) {
    struct kb_state *state = (struct kb_state *)info;
    CGEventFlags flags = CGEventGetFlags(event);
    if (type == kCGEventFlagsChanged) {
        if (debug) printf("flags: %d %d %d\n", state->pedal_ctrl_down, state->pedal_alt_down, state->pedal_cmd_down);
        TOGGLE(state->pedal_ctrl_down, flags & kCGEventFlagMaskControl);
        TOGGLE(state->pedal_alt_down, flags & kCGEventFlagMaskAlternate);
        TOGGLE(state->pedal_cmd_down, flags & kCGEventFlagMaskCommand);
        if (debug) printf("after: %d, %d, %d\n", state->pedal_ctrl_down, state->pedal_alt_down, state->pedal_cmd_down);
    } else if (type == kCGEventKeyDown) {
        CFTimeInterval since = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateCombinedSessionState, kCGEventFlagsChanged);
        if (since > 5.0) {
            memset(state, 0, sizeof(struct kb_state));
        }

        CGEventFlags newflags = flags;
        if (state->pedal_ctrl_down)
            newflags |= kCGEventFlagMaskControl;

        if (state->pedal_alt_down)
            newflags |= kCGEventFlagMaskAlternate;

        if (state->pedal_cmd_down)
            newflags |= kCGEventFlagMaskCommand;

        if (flags != newflags) {
            if (debug) printf("Modifying flags!!\n");
            if (dry_run) CGEventSetFlags(event, newflags);
        }

    }
    fflush(stdout);

    return event;
}

struct option options[] = {
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
};

int main(int argc, char * const * argv) {
    while (getopt_long(argc, argv, "", options, NULL) != -1);
    if (debug) printf("Booting up…\n");
    struct kb_state state = {0, 0, 0};
    CGEventMask mask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
    CFMachPortRef tap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, mask, pedalCallback, &state);
    if (tap == NULL) {
      fprintf(stderr, "couldn't create tap\n");
        exit(EXIT_FAILURE);
    } else if (debug) {
        printf("tap created\n");
    }

    CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(NULL, tap, 0);
    if (source == NULL) {
        printf("Couldn't create runloop source");
        exit(EXIT_FAILURE);
    }
    CFRunLoopRef runloop = CFRunLoopGetCurrent();
    if (runloop == NULL) {
        printf("Couldn't create runloop");
        exit(EXIT_FAILURE);
    }

    CFRunLoopAddSource(runloop, source, kCFRunLoopCommonModes);

    CFRunLoopRun();
    CFRelease(source);

    return 0;
}
