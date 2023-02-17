#include "displayapp/widgets/CircleIndicators.h"
#include "displayapp/screens/Symbols.h"
#include "components/heartrate/HeartRateController.h"

using namespace Pinetime::Applications::Widgets;


/************************************************************\
|                                                            |
|                  Batery Widget Indicator                   |
|                                                            |
\************************************************************/
BatteryWidgetIndicator::BatteryWidgetIndicator(Coordinates coordinatesInitial, Controllers::Battery *batteryControllerPtr)
{
  coordinates = coordinatesInitial;
  batteryController = batteryControllerPtr;
  arc = lv_arc_create(lv_scr_act(), nullptr);
  lv_arc_set_bg_angles(arc, 180, 540);
  lv_arc_set_range(arc, 0, 100);
  lv_arc_set_adjustable(arc, false);
  lv_obj_set_width(arc, lv_obj_get_width(lv_scr_act())*0.3);
  lv_obj_set_height(arc, lv_obj_get_height(lv_scr_act())*0.3);
  lv_obj_align(arc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  
  lv_obj_set_style_local_line_width(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_opa(arc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_click(arc, false);

  batteryIcon.Create(lv_scr_act());
  lv_obj_align(batteryIcon.GetObject(), arc, LV_ALIGN_CENTER, 0, 0);

  icon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(icon, Screens::Symbols::lightning);
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_obj_align(icon, arc, LV_ALIGN_CENTER, 0, 0);
  hidden = false;
}
BatteryWidgetIndicator::~BatteryWidgetIndicator()
{
  lv_obj_del(arc);
  lv_obj_del(icon);
  lv_obj_del(batteryIcon.GetObject());
}

void BatteryWidgetIndicator::SetPosition(Coordinates newCoordinates)
{
  coordinates = newCoordinates;
  lv_obj_align(this->arc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_realign(batteryIcon.GetObject());
  lv_obj_realign(icon);
}
Coordinates BatteryWidgetIndicator::GetPosition()
{
  return coordinates;
}

void BatteryWidgetIndicator::Refresh()
{
  if(!hidden)
  {
    batteryPercentRemaining = batteryController->PercentRemaining();
    if(batteryPercentRemaining.IsUpdated()) {
      lv_arc_set_value(arc, batteryPercentRemaining.Get());
    }
    isCharging = batteryController->IsCharging();
    if(isCharging.Get()){
      batteryIcon.SetBatteryPercentage(100);
      lv_obj_set_hidden(icon, false);
    }
    else{
      batteryIcon.SetBatteryPercentage(batteryPercentRemaining.Get());
      lv_obj_set_hidden(icon, true);
    }
  }
  
}

void BatteryWidgetIndicator::SetHidden(bool enable)
{
  lv_obj_set_hidden(arc, enable);
  lv_obj_set_hidden(icon, enable);
  lv_obj_set_hidden(batteryIcon.GetObject(), enable);
  hidden = enable;
}

void BatteryWidgetIndicator::ChangeColors(int arcFgColor, int arcBgColor, int objectColors)
{
  lv_obj_set_style_local_line_color(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(arcFgColor));
  lv_obj_set_style_local_line_color(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, lv_color_hex(arcBgColor));
  batteryIcon.SetColor(lv_color_hex(objectColors));
}


/************************************************************\
|                                                            |
|                   Steps Widget Indicator                   |
|                                                            |
\************************************************************/
StepsWidgetIndicator::StepsWidgetIndicator(Coordinates coordinatesInitial, Controllers::Settings * settingsCtrl, Controllers::MotionController * motionCtrl, const lv_font_t* font)
{
  coordinates = coordinatesInitial;
  settingsController = settingsCtrl;
  motionController = motionCtrl;

  arc = lv_arc_create(lv_scr_act(), nullptr);
  lv_arc_set_bg_angles(arc, 180, 540);
  lv_arc_set_range(arc, 0, settingsController->GetStepsGoal());
  lv_arc_set_adjustable(arc, false);
  lv_obj_set_width(arc, lv_obj_get_width(lv_scr_act())*0.3);
  lv_obj_set_height(arc, lv_obj_get_height(lv_scr_act())*0.3);
  lv_obj_align(arc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_set_style_local_line_width(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_opa(arc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_click(arc, false);

  icon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(icon, Screens::Symbols::shoe);
  lv_obj_align(icon, arc, LV_ALIGN_CENTER, 0, 10);
  value = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
  lv_obj_align(value, arc, LV_ALIGN_CENTER, 0, -10);
  lv_label_set_text(value, "0");
  hidden = false;
}
StepsWidgetIndicator::~StepsWidgetIndicator()
{
  lv_obj_del(arc);
  lv_obj_del(icon);
  lv_obj_del(value);
}
void StepsWidgetIndicator::SetPosition(Coordinates newCoordinates)
{
  coordinates = newCoordinates;
  lv_obj_align(arc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_realign(icon);
  lv_obj_realign(value);
}

Coordinates StepsWidgetIndicator::GetPosition()
{
  return coordinates;
}

void StepsWidgetIndicator::Refresh()
{
  if(!hidden)
  {
    stepCount = motionController->NbSteps();
    motionSensorOk = motionController->IsSensorOk();
    if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
      lv_label_set_text_fmt(value, "%lu", stepCount.Get());
      lv_obj_align(value, arc, LV_ALIGN_CENTER, 0, -10);
      lv_arc_set_value(arc, stepCount.Get());
    }
  }
  
}
void StepsWidgetIndicator::SetHidden(bool enable)
{
  lv_obj_set_hidden(arc, enable);
  lv_obj_set_hidden(icon, enable);
  lv_obj_set_hidden(value, enable);
  hidden = enable;
}
void StepsWidgetIndicator::ChangeColors(int arcFgColor, int arcBgColor, int objectColors)
{
  lv_obj_set_style_local_line_color(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(arcFgColor));
  lv_obj_set_style_local_line_color(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, lv_color_hex(arcBgColor));
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(objectColors));
  lv_obj_set_style_local_text_color(value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(objectColors));
  
}

/************************************************************\
|                                                            |
|                   Date Widget Indicator                    |
|                                                            |
\************************************************************/
DateWidgetIndicator::DateWidgetIndicator(Coordinates coordinatesInitial, const Controllers::DateTime * dateTimeCtrl, const lv_font_t* font)
{
  coordinates = coordinatesInitial;
  dateTimeController = dateTimeCtrl;
  arc = lv_arc_create(lv_scr_act(), nullptr);
  lv_arc_set_bg_angles(arc, 180, 540);
  lv_arc_set_range(arc, 0, 1);
  lv_arc_set_value(arc, 1);
  lv_arc_set_adjustable(arc, false);
  lv_obj_set_width(arc, lv_obj_get_width(lv_scr_act())*0.3);
  lv_obj_set_height(arc, lv_obj_get_height(lv_scr_act())*0.3);
  lv_obj_align(arc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_set_style_local_line_width(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_opa(arc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_click(arc, false);

  value = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_font(value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
  lv_label_set_text_fmt(value, "%s\n%02i", dateTimeController->DayOfWeekShortToStringLow(), dateTimeController->Day());
  lv_label_set_align(value, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(value, arc, LV_ALIGN_CENTER, 0, 0);
  hidden = false;
}
DateWidgetIndicator::~DateWidgetIndicator()
{
  lv_obj_del(arc);
  lv_obj_del(value);
}
void DateWidgetIndicator::SetPosition(Coordinates newCoordinates)
{
  coordinates = newCoordinates;
  lv_obj_align(arc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_realign(value);
}

Coordinates DateWidgetIndicator::GetPosition()
{
  return coordinates;
}

void DateWidgetIndicator::Refresh()
{
  if(!hidden)
  {
    currentDateTime = dateTimeController->CurrentDateTime();

    if (currentDateTime.IsUpdated()) {
      Pinetime::Controllers::DateTime::Months month = dateTimeController->Month();
      uint8_t day = dateTimeController->Day();
      Pinetime::Controllers::DateTime::Days dayOfWeek = dateTimeController->DayOfWeek();

      if ((month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
        lv_label_set_text_fmt(value, "%s\n%02i", dateTimeController->DayOfWeekShortToStringLow(), day);

        currentMonth = month;
        currentDayOfWeek = dayOfWeek;
        currentDay = day;
      }
    }
  }
  
}
void DateWidgetIndicator::SetHidden(bool enable)
{
  lv_obj_set_hidden(arc, enable);
  lv_obj_set_hidden(value, enable);
  hidden = enable;
}
void DateWidgetIndicator::ChangeColors(int arcFgColor, int arcBgColor, int objectColors)
{
  lv_obj_set_style_local_line_color(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(arcFgColor));
  lv_obj_set_style_local_line_color(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, lv_color_hex(arcBgColor));
  lv_obj_set_style_local_text_color(value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(objectColors));  
}

/************************************************************\
|                                                            |
|                 Heart Rate Widget Indicator                |
|                                                            |
\************************************************************/
HeartRateWidgetIndicator::HeartRateWidgetIndicator(Coordinates coordinatesInitial, Controllers::HeartRateController * heartRateCtrl, const lv_font_t* font)
{
  coordinates = coordinatesInitial;
  heartRateController = heartRateCtrl;

  arc = lv_arc_create(lv_scr_act(), nullptr);
  lv_arc_set_bg_angles(arc, 180, 540);
  lv_arc_set_range(arc, 0, 1);
  lv_arc_set_value(arc, 1);
  lv_arc_set_adjustable(arc, false);
  lv_obj_set_width(arc, lv_obj_get_width(lv_scr_act())*0.3);
  lv_obj_set_height(arc, lv_obj_get_height(lv_scr_act())*0.3);
  lv_obj_align(arc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_set_style_local_line_width(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_opa(arc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_click(arc, false);

  icon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(icon, Screens::Symbols::heartBeat);
  lv_obj_align(icon, arc, LV_ALIGN_CENTER, 0, 10);
  value = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
  lv_obj_align(value, arc, LV_ALIGN_CENTER, 0, -10);
  lv_label_set_text(value, ". . .");
  hidden = false;
}
HeartRateWidgetIndicator::~HeartRateWidgetIndicator()
{
  lv_obj_del(arc);
  lv_obj_del(icon);
  lv_obj_del(value);
}
void HeartRateWidgetIndicator::SetPosition(Coordinates newCoordinates)
{
  coordinates = newCoordinates;
  lv_obj_align(arc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_realign(icon);
  lv_obj_realign(value);
}

Coordinates HeartRateWidgetIndicator::GetPosition()
{
  return coordinates;
}

void HeartRateWidgetIndicator::Refresh()
{
  if(!hidden)
  {
    heartbeat = heartRateController->HeartRate();
    heartbeatRunning = heartRateController->State() != Controllers::HeartRateController::States::Stopped;
    if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
      if (heartbeatRunning.Get()) {
        lv_label_set_text_fmt(value, "%3d", heartbeat.Get());
      } else {
        lv_label_set_text_static(value, ". . .");
      }
    }
  }
}
void HeartRateWidgetIndicator::SetHidden(bool enable)
{
  lv_obj_set_hidden(arc, enable);
  lv_obj_set_hidden(icon, enable);
  lv_obj_set_hidden(value, enable);
  hidden = enable;
}
void HeartRateWidgetIndicator::ChangeColors(int arcFgColor, int arcBgColor, int objectColors)
{
  lv_obj_set_style_local_line_color(arc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(arcFgColor));
  lv_obj_set_style_local_line_color(arc, LV_ARC_PART_BG, LV_STATE_DEFAULT, lv_color_hex(arcBgColor));
  lv_obj_set_style_local_text_color(icon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(objectColors));
  lv_obj_set_style_local_text_color(value, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(objectColors));
  
}

/************************************************************\
|                                                            |
|                   Weather Widget Indicator                 |
|                                                            |   Add Support As Weather becomes Supported
\************************************************************/
// WeatherWidgetIndicator::WeatherWidgetIndicator(Coordinates coordinatesInitial, Controllers::WeatherService * weatherSrvc, const lv_font_t* font)
// {
//   coordinates = coordinatesInitial;
//   weatherService = weatherSrvc;

//   weatherArc = lv_arc_create(lv_scr_act(), nullptr);
//   lv_arc_set_bg_angles(arc, 180, 540);
//   lv_arc_set_range(weatherArc, 0, 1);
//   lv_arc_set_value(weatherArc, 1);
//   lv_arc_set_adjustable(weatherArc, false);
//   lv_obj_set_width(weatherArc, lv_obj_get_width(lv_scr_act())*0.3);
//   lv_obj_set_height(weatherArc, lv_obj_get_height(lv_scr_act())*0.3);
//   lv_obj_align(weatherArc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
//   lv_obj_set_style_local_line_width(weatherArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
//   lv_obj_set_style_local_line_width(weatherArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
//   lv_obj_set_style_local_line_opa(weatherArc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
//   lv_obj_set_click(weatherArc, false);

//   weatherIcon = lv_label_create(lv_scr_act(), nullptr);
//   lv_label_set_text_static(weatherIcon, Screens::Symbols::heartBeat);
//   lv_obj_align(weatherIcon, weatherArc, LV_ALIGN_CENTER, 0, 10);
//   temperatureValue = lv_label_create(lv_scr_act(), nullptr);
//   lv_obj_set_style_local_text_font(temperatureValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
//   lv_obj_align(temperatureValue, weatherArc, LV_ALIGN_CENTER, 0, -10);
//   lv_label_set_text(temperatureValue, ". . .");
//   hidden = false;
// }

// void WeatherWidgetIndicator::SetPosition(Coordinates newCoordinates)
// {
//   coordinates = newCoordinates;
//   lv_obj_align(weatherArc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
//   lv_obj_realign(weatherIcon);
//   lv_obj_realign(temperatureValue);
// }

// Coordinates WeatherWidgetIndicator::GetPosition()
// {
//   return coordinates;
// }

// void WeatherWidgetIndicator::Refresh()
// {
//   if(!hidden)
//   {
//     int16_t maxTemp = weatherService->GetTodayMaxTemp();
//     int16_t currentTemp = weatherService->GetCurrentTemperature().temperature;
//     if(maxTemp == -32768 || currentTemp = -32768)
//     {
//       lv_arc_set_range(weatherArc, 0, 1);
//       lv_arc_set_value(weatherArc, 1);
//     }
//     else
//     {
//       lv_arc_set_range(weatherArc, 0, maxTemp);
//       lv_arc_set_value(weatherArc, currentTemp);
//     }
//   }
// }
// void WeatherWidgetIndicator::SetHidden(bool enable)
// {
//   lv_obj_set_hidden(weatherArc, enable);
//   lv_obj_set_hidden(weatherIcon, enable);
//   lv_obj_set_hidden(temperatureValue, enable);
//   hidden = enable;
// }
// void WeatherWidgetIndicator::ChangeColors(int arcFgColor, int arcBgColor, int objectColors)
// {
//   lv_obj_set_style_local_line_color(weatherArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(arcFgColor));
//   lv_obj_set_style_local_line_color(weatherArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, lv_color_hex(arcBgColor));
//   lv_obj_set_style_local_text_color(weatherIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(objectColors));
//   lv_obj_set_style_local_text_color(temperatureValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(objectColors));
  
// }