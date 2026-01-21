#pragma once

#include "smDevice.h"
#include <OneButton.h>

class Button : public smDevice {
public:
    Button(uint8_t aPin, bool aActiveLow = true, bool aUsePullup = true);
    ~Button();

    // Lifecycle
    bool begin() override;
    bool start() override;  // Start monitoring button
    void stop() override;   // Stop monitoring button
    void end() override;

    // Must be called regularly to process button events
    void tick();

    // Check if button was pressed since last check (clears flag)
    bool wasPressed();

    // Check if button was long-pressed since last check (clears flag)
    bool wasLongPressed();

    // Check if button was double-clicked since last check (clears flag)
    bool wasDoubleClicked();

private:
    static void onClickStatic(void* ptr);
    static void onLongPressStatic(void* ptr);
    static void onDoubleClickStatic(void* ptr);

    void onClick();
    void onLongPress();
    void onDoubleClick();

    OneButton mButton;
    volatile bool mClickedFlag;
    volatile bool mLongPressedFlag;
    volatile bool mDoubleClickedFlag;
};
