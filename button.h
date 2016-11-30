


#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "Arduino.h"

typedef enum EButtonScanResult {
    EButtonNone = 0,
    EButtonClick,
    EButtonLongpress
} EButtonScanResult;

//typedef void (*encoder_cb_t)(uint8_t, EEncoderScanResult);
typedef void (*button_cb_t)(uint8_t, EButtonScanResult);
//typedef void (*analog_cb_t)(uint8_t, uint8_t);
//typedef void (*com_cb_t)(const char *);


typedef struct SButtonData {
    uint8_t     pin;
    bool        longpress;
    uint32_t    longpressTS;
    button_cb_t callback;
} SButtonData;

// typedef struct SAnalogData {
//     uint8_t     pin;
//     uint8_t     value; // 0..255
//     analog_cb_t callback;
// } SAnalogData;


// ====================================================================================
#pragma mark - HARDWARE CONTROL ROUTINES
// ====================================================================================

//void HW_ScanEncoders();
//void HW_ScanEncoder(SEncoderData * aEncoder);
//void HW_ScanSwitches();
void HW_ScanButton(SButtonData * aButton);
void HW_ScanButtons(SButtonData * aButtonArray[], uint8_t aCount);
//void HW_SetupEncoders(int aEncoderCount);
void HW_SetupButtons(int aButtonCount);
void HW_SetupButton(SButtonData * aButton, uint8_t aPin, button_cb_t aCallback);
//void HW_SetupEncoder(SEncoderData * aEncoder, uint8_t aPinA, uint8_t aPinB, uint8_t aPinClick, encoder_cb_t aCallback);
//void HW_AddEncoder(uint8_t aPinA, uint8_t aPinB, uint8_t aPinClick, encoder_cb_t aCallback);
void HW_AddButton(uint8_t aButtonPin, button_cb_t aCallback);
// void HW_SetupAnalog(SAnalogData * aAnalog, uint8_t aPin, analog_cb_t aCallback);
// void HW_ScanAnalog(SAnalogData * aAnalog);


#endif
