#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"
#include "components/heartrate/HeartRateController.h"
#include <displayapp/screens/BatteryIcon.h>
#include "displayapp/widgets/CircleIndicators.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class MotionController;
    class HeartRateController;
  }
  namespace Applications {
    namespace Screens {

      class WatchFaceModernAnalog : public Screen {
      public:
        WatchFaceModernAnalog(DisplayApp* app,
                              Controllers::DateTime& dateTimeController,
                              Controllers::Battery& batteryController,
                              Controllers::Ble& bleController,
                              Controllers::NotificationManager& notificationManager,
                              Controllers::Settings& settingsController,
                              Controllers::MotionController& motionController,
                              Controllers::HeartRateController& heartRateController,
                              Controllers::FS& fs);

        ~WatchFaceModernAnalog() override;

        void Refresh() override;

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        uint8_t sHour, sMinute, sSecond;

        DirtyValue<bool> bleState {};
        DirtyValue<bool> bleRadioEnabled {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;
        DirtyValue<size_t> notificationNumber {};
        
        
        lv_obj_t* hour_body;
        lv_obj_t* hour_body_trace;
        lv_obj_t* minute_body;
        lv_obj_t* minute_body_trace;
        lv_obj_t* second_body;
        lv_obj_t* tick_lines[51];

        lv_point_t hour_point[2];
        lv_point_t hour_point_trace[2];
        lv_point_t minute_point[2];
        lv_point_t minute_point_trace[2];
        lv_point_t second_point[2];
        lv_point_t tick_lines_points[51][2];

        lv_style_t hour_line_style;
        lv_style_t hour_line_style_trace;
        lv_style_t minute_line_style;
        lv_style_t minute_line_style_trace;
        lv_style_t second_line_style;
        lv_style_t tick_lines_style[51];

        
        lv_obj_t* logoPine;
        lv_obj_t* label_hour_minute;
        lv_obj_t* bleIcon;
        lv_obj_t* notificationIcon;

        Widgets::ICircleWidgetIndicator* circleWidgets[3];
        static const int8_t numberOfWidgets = 4;
        enum CircleWidgetName{
          BatteryWidget,
          StepsWidget,
          DateWidget,
          HeartRateWidget
        };
        CircleWidgetName circleWidgetNames[3];
        int16_t leadWidgetAngle;
        bool rotateWidgets;

        const Controllers::DateTime& dateTimeController;
        Controllers::Battery& batteryController;
        Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;
        Controllers::HeartRateController& heartRateController;

        void UpdateClock();
        void RotateWidgets();
        bool OnTouchEvent(TouchEvents event) override;
        void CreateWidgets();

        lv_task_t* taskRefresh;
        lv_font_t* font_segment40 = nullptr;
        lv_font_t* font_teko = nullptr;
      };
    }
  }
}
