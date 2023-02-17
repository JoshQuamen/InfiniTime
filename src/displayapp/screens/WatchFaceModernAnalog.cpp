#include "displayapp/screens/WatchFaceModernAnalog.h"

#include <lvgl/lvgl.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/NotificationIcon.h"
#include "components/settings/Settings.h"
#include "components/motion/MotionController.h"
#include "components/heartrate/HeartRateController.h"
#include "displayapp/InfiniTimeTheme.h"

LV_IMG_DECLARE(bg_clock);
#define INDICATOR_ARC_SIZE lv_obj_get_width(lv_scr_act())*0.3
using namespace Pinetime::Applications::Screens;

namespace {
  constexpr int16_t HourLength = 70;
  constexpr int16_t MinuteLength = 90;
  constexpr int16_t SecondLength = 110;
  constexpr int16_t FivesTickLength = 5;
  constexpr int16_t OnesTickLength = 3;
  constexpr float PI = 3.14159265358979323846;
  constexpr int16_t WidgetRotationRadius = 60;


  // sin(90) = 1 so the value of _lv_trigo_sin(90) is the scaling factor
  const auto LV_TRIG_SCALE = _lv_trigo_sin(90);

  int16_t Cosine(int16_t angle) {
    return _lv_trigo_sin(angle + 90);
  }

  int16_t Sine(int16_t angle) {
    return _lv_trigo_sin(angle);
  }

  int16_t CoordinateXRelocate(int16_t x) {
    return (x + LV_HOR_RES / 2);
  }

  int16_t CoordinateYRelocate(int16_t y) {
    return std::abs(y - LV_HOR_RES / 2);
  }

  lv_point_t CoordinateRelocate(int16_t radius, int16_t angle) {
    return lv_point_t {.x = CoordinateXRelocate(radius * static_cast<int32_t>(Sine(angle)) / LV_TRIG_SCALE),
                       .y = CoordinateYRelocate(radius * static_cast<int32_t>(Cosine(angle)) / LV_TRIG_SCALE)};
  }

}

WatchFaceModernAnalog::WatchFaceModernAnalog(DisplayApp* app,
                                            Controllers::DateTime& dateTimeController,
                                            Controllers::Battery& batteryController,
                                            Controllers::Ble& bleController,
                                            Controllers::NotificationManager& notificationManager,
                                            Controllers::Settings& settingsController,
                                            Controllers::MotionController& motionController,
                                            Controllers::HeartRateController& heartRateController,
                                            Controllers::FS& filesystem)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController},
    heartRateController {heartRateController}{
  
  rotateWidgets = false;
  circleWidgetNames[0] = BatteryWidget;
  circleWidgetNames[1] = StepsWidget;
  circleWidgetNames[2] = DateWidget;

  lfs_file f = {};
  if (filesystem.FileOpen(&f, "/fonts/7segments_40.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_segment40 = lv_font_load("F:/fonts/7segments_40.bin");
  }
  if (filesystem.FileOpen(&f, "/fonts/teko.bin", LFS_O_RDONLY) >= 0) {
    filesystem.FileClose(&f);
    font_teko = lv_font_load("F:/fonts/teko.bin");
  }

  sHour = 99;
  sMinute = 99;
  sSecond = 99;

  // Clock Face
  lv_coord_t x1, x2, y1, y2;
  for(int i = 0; i < 51; i++)
  {
    tick_lines[i] = lv_line_create(lv_scr_act(), nullptr);
    lv_style_init(&(tick_lines_style[i]));
    lv_style_set_line_width(&(tick_lines_style[i]), LV_STATE_DEFAULT, (i%5 == 0 ? 4:2));
    lv_style_set_line_color(&(tick_lines_style[i]), LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_add_style(tick_lines[i], LV_LINE_PART_MAIN, &(tick_lines_style[i]));
    x1 = (lv_coord_t)(115*Cosine(i*6+300)/LV_TRIG_SCALE+120);
    x2 = (lv_coord_t)((i%5==0?105:110)*Cosine(i*6+300)/LV_TRIG_SCALE+120);
    y1 = (lv_coord_t)(115*Sine(i*6+300)/LV_TRIG_SCALE+120);
    y2 = (lv_coord_t)((i%5==0?105:110)*Sine(i*6+300)/LV_TRIG_SCALE+120);
    tick_lines_points[i][0] = {x1, y1};
    tick_lines_points[i][1] = {x2, y2};
    lv_line_set_points(tick_lines[i], tick_lines_points[i], 2);
  }

  logoPine = lv_img_create(lv_scr_act(), nullptr);
  lv_obj_align(logoPine, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_img_set_src(logoPine, "F:/images/pine_small.bin");
  lv_obj_set_pos(logoPine, (lv_obj_get_width(lv_scr_act()) - lv_obj_get_width(logoPine))/2, 0);
  lv_obj_set_style_local_image_recolor_opa(logoPine, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
  lv_obj_set_style_local_image_recolor(logoPine, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  CreateWidgets();


  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_align(bleIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  notificationIcon = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text_static(notificationIcon, Symbols::info);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  label_hour_minute = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(label_hour_minute, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  int hour = dateTimeController.Hours(); 
  if(settingsController.GetClockType() == Controllers::Settings::ClockType::H12 && hour > 12)
    hour -= 12;
  lv_label_set_text_fmt(label_hour_minute, "%d:%02d", hour, dateTimeController.Minutes());
  lv_obj_set_style_local_text_font(label_hour_minute, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_segment40);
  lv_label_set_align(label_hour_minute, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_hour_minute, NULL, LV_ALIGN_CENTER, 0, -INDICATOR_ARC_SIZE + 10);

  minute_body = lv_line_create(lv_scr_act(), NULL);
  minute_body_trace = lv_line_create(lv_scr_act(), NULL);
  hour_body = lv_line_create(lv_scr_act(), NULL);
  hour_body_trace = lv_line_create(lv_scr_act(), NULL);
  second_body = lv_line_create(lv_scr_act(), NULL);

  lv_style_init(&second_line_style);
  lv_style_set_line_width(&second_line_style, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&second_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&second_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(second_body, LV_LINE_PART_MAIN, &second_line_style);

  lv_style_init(&minute_line_style);
  lv_style_set_line_width(&minute_line_style, LV_STATE_DEFAULT, 7);
  lv_style_set_line_color(&minute_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&minute_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(minute_body, LV_LINE_PART_MAIN, &minute_line_style);

  lv_style_init(&minute_line_style_trace);
  lv_style_set_line_width(&minute_line_style_trace, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&minute_line_style_trace, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&minute_line_style_trace, LV_STATE_DEFAULT, false);
  lv_obj_add_style(minute_body_trace, LV_LINE_PART_MAIN, &minute_line_style_trace);

  lv_style_init(&hour_line_style);
  lv_style_set_line_width(&hour_line_style, LV_STATE_DEFAULT, 7);
  lv_style_set_line_color(&hour_line_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&hour_line_style, LV_STATE_DEFAULT, true);
  lv_obj_add_style(hour_body, LV_LINE_PART_MAIN, &hour_line_style);

  lv_style_init(&hour_line_style_trace);
  lv_style_set_line_width(&hour_line_style_trace, LV_STATE_DEFAULT, 3);
  lv_style_set_line_color(&hour_line_style_trace, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_line_rounded(&hour_line_style_trace, LV_STATE_DEFAULT, false);
  lv_obj_add_style(hour_body_trace, LV_LINE_PART_MAIN, &hour_line_style_trace);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);

  Refresh();
}

WatchFaceModernAnalog::~WatchFaceModernAnalog() {
  delete circleWidgets[0];
  delete circleWidgets[1];
  delete circleWidgets[2];

  lv_task_del(taskRefresh);
  
  for(int i = 0; i < 51; i++)
  {
    lv_style_reset(&(tick_lines_style[i]));
  }
  lv_style_reset(&hour_line_style);
  lv_style_reset(&hour_line_style_trace);
  lv_style_reset(&minute_line_style);
  lv_style_reset(&minute_line_style_trace);
  lv_style_reset(&second_line_style);
  if (font_segment40 != nullptr) {
    lv_font_free(font_segment40);
  }
  if (font_teko != nullptr) {
    lv_font_free(font_teko);
  }
  lv_obj_clean(lv_scr_act());
}

void WatchFaceModernAnalog::UpdateClock() {
  uint8_t hour = dateTimeController.Hours();
  uint8_t minute = dateTimeController.Minutes();
  uint8_t second = dateTimeController.Seconds();

  if (sMinute != minute) {
    auto const angle = minute * 6;
    minute_point[0] = CoordinateRelocate(30, angle);
    minute_point[1] = CoordinateRelocate(MinuteLength, angle);

    minute_point_trace[0] = CoordinateRelocate(5, angle);
    minute_point_trace[1] = CoordinateRelocate(31, angle);

    lv_line_set_points(minute_body, minute_point, 2);
    lv_line_set_points(minute_body_trace, minute_point_trace, 2);
  }

  if (sHour != hour || sMinute != minute) {
    sHour = hour;
    sMinute = minute;
    auto const angle = (hour * 30 + minute / 2);

    hour_point[0] = CoordinateRelocate(30, angle);
    hour_point[1] = CoordinateRelocate(HourLength, angle);

    hour_point_trace[0] = CoordinateRelocate(5, angle);
    hour_point_trace[1] = CoordinateRelocate(31, angle);

    lv_line_set_points(hour_body, hour_point, 2);
    lv_line_set_points(hour_body_trace, hour_point_trace, 2);
    if(settingsController.GetClockType() == Controllers::Settings::ClockType::H12 && hour > 12)
      hour -= 12;
    lv_label_set_text_fmt(label_hour_minute, "%d:%02d", hour, minute);
  }

  if (sSecond != second) {
    sSecond = second;
    auto const angle = second * 6;

    second_point[0] = CoordinateRelocate(-20, angle);
    second_point[1] = CoordinateRelocate(SecondLength, angle);
    lv_line_set_points(second_body, second_point, 2);
  }
}
bool WatchFaceModernAnalog::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  if ((event == Pinetime::Applications::TouchEvents::SwipeLeft)){
    if(!rotateWidgets){
      rotateWidgets = true;
      leadWidgetAngle = 90;
      delete circleWidgets[0];
      circleWidgets[0] = circleWidgets[1];
      circleWidgets[1] = circleWidgets[2];
      circleWidgetNames[0] = circleWidgetNames[1];
      circleWidgetNames[1] = circleWidgetNames[2];
      circleWidgetNames[2] = static_cast<CircleWidgetName>((circleWidgetNames[1]+1) % numberOfWidgets);
      switch(circleWidgetNames[2])
      {
        case BatteryWidget:
          circleWidgets[2] = new Widgets::BatteryWidgetIndicator({WidgetRotationRadius, 0}, &batteryController);
          break;
        case StepsWidget:
          circleWidgets[2] = new Widgets::StepsWidgetIndicator({WidgetRotationRadius, 0}, &settingsController, &motionController, font_teko);
          break;
        case DateWidget:
          circleWidgets[2] = new Widgets::DateWidgetIndicator({WidgetRotationRadius, 0}, &dateTimeController, font_teko);
          break;
        case HeartRateWidget:
        default:
          circleWidgets[2] = new Widgets::HeartRateWidgetIndicator({WidgetRotationRadius, 0}, &heartRateController, font_teko);
          break;
      }
      circleWidgets[2]->ChangeColors(0x00FFFF, 0x808080, 0x808080);
      circleWidgets[2]->SetHidden(true);
    }
    return true;
  }
  else
    return false;
}

void WatchFaceModernAnalog::Refresh() {
  
  for(auto widget: circleWidgets)
  {
    widget->Refresh();
  }

  if(rotateWidgets)
    RotateWidgets();

  bleState = bleController.IsConnected();
  if(bleState.IsUpdated()){
    lv_obj_set_hidden(bleIcon, !bleState.Get());
  }

  notificationNumber = notificationManager.NbNotifications();

  if (notificationNumber.IsUpdated()) {
    lv_obj_set_hidden(notificationIcon, notificationNumber.Get() == 0);
  }

  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    UpdateClock();
  }
}

bool WatchFaceModernAnalog::IsAvailable(Pinetime::Controllers::FS& filesystem) {
  lfs_file file = {};
  if (filesystem.FileOpen(&file, "/fonts/teko.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/fonts/7segments_40.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);
  if (filesystem.FileOpen(&file, "/images/pine_small.bin", LFS_O_RDONLY) < 0) {
    return false;
  }
  filesystem.FileClose(&file);
  return true;
}

void WatchFaceModernAnalog::CreateWidgets()
{
  circleWidgets[0] = new Widgets::BatteryWidgetIndicator({-WidgetRotationRadius, 0}, &batteryController);
  circleWidgets[0]->ChangeColors(0x00FFFF, 0x808080, 0x808080);
  circleWidgets[1] = new Widgets::StepsWidgetIndicator({0, WidgetRotationRadius}, &settingsController, &motionController, font_teko);
  circleWidgets[1]->ChangeColors(0x00FFFF, 0x808080, 0x808080);
  circleWidgets[2] = new Widgets::DateWidgetIndicator({WidgetRotationRadius, 0}, &dateTimeController, font_teko);
  circleWidgets[2]->ChangeColors(0x00FFFF, 0x808080, 0x808080);
}
void WatchFaceModernAnalog::RotateWidgets()
{
  
  if(leadWidgetAngle < 180)
  {
    leadWidgetAngle += 10;
    lv_coord_t x1 = (lv_coord_t)(WidgetRotationRadius*Cosine(leadWidgetAngle)/LV_TRIG_SCALE);
    lv_coord_t x2 = (lv_coord_t)(WidgetRotationRadius*Cosine(leadWidgetAngle - 90)/LV_TRIG_SCALE);
    lv_coord_t y1 = (lv_coord_t)(WidgetRotationRadius*Sine(leadWidgetAngle)/LV_TRIG_SCALE);
    lv_coord_t y2 = (lv_coord_t)(WidgetRotationRadius*Sine(leadWidgetAngle - 90)/LV_TRIG_SCALE);
    circleWidgets[0]->SetPosition({x1, y1});
    circleWidgets[1]->SetPosition({x2, y2});
  }
  else
  {
    circleWidgets[2]->SetHidden(false);
    rotateWidgets = false;
  }
  return;
}