/*
 * Copyright (c) 2017 Dmytro Shestakov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

// platform specific delay loop implementation
#include "stdint.h"
#include "clock.h"

namespace Mcudrv
{
	enum
	{	
		PlatformCyslesPerDelayLoop32 = 42,
		PlatformCyslesPerDelayLoop16 = 6,
		PlatformCyslesPerDelayLoop8 = 5
	};

//	namespace Ndelay
//	{
	#pragma inline=forced
	inline void DelayCycle32(uint32_t delayLoops)
	{
		do
		{
			// TODO: rewrite in assembly
			// __no_operation();
		}while(delayLoops--);
	}
	#pragma inline=forced
	inline void DelayCycle16(uint16_t delayLoops)
	{
		do
		{
			// TODO: rewrite in assembly
			// __no_operation();
		}while(delayLoops--);
	}
	#pragma inline=forced
	inline void DelayCycle8(uint8_t delayLoops)
	{
		do
		{
			// TODO: rewrite in assembly
			// __no_operation();
		}while(delayLoops--);
	}

// 	#pragma inline=forced
// 	template<unsigned long ns, unsigned long CpuFreq = Fcpu>
// 	void delay_ns()
// 	{
// //		const unsigned long delayLoops32 = (unsigned long)(CpuFreq / (1.0e9 * PlatformCyslesPerDelayLoop32) * ns);
// 			const unsigned long delayLoops16 = (unsigned long)(CpuFreq / (1.0e9 * PlatformCyslesPerDelayLoop16) * ns);
// 			const unsigned long delayLoops8 = (unsigned long)(CpuFreq / (1.0e9 * PlatformCyslesPerDelayLoop8) * ns);
// 			if(delayLoops16 > 0xfffful);
// 				#pragma message("Too Large Loop")
// 				//DelayCycle32(delayLoops32);
// 			else if(delayLoops8 > 0xfful)
// 				DelayCycle16((uint16_t)delayLoops16);
//  			else 
//  			{	
//  				if(delayLoops8 == 0) return;
//  				DelayCycle8((uint8_t)delayLoops8);
//  			}
// 	}

	#pragma inline=forced
	template<unsigned long us, unsigned long CpuFreq = F_CPU>
	void delay_us()
	{
		const unsigned long delayLoops32 = (unsigned long)(CpuFreq / (1.0e6 * PlatformCyslesPerDelayLoop32) * us);
			const unsigned long delayLoops16 = (unsigned long)(CpuFreq / (1.0e6 * PlatformCyslesPerDelayLoop16) * us);
			const unsigned long delayLoops8 = (unsigned long)(CpuFreq / (1.0e6 * PlatformCyslesPerDelayLoop8) * us);

			if(delayLoops16 > 0xfffful)
				DelayCycle32(delayLoops32);
			else if(delayLoops8 > 0xfful)
				DelayCycle16((uint16_t)delayLoops16);
			else 
			{	
				if(delayLoops8 == 0) return;
				DelayCycle8((uint8_t)delayLoops8);
			}
	}

//	#pragma inline=forced
//	template<unsigned long ms, unsigned long CpuFreq = Fcpu>
	void delay_ms(uint16_t n)
	{
		do
		{
			delay_us<1000>();
		} while (--n);		
	}

}
