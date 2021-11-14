// -*- mode: c; c-basic-offset: 4; -*-
//
//  main.c
//  centripedal
//
//  Created by Patrick Thomson on 11/12/21.
//

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

// TODO: we could just use CGEventFlags here.
// also TODO: there should be a CFRunLoopTimer here
struct kb_state {
    _Bool pedal_ctrl_down : 1;
    _Bool pedal_alt_down : 1;
    _Bool pedal_cmd_down : 1;
} __attribute__((packed));

CGEventRef pedalCallback(CGEventTapProxy proxy __attribute__((unused)), CGEventType type, CGEventRef event, void *info) {
    struct kb_state *state = (struct kb_state *)info;
    CGEventFlags flags = CGEventGetFlags(event);
    if (type == kCGEventFlagsChanged) {
        _Bool ctrlOn = (_Bool)(flags & kCGEventFlagMaskControl);
        _Bool altOn = (_Bool)(flags & kCGEventFlagMaskAlternate);
        _Bool cmdOn = (_Bool)(flags & kCGEventFlagMaskCommand);
        printf("flags: %d %d %d\n", ctrlOn, altOn, cmdOn);
        state->pedal_ctrl_down = !state->pedal_ctrl_down && ctrlOn;
        state->pedal_alt_down = !state->pedal_alt_down && altOn;
        state->pedal_cmd_down = !state->pedal_cmd_down && cmdOn;
        printf("after: %d, %d, %d\n", state->pedal_ctrl_down, state->pedal_alt_down, state->pedal_cmd_down);
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
            printf("Modifying flags!!\n");
            CGEventSetFlags(event, newflags);
        }

    }
    fflush(stdout);

    return event;
}

int main() {
    // insert code here...
    printf("Booting up…\n");
    struct kb_state state = {0, 0, 0};
    CGEventMask mask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
    CFMachPortRef tap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, mask, pedalCallback, &state);
    if (tap == NULL) {
      fprintf(stderr, "couldn't create tap\n");
        exit(EXIT_FAILURE);
    } else {
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
