#include "app.h"
#include "DD_Gene.h"
#include "DD_RCDefinition.h"
#include "SystemTaskManager.h"
#include <stdlib.h>
#include "MW_GPIO.h"
#include "MW_IWDG.h"
#include "message.h"
#include "MW_flash.h"
#include "constManager.h"

/*メモ
 *g_ab_h...ABのハンドラ
 *g_md_h...MDのハンドラ
 *
 *g_rc_data...RCのデータ
 */

/* 駆動の台形制御の変化量 */
tc_slope_lim_t tc_slope_lim_dri;

/* アーム回転と上下の台形制御の変化量 */
tc_slope_lim_t tc_slope_lim_arm;

/* アームの伸縮の台形制御の変化量 */
tc_slope_lim_t tc_slope_lim_armS;

/* 台形制御の変化量の初期化 */
static
void setTCVal(void);

/*suspensionSystem*/
static
int suspensionSystem(void);

/*armSystem*/
static
int armSystem_modeA(void);

/*LEDSystem*/
static
int LEDSystem(void);


int appInit(void){

  ad_init();

  setTCVAL();

  /*GPIO の設定などでMW,GPIOではHALを叩く*/
  return EXIT_SUCCESS;
}

/*application tasks*/
int appTask(void){
  int ret=0;

  if( __RC_ISPRESSED_R1(g_rc_data) && __RC_ISPRESSED_R2(g_rc_data) &&
      __RC_ISPRESSED_L1(g_rc_data) && __RC_ISPRESSED_L2(g_rc_data)){
    while( __RC_ISPRESSED_R1(g_rc_data) || __RC_ISPRESSED_R2(g_rc_data) ||
           __RC_ISPRESSED_L1(g_rc_data) || __RC_ISPRESSED_L2(g_rc_data)){
      SY_wait(10);
    }

    ad_main();
    setTCVal();
  }

  /*それぞれの機構ごとに処理をする*/
  /*途中必ず定数回で終了すること。*/
  ret = suspensionSystem();
  if(ret){
    return ret;
  }

  ret = armSystem();
  if(ret){
    return ret;
  }

  ret = LEDSystem();
  if(ret){
    return ret;
  }
     
  return EXIT_SUCCESS;
}

/* 台形制御の変化量の初期化 */
static
void setTCVal(void){
  tc_slope_lim_dri.rising_val = g_adjust.tc_dri_rise.value;
  tc_slope_lim_dri.falling_val = g_adjust.tc_dri_fall.value;
  tc_slope_lim_arm.rising_val = g_adjust.tc_arm_rise.value;
  tc_slope_lim_arm.falling_val = g_adjust.tc_arm_fall.value;
  tc_slope_lim_armS.rising_val = g_adjust.tc_armS_rise.value;
  tc_slope_lim_armS.falling_val = g_adjust.tc_armS_fall.value;
}

/*プライベート 足回りシステム*/
static
int suspensionSystem(void){
 /* 目標値 */
  int DRIL_target_duty;
  int DRIR_target_duty;
  int DRIBF_target_duty;
  int DRIBB_target_duty;

  /* ボタンを１つ押すと、2つのモータが動作する */
  if( __RC_ISPRESSED_UP(g_rc_data)){
    DRIL_target_duty = -MD_MAX_DUTY_DRIL;
    DRIR_target_duty = -MD_MAX_DUTY_DRIR;
  }else if( __RC_ISPRESSED_DOWN(g_rc_data)){
    DRIL_target_duty = MD_MAX_DUTY_DRIL;
    DRIR_target_duty = MD_MAX_DUTY_DRIR;
  }else if( __RC_ISPRESSED_LEFT(g_rc_data)){
    DRIL_target_duty = MD_MAX_DUTY_DRIL;
    DRIR_target_duty = -MD_MAX_DUTY_DRIR;
  }else if( __RC_ISPRESSED_RIGHT(g_rc_data)){
    DRIL_target_duty = -MD_MAX_DUTY_DRIL;
    DRIR_target_duty = MD_MAX_DUTY_DRIR;
  }else{
    DRIL_target_duty = 0;
    DRIR_target_duty = 0;
  }

  control_trapezoid(&tc_slope_lim_dri, &g_md_h[ROB0_DRIL], DRIL_target_duty, _IS_REVERSE_DRIL);
  control_trapezoid(&tc_slope_lim_dri, &g_md_h[ROB0_DRIR], DRIR_target_duty, _IS_REVERSE_DRIR);

  return EXIT_SUCCESS;
}/*suspensionSystem*/

/*アームシステム*/
static
int armSystem(void){
  const int num_of_motor = ROB0_ARMT + 3; /*モータの個数*/
  int rc_analogdata; /*アナログデータ*/
  int is_reverse; /* 反転するか */
  unsigned int idx; /*インデックス*/
  unsigned int md_gain; /*アナログデータの補正値 */
  int ctl_motor_kind; /* 現在制御しているモータ */
  int target_duty; /* 目標値 */

  /*for each motor*/
  for( ctl_motor_kind = ROB0_ARMT; ctl_motor_kind < num_of_motor; ctl_motor_kind++ ){
    is_reverse = 0;

    /*それぞれの差分*/
    switch( ctl_motor_kind ){
    case ROB0_ARMT:
      rc_analogdata = DD_RCGetLX(g_rc_data);
      md_gain = MD_GAIN_ARMT;
      /* 前後の向きを反転 */
      is_reverse = _IS_REVERSE_ARMT;
      break;
    case ROB0_ARME:
      rc_analogdata = DD_RCGetLY(g_rc_data);
      md_gain = MD_GAIN_ARME;
      /* 前後の向きを反転 */
      is_reverse = _IS_REVERSE_ARME;
      break;
    case ROB0_ARMS:
      rc_analogdata = DD_RCGetRY(g_rc_data);
      md_gain = MD_GAIN_ARMS;
      /* 前後の向きを反転 */
      is_reverse = _IS_REVERSE_ARMS;
      break;
    default: return EXIT_FAILURE;
    }
    idx = ctl_motor_kind;

    /* 目標値を計算 */
    /*これは中央か?±3程度余裕を持つ必要がある。*/
    if( abs(rc_analogdata) < CENTRAL_THRESHOLD ){
      target_duty = 0;
    }else{
      target_duty = rc_analogdata * md_gain;
    }

    if( idx == ROB0_ARMS ){ /* ロボット伸縮の制御か */
      control_trapezoid(&tc_slope_lim_armS, &g_md_h[idx], target_duty, is_reverse);
    }else{
      control_trapezoid(&tc_slope_lim_arm, &g_md_h[idx], target_duty, is_reverse);
    }
  }

  return EXIT_SUCCESS;
} /* armSystem*/

static int LEDSystem(void){
  if(__RC_ISPRESSED_UP(g_rc_data)){
    g_led_mode = lmode_1;
  }
  if(__RC_ISPRESSED_DOWN(g_rc_data)){
    g_led_mode = lmode_2;
  }
  if(__RC_ISPRESSED_RIGHT(g_rc_data)){
    g_led_mode = lmode_3;
  }

  return EXIT_SUCCESS;
}

