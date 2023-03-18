#pragma once

typedef enum {
    DesktopMainEventOpenLockMenu,
    DesktopMainEventOpenArchive,
    DesktopMainEventOpenFavoritePrimary,
    DesktopMainEventOpenFavoriteSecondary,
    DesktopMainEventOpenFavoriteTertiary,
    DesktopMainEventOpenFavoriteQuaternary,
    DesktopMainEventOpenMenu,
    DesktopMainEventOpenGames,
    DesktopMainEventOpenDebug,
    DesktopMainEventOpenPassport,
    DesktopMainEventOpenPowerOff,
    DesktopMainEventLock,

    DesktopMainEventOpenSnake,
    DesktopMainEventOpen2048,
    DesktopMainEventOpenZombiez,
    DesktopMainEventOpenTetris,
    DesktopMainEventOpenDOOM,
    DesktopMainEventOpenDice,
    DesktopMainEventOpenArkanoid,
    DesktopMainEventOpenHeap,
    DesktopMainEventOpenClock,

    DesktopLockedEventUnlocked,
    DesktopLockedEventUpdate,
    DesktopLockedEventShowPinInput,

    DesktopPinInputEventResetWrongPinLabel,
    DesktopPinInputEventUnlocked,
    DesktopPinInputEventUnlockFailed,
    DesktopPinInputEventBack,

    DesktopPinTimeoutExit,

    DesktopDebugEventDeed,
    DesktopDebugEventWrongDeed,
    DesktopDebugEventSaveState,
    DesktopDebugEventExit,

    DesktopLockMenuEventLock,
    DesktopLockMenuEventPinLock,
    DesktopLockMenuEventPinLockShutdown,
    DesktopLockMenuEventExit,
    DesktopLockMenuEventDummyModeOn,
    DesktopLockMenuEventDummyModeOff,

    DesktopAnimationEventCheckAnimation,
    DesktopAnimationEventNewIdleAnimation,
    DesktopAnimationEventInteractAnimation,

    DesktopSlideshowCompleted,
    DesktopSlideshowPoweroff,

    // Global events
    DesktopGlobalBeforeAppStarted,
    DesktopGlobalAfterAppFinished,
    DesktopGlobalAutoLock,
} DesktopEvent;
