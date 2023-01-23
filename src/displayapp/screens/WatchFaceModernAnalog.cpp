#include "displayapp/screens/WatchFaceModernAnalog.h"
#include <cmath>
#include <lvgl/lvgl.h>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/NotificationIcon.h"
#include "components/settings/Settings.h"
#include "components/motion/MotionController.h"
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

WatchFaceModernAnalog::WatchFaceModernAnalog(Pinetime::Applications::DisplayApp* app,
                                            Controllers::DateTime& dateTimeController,
                                            Controllers::Battery& batteryController,
                                            Controllers::Ble& bleController,
                                            Controllers::NotificationManager& notificationManager,
                                            Controllers::Settings& settingsController,
                                            Controllers::MotionController& motionController,
                                            Controllers::FS& filesystem)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificationManager {notificationManager},
    settingsController {settingsController},
    motionController {motionController} {

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

  lv_coord_t x1, x2, y1, y2;
  for(int i = 0; i < 51; i++)
  {
    tick_lines[i] = lv_line_create(lv_scr_act(), nullptr);
    lv_style_init(&(tick_lines_style[i]));
    lv_style_set_line_width(&(tick_lines_style[i]), LV_STATE_DEFAULT, (i%5 == 0 ? 4:2));
    lv_style_set_line_color(&(tick_lines_style[i]), LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_add_style(tick_lines[i], LV_LINE_PART_MAIN, &(tick_lines_style[i]));
    x1 = (lv_coord_t)(115*cos(PI/180*(i*6+300))+120);
    x2 = (lv_coord_t)((i%5==0?105:110)*cos(PI/180*(i*6+300))+120);
    y1 = (lv_coord_t)(115*sin(PI/180*(i*6+300))+120);
    y2 = (lv_coord_t)((i%5==0?105:110)*sin(PI/180*(i*6+300))+120);
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

  // Battery Indicator
  batteryArc = lv_arc_create(lv_scr_act(), nullptr);
  batteryArc->user_data = this;  
  lv_arc_set_bg_angles(batteryArc, 180, 540);
  lv_arc_set_range(batteryArc, 0, 100);
  lv_arc_set_adjustable(batteryArc, false);
  lv_obj_set_width(batteryArc, INDICATOR_ARC_SIZE);
  lv_obj_set_height(batteryArc, INDICATOR_ARC_SIZE);
  lv_obj_align(batteryArc, NULL, LV_ALIGN_CENTER, -lv_obj_get_width(lv_scr_act())/4, 0);
  lv_obj_set_style_local_line_color(batteryArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  lv_obj_set_style_local_line_width(batteryArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(batteryArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_opa(batteryArc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_click(batteryArc, false);

  batteryIcon.Create(lv_scr_act());
  batteryIcon.SetColor(lv_color_hex(0x999999));
  lv_obj_align(batteryIcon.GetObject(), batteryArc, LV_ALIGN_CENTER, 0, 0);

  chargeIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(chargeIcon, Symbols::lightning);
  lv_obj_set_style_local_text_color(chargeIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_obj_align(chargeIcon, batteryArc, LV_ALIGN_CENTER, 0, 0);

  // Step Indicator
  stepsArc = lv_arc_create(lv_scr_act(), nullptr);
  stepsArc->user_data = this;  
  lv_arc_set_bg_angles(stepsArc, 180, 540);
  lv_arc_set_range(stepsArc, 0, settingsController.GetStepsGoal());
  lv_arc_set_adjustable(stepsArc, false);
  lv_obj_set_width(stepsArc, INDICATOR_ARC_SIZE);
  lv_obj_set_height(stepsArc, INDICATOR_ARC_SIZE);
  lv_obj_align(stepsArc, NULL, LV_ALIGN_CENTER, 0, lv_obj_get_height(lv_scr_act())/4);
  lv_obj_set_style_local_line_color(stepsArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  lv_obj_set_style_local_line_width(stepsArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(stepsArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_opa(stepsArc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_click(stepsArc, false);

  stepsIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(stepsIcon, Symbols::shoe);
  lv_obj_align(stepsIcon, stepsArc, LV_ALIGN_CENTER, 0, 10);
  stepsValue = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(stepsIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_set_style_local_text_color(stepsValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_set_style_local_text_font(stepsValue, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
  lv_obj_align(stepsValue, stepsArc, LV_ALIGN_CENTER, 0, -10);
  lv_label_set_text(stepsValue, "0");

  // Date Indicator
  dateArc = lv_arc_create(lv_scr_act(), nullptr);
  dateArc->user_data = this;  
  lv_arc_set_bg_angles(dateArc, 180, 540);
  lv_arc_set_range(dateArc, 0, 1);
  lv_arc_set_value(dateArc, 1);
  lv_arc_set_adjustable(dateArc, false);
  lv_obj_set_width(dateArc, INDICATOR_ARC_SIZE);
  lv_obj_set_height(dateArc, INDICATOR_ARC_SIZE);
  lv_obj_align(dateArc, NULL, LV_ALIGN_CENTER, lv_obj_get_width(lv_scr_act())/4, 0);
  lv_obj_set_style_local_line_color(dateArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  lv_obj_set_style_local_line_width(dateArc, LV_ARC_PART_INDIC, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_width(dateArc, LV_ARC_PART_BG, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_line_opa(dateArc, LV_ARC_PART_KNOB, LV_STATE_DEFAULT, LV_OPA_0);
  lv_obj_set_click(dateArc, false);

  label_date_day = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(label_date_day, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_set_style_local_text_font(label_date_day, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, font_teko);
  lv_label_set_text_fmt(label_date_day, "%s\n%02i", dateTimeController.DayOfWeekShortToStringLow(), dateTimeController.Day());
  lv_label_set_align(label_date_day, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label_date_day, dateArc, LV_ALIGN_CENTER, 0, 0);


  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(bleIcon, Symbols::bluetooth);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x999999));
  lv_obj_align(bleIcon, nullptr, LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  notificationIcon = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  label_hour_minute = lv_label_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_text_color(label_hour_minute, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_CYAN);
  int hour = settingsController.GetClockType() == Controllers::Settings::ClockType::H12 ? dateTimeController.Hours()%12 : dateTimeController.Hours(); 
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
    uint8_t hourText = settingsController.GetClockType() == Controllers::Settings::ClockType::H12 ? hour%12 : hour; 
    lv_label_set_text_fmt(label_hour_minute, "%d:%02d", hourText, minute);
  }

  if (sSecond != second) {
    sSecond = second;
    auto const angle = second * 6;

    second_point[0] = CoordinateRelocate(-20, angle);
    second_point[1] = CoordinateRelocate(SecondLength, angle);
    lv_line_set_points(second_body, second_point, 2);
  }
}

void WatchFaceModernAnalog::SetBatteryIndicator() {
  batteryPercentRemaining = batteryController.PercentRemaining();
  if(batteryPercentRemaining.IsUpdated()) {
    lv_arc_set_value(batteryArc, batteryPercentRemaining.Get());
  }
  isCharging = batteryController.IsCharging();
  if(isCharging.Get()){
    batteryIcon.SetBatteryPercentage(100);
    lv_obj_set_hidden(chargeIcon, false);
  }
  else{
    batteryIcon.SetBatteryPercentage(batteryPercentRemaining.Get());
    lv_obj_set_hidden(chargeIcon, true);
  }
}

void WatchFaceModernAnalog::Refresh() {
  SetBatteryIndicator();

  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(stepsValue, "%lu", stepCount.Get());
    lv_obj_align(stepsValue, stepsArc, LV_ALIGN_CENTER, 0, -10);
    lv_arc_set_value(stepsArc, stepCount.Get());
  }

  bleState = bleController.IsConnected();
  if(bleState.IsUpdated()){
    if(bleState.Get()){
      lv_obj_set_hidden(bleIcon, false);
    }else{
      lv_obj_set_hidden(bleIcon, true);
    }
  }

  notificationState = notificationManager.AreNewNotificationsAvailable();

  if (notificationState.IsUpdated()) {
    lv_label_set_text_static(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  currentDateTime = dateTimeController.CurrentDateTime();

  if (currentDateTime.IsUpdated()) {
    Pinetime::Controllers::DateTime::Months month = dateTimeController.Month();
    uint8_t day = dateTimeController.Day();
    Pinetime::Controllers::DateTime::Days dayOfWeek = dateTimeController.DayOfWeek();

    UpdateClock();
    if ((month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      lv_label_set_text_fmt(label_date_day, "%s\n%02i", dateTimeController.DayOfWeekShortToStringLow(), day);

      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
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