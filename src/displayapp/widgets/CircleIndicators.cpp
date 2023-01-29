#include "displayapp/widgets/CircleIndicators.h"
#include "displayapp/screens/Symbols.h"

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
  batteryArc = lv_arc_create(lv_scr_act(), nullptr);
  lv_arc_set_bg_angles(batteryArc, 180, 540);
  lv_arc_set_range(batteryArc, 0, 100);
  lv_arc_set_adjustable(batteryArc, false);
  lv_obj_set_width(batteryArc, lv_obj_get_width(lv_scr_act())*0.3);
  lv_obj_set_height(batteryArc, lv_obj_get_height(lv_scr_act())*0.3);
  lv_obj_align(batteryArc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  
  lv_obj_set_style_local_line_width(batteryArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(batteryArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_opa(batteryArc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_click(batteryArc, false);

  batteryIcon.Create(lv_scr_act());
  lv_obj_align(batteryIcon.GetObject(), batteryArc, LV_ALIGN_CENTER, 0, 0);

  chargeIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(chargeIcon, Screens::Symbols::lightning);
  lv_obj_set_style_local_text_color(chargeIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_obj_align(chargeIcon, batteryArc, LV_ALIGN_CENTER, 0, 0);
}

void BatteryWidgetIndicator::SetPosition(Coordinates newCoordinates)
{
  coordinates = newCoordinates;
  lv_obj_align(this->batteryArc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_align(batteryIcon.GetObject(), batteryArc, LV_ALIGN_CENTER, 0, 0);
  lv_obj_align(chargeIcon, batteryArc, LV_ALIGN_CENTER, 0, 0);
}
Coordinates BatteryWidgetIndicator::GetPosition()
{
  return coordinates;
}

void BatteryWidgetIndicator::Refresh()
{
  batteryPercentRemaining = batteryController->PercentRemaining();
  if(batteryPercentRemaining.IsUpdated()) {
    lv_arc_set_value(batteryArc, batteryPercentRemaining.Get());
  }
  isCharging = batteryController->IsCharging();
  if(isCharging.Get()){
    batteryIcon.SetBatteryPercentage(100);
    lv_obj_set_hidden(chargeIcon, false);
  }
  else{
    batteryIcon.SetBatteryPercentage(batteryPercentRemaining.Get());
    lv_obj_set_hidden(chargeIcon, true);
  }
}

void BatteryWidgetIndicator::SetHidden(bool enable)
{
  lv_obj_set_hidden(batteryArc, enable);
  lv_obj_set_hidden(chargeIcon, enable);
  lv_obj_set_hidden(batteryIcon.GetObject(), enable);
}

void BatteryWidgetIndicator::ChangeColors(int arcFgColor, int arcBgColor, int objectColors)
{
  lv_obj_set_style_local_line_color(batteryArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(arcFgColor));
  lv_obj_set_style_local_line_color(batteryArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, lv_color_hex(arcBgColor));
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

  stepsArc = lv_arc_create(lv_scr_act(), nullptr);
  lv_arc_set_bg_angles(stepsArc, 180, 540);
  lv_arc_set_range(stepsArc, 0, settingsController->GetStepsGoal());
  lv_arc_set_adjustable(stepsArc, false);
  lv_obj_set_width(stepsArc, lv_obj_get_width(lv_scr_act())*0.3);
  lv_obj_set_height(stepsArc, lv_obj_get_height(lv_scr_act())*0.3);
  lv_obj_align(stepsArc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_set_style_local_line_width(stepsArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_opa(stepsArc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_click(stepsArc, false);

  stepsIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(stepsIcon, Screens::Symbols::shoe);
  lv_obj_align(stepsIcon, stepsArc, LV_ALIGN_CENTER, 0, 10);
  stepsValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(stepsValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font);
  lv_obj_align(stepsValue, stepsArc, LV_ALIGN_CENTER, 0, -10);
  lv_label_set_text(stepsValue, "0");
}

void StepsWidgetIndicator::SetPosition(Coordinates newCoordinates)
{
  coordinates = newCoordinates;
  lv_obj_align(stepsArc, NULL, LV_ALIGN_CENTER, coordinates.x, coordinates.y);
  lv_obj_align(stepsIcon, stepsArc, LV_ALIGN_CENTER, 0, 10);
  lv_obj_align(stepsValue, stepsArc, LV_ALIGN_CENTER, 0, -10);
}

Coordinates StepsWidgetIndicator::GetPosition()
{
  return coordinates;
}

void StepsWidgetIndicator::Refresh()
{
  stepCount = motionController->NbSteps();
  motionSensorOk = motionController->IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(stepsValue, "%lu", stepCount.Get());
    lv_obj_align(stepsValue, stepsArc, LV_ALIGN_CENTER, 0, -10);
    lv_arc_set_value(stepsArc, stepCount.Get());
  }
}
void StepsWidgetIndicator::SetHidden(bool enable)
{
  lv_obj_set_hidden(stepsArc, enable);
  lv_obj_set_hidden(stepsIcon, enable);
  lv_obj_set_hidden(stepsValue, enable);
}
void StepsWidgetIndicator::ChangeColors(int arcFgColor, int arcBgColor, int objectColors)
{
  lv_obj_set_style_local_line_color(stepsArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(arcFgColor));
  lv_obj_set_style_local_line_color(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, lv_color_hex(arcBgColor));
  lv_obj_set_style_local_text_color(stepsIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(objectColors));
  lv_obj_set_style_local_text_color(stepsValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(objectColors));
  
}