#ifndef _MDO_ServoFactoryDecorator_H
#define _MDO_ServoFactoryDecorator_H

#include "Esp32LedcFactory.h"

namespace MDO {
namespace ESP32ServoController {

class ServoController;

/**
 * 
 */ 
class ServoFactoryDecorator {
	
	public:		//types
	private:
		const Esp32LedcFactory&	m_oFactory;
	
	private:
	
	public:
		//virtual ledc_mode_t						getAlternativeSpeedMode() const override;
		//virtual bool							supportAlternativeSpeedMode() const override;
		//virtual ledc_mode_t						getDefaultSpeedMode() const override;
		
		virtual std::shared_ptr<LedcTimer>		createTimer(uint8_t uiResolutionBits = 0) const;
		virtual std::shared_ptr<LedcChannel>	createChannel(int iPinNr, ServoController* pServoController, double dDuty, bool bInvertOutput = false) const;	

		ServoFactoryDecorator(const Esp32LedcFactory& oFactory);
		virtual ~ServoFactoryDecorator();
};

}	//namespace end
}	//namespace end

#endif