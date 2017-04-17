/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer.c,v 1.1 2006/08/22 21:35:13 wolti Exp $
 */

/* ----------------------- Platform includes --------------------------------*/
//#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "timers.h"

using namespace Mcudrv::T2;

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit( USHORT usTim1Timerout50us )
{
  Timer2::Init(Div_128, Default);
  Timer2::WriteAutoReload(usTim1Timerout50us);
  Timer2::ClearIntFlag(IRQ_Update);
  Timer2::EnableInterrupt(IRQ_Update);
  return TRUE;
}


void vMBPortTimersEnable(  )
{
  /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
  Timer2::WriteCounter(0);
  Timer2::Enable();
}

void vMBPortTimersDisable(  )
{
  Timer2::WriteCounter(0);
  Timer2::Disable();
  /* Disable any pending timers. */
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
INTERRUPT_HANDLER(TIM2_UPD_UIF_IRQHandler, TIM2_OVR_UIF_vector - 2)
{
  Timer2::ClearIntFlag(IRQ_Update);
  (void)pxMBPortCBTimerExpired();
}

