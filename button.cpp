#include "button.h"

/*
 Pullup button wiring :
 GND => button => pin
 */

#define         BUTTON_LONGPRESS_DELAY  (2000)
static int8_t   gPrevButton             = -1;
static uint8_t  gCurrentIndex           = 0;


// ====================================================================================


void HW_SetupButton(SButtonData * aButton, uint8_t aPin, button_cb_t aCallback) {
    pinMode(aPin, INPUT_PULLUP);
    aButton->pin = aPin;
    aButton->callback = aCallback;
    aButton->longpress = false;
    aButton->longpressTS = 0;
    gPrevButton = -1;
    gCurrentIndex = 0;
}


static void HW_OnButtonReleased(SButtonData * aButton) {
    if(aButton->pin == gPrevButton) {
        // unclick
        if(aButton->longpress == false) {
            aButton->callback(aButton->pin, EButtonClick);
        }
        else {
            // unlongpress
            aButton->longpress = false;
        }
        gPrevButton = -1;
    }
}


static void HW_OnButtonPressed(SButtonData * aButton) {
    
    // previous code w/ longpress detection
    if(aButton->pin == gPrevButton) {
        // same pin still pressed
        if(aButton->longpress == false && (millis() - aButton->longpressTS) >= BUTTON_LONGPRESS_DELAY) {
            aButton->longpress = true;
            aButton->callback(aButton->pin, EButtonLongpress);
        }
    }
    else {
        // other pin pressed
        aButton->longpressTS = millis();
        gPrevButton = aButton->pin;
    }
}


void HW_ScanButton(SButtonData * aButton) {
    if(digitalRead(aButton->pin) == LOW) {
        // pressed
        HW_OnButtonPressed(aButton);
    }
    else {
        // released
        HW_OnButtonReleased(aButton);
    }
    
}


// void HW_SetupAnalog(SAnalogData * aAnalog, uint8_t aPin, analog_cb_t aCallback) {
//     pinMode(aPin, INPUT_PULLUP);
//     aAnalog->pin = aPin;
//     aAnalog->callback = aCallback;
//     aAnalog->value = (uint8_t)map(analogRead(aPin), 0, MAX_INPUT_STEPS, 0, MAX_OUTPUT_STEPS);
//     aAnalog->callback(aAnalog->pin, aAnalog->value);
    
//     //    dprint(F("analog setup pin "));
//     //    dprint(aPin);
//     //    dprint(F(" value "));
//     //    dprintln(aAnalog->value, DEC);
// }


// // scans expression pedal if any, and sends changes to amp
// void HW_ScanAnalog(SAnalogData * aAnalog) {
//     if(aAnalog->callback != NULL) {
//         uint8_t step256 = (uint8_t)map(analogRead(aAnalog->pin), 0, MAX_INPUT_STEPS, 0, MAX_OUTPUT_STEPS);
//         // basic filter
//         if(abs(step256 - aAnalog->value) > 1) {
//             //        dprint(F("analog read "));
//             //        dprintln(percent, DEC);
            
//             aAnalog->value = step256;
//             aAnalog->callback(aAnalog->pin, step256);
            
//         }
//     }
// }



// scans master volume offset pot if any, and sends new volume value to amp
//#ifdef USE_VOLUME_OFFSET
//static void HW_ScanVolumeOffset() {
//    int16_t offset = (uint8_t)map(analogRead(PIN_VOLUME_OFFSET), 0, MAX_INPUT_STEPS, -(MAX_OUTPUT_STEPS >> 1), (MAX_OUTPUT_STEPS >> 1));
//
//    // algo de filtrage de Leo/Alex (filtre à réponse impulsionelle infinie)
//#ifdef USE_FILTERING
//    float f1 = (float)((float)offset * FILTER_ALPHA);
//    float f2 = (float)((float)gVolumeOffset * (float)(1 - FILTER_ALPHA));
//    offset = (uint16_t)(f1 + f2);
//#endif
//
//    if(offset != gVolumeOffset) {
//        gVolumeOffset = offset;
//
//        dprint(F("BT OFFSET VOLUME BY ");
//        dprint(offset, DEC);
//        dprintln(F("%...");
//
//        char buffer[6] = "";
//        sprintf(buffer, "%c%d%03d", EBluetoothCommandSetPotValue, MASTER_POT_INDEX, gPatches[gCurrentPatchNum].step[MASTER_POT_INDEX] + offset);
//        buffer[4] = BT_END_CHAR;
//        gBTSerial.print(buffer);
//    }
//}
//#else
//#  define HW_ScanVolumeOffset()
//#endif
