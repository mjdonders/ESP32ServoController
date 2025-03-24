#ifndef _MDO_LedcTimerHighSpeed_H
#define _MDO_LedcTimerHighSpeed_H

#include "LedcTimer.h"

namespace MDO {
namespace ESP32ServoController {

/**
 * 
 */ 
class LedcTimerHighSpeed: public LedcTimer {
	
	public:	//types
		enum LedcTimerHighSpeed_Source_t {	//see enum ledc_clk_src_t
#ifdef SOC_LEDC_SUPPORT_REF_TICK	//MDO, only in this case LEDC_REF_TICK is defined
			FAST_CLOCK_SOURCE_REF_TICK = LEDC_REF_TICK,	//LEDC timer clock divided from reference tick (1Mhz)  
#endif
			FAST_CLOCK_SOURCE_APB_CLK  = LEDC_APB_CLK	//LEDC timer clock divided from APB clock (80Mhz) 
		};
		
	private:
		enum LedcTimerHighSpeed_Source_t	m_eClockSource;
	
	private:
		ledc_mode_t			getSpeedModePrivate() const;
	
	public:
		virtual uint8_t		getClockSource() const;		
		virtual bool		begin(uint32_t uiFreqHz, uint8_t uiResolutionBits = 0);
	
		LedcTimerHighSpeed(uint8_t uiTimerNr, enum LedcTimerHighSpeed_Source_t eClockSource = FAST_CLOCK_SOURCE_APB_CLK);
	private:
		LedcTimerHighSpeed(const LedcTimerHighSpeed& oSrc);	//not implemented
	public:
		virtual ~LedcTimerHighSpeed();
};

}	//namespace end
}	//namespace end

#endif