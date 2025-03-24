#include "PWMController.h"


//#include "debug_config.h"
#include "DebugMsg.h"
//#include <mdomisc.h>
#include "Esp32LedcRegistry.h"
#include "LedcChannelHighSpeed.h"
#include "LedcChannelLowSpeed.h"
#include "LedcTimerHighSpeed.h"
#include "LedcTimerLowSpeed.h"


namespace MDO {
namespace ESP32ServoController {

/**
 * Protected: Convert a duty (double, percentage from 0 to 1) to the relevant integer values for a channel, based on our timer
 */
bool PWMController::dutyToInt(double dDuty, uint32_t& uiDuty, int& iHighPoint) const {
	if (m_spTimer == nullptr) {
		return false;
	}
	if ((dDuty < 0.0) || (dDuty > 1.0)) {
		MDO_SERVO_DEBUG_PRINTLN(String("Invalid duty cycle parameter: ") + dDuty);
		return false;
	}
	
	uint32_t uiMaxResolutionValue = m_spTimer->getMaxResolutionValue();
	uiDuty = (uint32_t)std::round(uiMaxResolutionValue * dDuty);
	iHighPoint = uiMaxResolutionValue-1;	//the '-1' is a hardware limit which is checked in LedcChannel as well
	
	//and now for some final annoying rounding cases
	if (dDuty == 0.0) {
		//MDO_SERVO_DEBUG_PRINTLN("PWMController::dutyToInt - fixing 0% rounding");
		uiDuty = 0;	
	} else if (dDuty == 1.0) {
		uiDuty = uiMaxResolutionValue;	
	}	
	//MDO_SERVO_DEBUG_PRINTLN(String("dutyToInt: ") + uiMaxResolutionValue + ", " + uiDuty + ", " + iHighPoint);
	return true;
}

//protected
void PWMController::setTimer(std::shared_ptr<LedcTimer> oTimer) {
	m_spTimer = oTimer;
}

//protected
void PWMController::cleanUp() {
	if (m_spChannel != nullptr) {	//it's advised to start cleaning with the channel
		m_spChannel.reset();
	}
	
	if (m_spTimer != nullptr) {
		m_spTimer.reset();
	}	
}

ledc_mode_t PWMController::getSpeedMode() const {
	const LedcTimer* pTimer(m_spTimer.get());
	if (pTimer != nullptr) {
		return pTimer->getSpeedMode();
	}
	MDO_SERVO_DEBUG_PRINTLN("PWMController::getSpeedMode - initialisation issue (?)");
	return LEDC_SPEED_MODE_MAX;
}

std::shared_ptr<LedcTimer> PWMController::getTimer() const {
	return m_spTimer;
} 

std::shared_ptr<LedcChannel> PWMController::getChannel() const {
	return m_spChannel;
}

/**
 * Fades to a duty as number, [0, (2**duty_resolution)]. Might be more precise compared to the easier-to-use double in the other fade method.
 * note that the time accuracy can be quite low, depending on the frequency and such. See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#_CPPv423ledc_set_fade_with_time11ledc_mode_t14ledc_channel_t8uint32_ti
 * bBlocking can be used to make this a blocking method, or not. The controller will not allow a new fade command within the provided iMaxFadeTime_ms.
 */
//bool PWMController::fade(uint32_t uiDuty, int iMaxFadeTime_ms /*=1*/, bool bBlocking /* = false*/) {
/*	if (m_spChannel == nullptr) {
		return false;
	}

	return m_spChannel->fade(uiDuty, iMaxFadeTime_ms, bBlocking);
}*/

/**
 * Fades to a duty cycle 'percentage' (from 0 to 1) in the required time
 * note that the time accuracy can be quite low, depending on the frequency and such. See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html#_CPPv423ledc_set_fade_with_time11ledc_mode_t14ledc_channel_t8uint32_ti
 * bBlocking can be used to make this a blocking method, or not. The controller will not allow a new fade command within the provided iMaxFadeTime_ms.
 */
bool PWMController::fade(double dDuty, int iMaxFadeTime_ms /*=1*/, bool bBlocking /* = false*/) {
	
	if (m_spChannel == nullptr) {
		return false;
	}
	uint32_t uiDuty = 0;
	int iHighPoint = 0;
	dutyToInt(dDuty, uiDuty, iHighPoint);
	return m_spChannel->fade(uiDuty, iMaxFadeTime_ms, bBlocking);
	//m_pChannel->updateDuty(uiDuty, iHighPoint);
}

//void PWMController::updateDuty(double dDuty) {
//}

//just adds a pin to an existing channel
//keeping this PWMController instance practically empty
bool PWMController::begin(int iPinNr, const PWMController* pChannelProvider) {
	//m_bTimerIsMine = false;
	//m_bChannelIsMine = false;
	m_spTimer = pChannelProvider->getTimer();
	m_spChannel = pChannelProvider->getChannel();
	
	bool bOk = (m_spTimer != nullptr) && (m_spChannel != nullptr);
	if (bOk) {
		bOk = m_spChannel->addPin(iPinNr);
	}
	
	return bOk;
}	

bool PWMController::begin(const Esp32LedcFactory& oFactory, int iPinNr, const PWMController* pTimerProvider, double dDuty, bool bInvertOutput /*= false*/) {
	cleanUp();	//just in case

	if ((pTimerProvider == nullptr) || 
		(pTimerProvider->getTimer() == nullptr)) {
		MDO_SERVO_DEBUG_PRINTLN("Invalid timer provider");
		return false;
	}
	if ((dDuty < 0.0) || (dDuty > 1.0)) {
		MDO_SERVO_DEBUG_PRINTLN("Invalid duty cycle parameter");
		return false;
	}
	
	setTimer(pTimerProvider->getTimer());
	if (m_spTimer != nullptr) {
		uint32_t uiDuty = 0;
		int iHighPoint = 0;
		dutyToInt(dDuty, uiDuty, iHighPoint);
		
		m_spChannel = oFactory.createChannel(iPinNr, m_spTimer.get(), uiDuty, iHighPoint, bInvertOutput);		
	}

	bool bOk = (m_spTimer != nullptr) && (m_spChannel != nullptr);

	if (!bOk) {
		cleanUp();
		MDO_SERVO_DEBUG_PRINTLN("PWMController::begin - failed to allocate/configure a timer and/or channel");
	}

	return bOk;
}

//uses a new channel, and maybe a new timer
bool PWMController::begin(const Esp32LedcFactory& oFactory, int iPinNr, uint32_t uiFreqHz, double dDuty, bool bInvertOutput /*= false*/) {
	//MDO_SERVO_DEBUG_PRINTLN("PWMController::begin");
	
	cleanUp();	//just in case

	if ((dDuty < 0.0) || (dDuty > 1.0)) {
		MDO_SERVO_DEBUG_PRINTLN("Invalid duty cycle parameter");
		return false;
	}

	setTimer(oFactory.createTimer(uiFreqHz, 0));
	if (m_spTimer != nullptr) {
		uint32_t uiDuty = 0;
		int iHighPoint = 0;
		dutyToInt(dDuty, uiDuty, iHighPoint);
		
		m_spChannel = oFactory.createChannel(iPinNr, m_spTimer.get(), uiDuty, iHighPoint, bInvertOutput);
	}
	
	bool bOk = (m_spTimer != nullptr) && (m_spChannel != nullptr);

	if (!bOk) {
		cleanUp();
		MDO_SERVO_DEBUG_PRINTLN("PWMController::begin - failed to allocate/configure a timer and/or channel");
	}

	return bOk;
}

PWMController::PWMController() {	//ledc_mode_t eSpeedMode) {
	//m_eSpeedMode = eSpeedMode;
}

PWMController::~PWMController() {
	cleanUp();
}

}	//namespace end
}	//namespace end