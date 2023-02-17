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

      class ICircleWidgetIndicator{
        public:
          virtual void SetPosition(Coordinates coordinates) = 0;
          virtual ~ICircleWidgetIndicator(){};
          virtual Coordinates GetPosition() = 0;
          virtual void Refresh() = 0;
          virtual void SetHidden(bool enable) = 0;
          virtual void ChangeColors(int arcFgColor, int arcBgColor, int objectColors) = 0;
        protected:
          bool hidden;
          lv_obj_t* arc;
          lv_obj_t* icon;
          lv_obj_t* value;
          Coordinates coordinates;
      };

      class BatteryWidgetIndicator: public ICircleWidgetIndicator{
        private:
          Pinetime::Controllers::Battery * batteryController;
          Screens::BatteryIcon batteryIcon;
          Screens::DirtyValue<uint8_t> batteryPercentRemaining {0};
          Screens::DirtyValue<bool> isCharging {};
        public:
          BatteryWidgetIndicator(Coordinates coordinatesInital, Controllers::Battery * batteryControllerPtr);
          ~BatteryWidgetIndicator();
          void SetPosition(Coordinates coordinates);
          Coordinates GetPosition();
          void Refresh();
          void SetHidden(bool enable);
          void ChangeColors(int arcFgColor, int arcBgColor, int objectColors);
      };
      class StepsWidgetIndicator: public ICircleWidgetIndicator{
        private:
          Pinetime::Controllers::Settings * settingsController;
          Pinetime::Controllers::MotionController * motionController;
          Screens::DirtyValue<uint32_t> stepCount {};
          Screens::DirtyValue<bool> motionSensorOk {};
        public:
          StepsWidgetIndicator(Coordinates coordinatesInitial, Controllers::Settings * settingsController, Controllers::MotionController * motionController, const lv_font_t* font);
          ~StepsWidgetIndicator();
          void SetPosition(Coordinates coordinates);
          Coordinates GetPosition();
          void Refresh();
          void SetHidden(bool enable);
          void ChangeColors(int arcFgColor, int arcBgColor, int objectColors);
      };
      class DateWidgetIndicator: public ICircleWidgetIndicator{
        private:
          const Pinetime::Controllers::DateTime * dateTimeController;
          Screens::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;
          Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
          Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
          uint8_t currentDay = 0;
        public:
          DateWidgetIndicator(Coordinates coordinatesInitial, const Controllers::DateTime * dateTimeController, const lv_font_t* font);
          ~DateWidgetIndicator();
          void SetPosition(Coordinates coordinates);
          Coordinates GetPosition();
          void Refresh();
          void SetHidden(bool enable);
          void ChangeColors(int arcFgColor, int arcBgColor, int objectColors);
      };
      class HeartRateWidgetIndicator: public ICircleWidgetIndicator{
        private:
          Pinetime::Controllers::HeartRateController * heartRateController;
          Screens::DirtyValue<uint8_t> heartbeat {};
          Screens::DirtyValue<bool> heartbeatRunning {};
        public:
          HeartRateWidgetIndicator(Coordinates coordinatesInitial, Controllers::HeartRateController * heartRateController, const lv_font_t* font);
          ~HeartRateWidgetIndicator();
          void SetPosition(Coordinates coordinates);
          Coordinates GetPosition();
          void Refresh();
          void SetHidden(bool enable);
          void ChangeColors(int arcFgColor, int arcBgColor, int objectColors);
      };
      
      // class WeatherWidgetIndicator: public ICircleWidgetIndicator{
      //   private:
      //     Pinetime::Controllers::WeatherService * weatherService;
      //     Coordinates coordinates;
      //     lv_obj_t* temperatureValue;
      //     lv_obj_t* weatherIcon;
      //     lv_obj_t *weatherArc;
      //     bool hidden;
      //   public:
      //     WeatherWidgetIndicator(Coordinates coordinatesInitial, Controllers::WeatherService * weatherService, const lv_font_t* font);
      //     void SetPosition(Coordinates coordinates);
      //     Coordinates GetPosition();
      //     void Refresh();
      //     void SetHidden(bool enable);
      //     void ChangeColors(int arcFgColor, int arcBgColor, int objectColors);
      // };
    }
  }
}
