#ifndef _MDO_LedcChannelLowSpeed_H
#define _MDO_LedcChannelLowSpeed_H

#include "LedcChannel.h"

namespace MDO {
namespace ESP32ServoController {

/**
 * 
 */ 
class LedcChannelLowSpeed: public LedcChannel {
	
	private:
	
	private:
		ledc_mode_t			getSpeedModePrivate() const;
	
	public:
		virtual bool		begin(int iPinNr, LedcTimer* pTimer, uint32_t uiDuty, int iHighPoint, bool bInvertOutput = false);
	
		LedcChannelLowSpeed(uint8_t uiChannelNr);
		virtual ~LedcChannelLowSpeed();
};

}	//namespace end
}	//namespace end

#endif