#ifndef _MDO_PWMController_H
#define _MDO_PWMController_H

#include "Esp32LedcFactory.h"

#include <driver/ledc.h>
#include <memory>

namespace MDO {
namespace ESP32ServoController {

class LedcTimer;
class LedcChannel;

/**
 * 
 */ 
class PWMController {
	friend class PwmFactory;
	
	public:		//types
	protected:
		std::shared_ptr<LedcTimer>		m_spTimer;
		std::shared_ptr<LedcChannel>	m_spChannel;
	
	private:

	protected:
		bool							dutyToInt(double dDuty, uint32_t& uiDuty, int& iHighPoint) const;
		void							setTimer(std::shared_ptr<LedcTimer> oTimer);
		void							cleanUp();
		bool							createAndConfigureTimer(uint32_t uiFreqHz);
		bool							createAndConfigureChannel(int iPinNr, double dDuty, bool bInvertOutput);
	
	public:
		ledc_mode_t						getSpeedMode() const;
		std::shared_ptr<LedcTimer>		getTimer() const;
		std::shared_ptr<LedcChannel>	getChannel() const;
	
		bool			fade(double dDuty, 		int iMaxFadeTime_ms = 1, bool bBlocking = false);

		bool			begin(int iPinNr, const PWMController* pChannelProvider);																			//just adds a pin to an existing channel
		bool			begin(const Esp32LedcFactory& oFactory, int iPinNr, const PWMController* pTimerProvider, double dDuty, bool bInvertOutput = false);	//start sharing a timer, however uses a new channel
		bool			begin(const Esp32LedcFactory& oFactory, int iPinNr, uint32_t uiFreqHz, double dDuty, bool bInvertOutput = false);					//uses a new channel, and a new timer
		
		PWMController();
		virtual ~PWMController();
};

}	//namespace end
}	//namespace end

#endif