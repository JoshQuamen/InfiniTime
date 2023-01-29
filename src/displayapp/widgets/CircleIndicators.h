#pragma once
#include <lvgl/lvgl.h>
#include "components/battery/BatteryController.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      struct Coordinates
      {
        lv_coord_t x;
        lv_coord_t y;
      };

      class BatteryWidgetIndicator{
        private:
          Pinetime::Controllers::Battery * batteryController;
          Coordinates coordinates;
          lv_obj_t* batteryArc;
          lv_obj_t* chargeIcon;
          Screens::BatteryIcon batteryIcon;
          Screens::DirtyValue<uint8_t> batteryPercentRemaining {0};
          Screens::DirtyValue<bool> isCharging {};
        public:
          BatteryWidgetIndicator(Coordinates coordinatesInital, Controllers::Battery * batteryControllerPtr);
          void SetPosition(Coordinates coordinates);
          Coordinates GetPosition();
          void Refresh();
          void SetHidden(bool enable);
          void ChangeColors(int arcFgColor, int arcBgColor, int objectColors);
      };
      class StepsWidgetIndicator{
        private:
          Pinetime::Controllers::Settings * settingsController;
          Pinetime::Controllers::MotionController * motionController;
          Coordinates coordinates;
          lv_obj_t* stepsArc;
          lv_obj_t* stepsIcon;
          lv_obj_t* stepsValue;
          Screens::DirtyValue<uint32_t> stepCount {};
          Screens::DirtyValue<bool> motionSensorOk {};
        public:
          StepsWidgetIndicator(Coordinates coordinatesInitial, Controllers::Settings * settingsController, Controllers::MotionController * motionController, const lv_font_t* font);
          void SetPosition(Coordinates coordinates);
          Coordinates GetPosition();
          void Refresh();
          void SetHidden(bool enable);
          void ChangeColors(int arcFgColor, int arcBgColor, int objectColors);
      };
      class DateWidgetIndicator{
        private:
          Pinetime::Controllers::DateTime * dateTimeController;
          Coordinates coordinates;
          lv_obj_t* label_date_day;
          lv_obj_t *dateArc;
          Screens::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;
          Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
          Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
          uint8_t currentDay = 0;
        public:
          DateWidgetIndicator(Coordinates coordinatesInitial, Controllers::DateTime * dateTimeController, const lv_font_t* font);
          void SetPosition(Coordinates coordinates);
          Coordinates GetPosition();
          void Refresh();
          void SetHidden(bool enable);
          void ChangeColors(int arcFgColor, int arcBgColor, int objectColors);
      };
      // class HeartRateWidgetIndicator: ICircleWidgetIndicator{

      // };
      // class NotificationsWidgetIndicator: ICircleWidgetIndicator{

      // };
      // class WeatherWidgetIndicator: ICircleWidgetIndicator{

      // };
    }
  }
}
