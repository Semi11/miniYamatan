#ifndef __APP_H
#define __APP_H

/* モータ、電磁弁の個数 */ｊ
#define DD_NUM_OF_MD 5
#define DD_NUM_OF_AB 0

#define DD_USE_ENCODER1 0
#define DD_USE_ENCODER2 0
#define DD_NUM_OF_SV 0

#include "DD_RC.h"
#include "DD_MD.h"
#include "DD_SV.h"

/*NO Device mode*/
#define _NO_DEVICE 0

/* モータを反転させるか(１で反転) */
/* 駆動のモータの回転の向きを揃えるために使用 */
#define _IS_REVERSE_DRIL 0
#define _IS_REVERSE_DRIR 0
#define _IS_REVERSE_ARMT 0
#define _IS_REVERSE_ARME 0
#define _IS_REVERSE_ARMS 0

/* 各モータのインデックス */
#define ROB0_DRIL 0
#define ROB0_DRIR 1
#define ROB0_ARMT 2
#define ROB0_ARME 3
#define ROB0_ARMS 4

/* コントローラのスティックの補正値 */
#define CENTRAL_THRESHOLD 5

#define MD_MAX_DUTY 9999

#define MD_MAX_DUTY_DRIL (int)( MD_MAX_DUTY * g_adjust.DRILadjust.value / 100 )
#define MD_MAX_DUTY_DRIR (int)( MD_MAX_DUTY * g_adjust.DRIRadjust.value / 100 )
#define MD_MAX_DUTY_DRIBF (int)( MD_MAX_DUTY * g_adjust.DRIBFadjust.value / 100 )
#define MD_MAX_DUTY_DRIBB (int)( MD_MAX_DUTY * g_adjust.DRIBBadjust.value / 100 )
#define MD_MAX_DUTY_ARMT (int)( MD_MAX_DUTY * g_adjust.ARMTadjust.value / 100 )
#define MD_MAX_DUTY_ARME (int)( MD_MAX_DUTY * g_adjust.ARMEadjust.value / 100 )
#define MD_MAX_DUTY_ARMS (int)( MD_MAX_DUTY * g_adjust.ARMSadjust.value / 100 )

#define MD_GAIN_DRIL ( MD_MAX_DUTY_DRIL / DD_RC_ANALOG_MAX )
#define MD_GAIN_DRIR ( MD_MAX_DUTY_DRIR / DD_RC_ANALOG_MAX )
#define MD_GAIN_DRIBF ( MD_MAX_DUTY_DRIBF / DD_RC_ANALOG_MAX )
#define MD_GAIN_DRIBB ( MD_MAX_DUTY_DRIBB / DD_RC_ANALOG_MAX )
#define MD_GAIN_ARMT ( MD_MAX_DUTY_ARMT / DD_RC_ANALOG_MAX )
#define MD_GAIN_ARME ( MD_MAX_DUTY_ARME / DD_RC_ANALOG_MAX )
#define MD_GAIN_ARMS ( MD_MAX_DUTY_ARMS / DD_RC_ANALOG_MAX )
#define MD_GAIN ( DD_MD_MAX_DUTY / DD_RC_ANALOG_MAX )

int appTask(void);
int appInit(void);

#endif
