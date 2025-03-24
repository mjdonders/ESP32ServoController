#include "Esp32LedcRegistry.h"


//#include "debug_config.h"
#include "DebugMsg.h"
//#include <mdomisc.h>

#include "LedcTimer.h"
#include "LedcChannel.h"
#include "ServoController.h"


namespace MDO {
namespace ESP32ServoController {

Esp32LedcRegistry* Esp32LedcRegistry::m_pSingleton = nullptr;

/**
 * Searches through our registry to see if there already is a timer for the provided frequency and speed mode
 * Providing no speed mode means 'search all options' (from a speed mode point of view)
 * return either a timer pointer when a hit is found, or nullptr
 */
//const LedcTimer* Esp32LedcRegistry::hasTimerFor(uint32_t uiFreqHz, ledc_mode_t eSpeedMode /*= LEDC_SPEED_MODE_MAX*/) const {
/*	const LedcTimer* pTimerFound = nullptr;
#if SOC_LEDC_SUPPORT_HS_MODE
	if ((eSpeedMode == LEDC_SPEED_MODE_MAX) || (eSpeedMode == LEDC_HIGH_SPEED_MODE)) {
		for (auto cit = m_sPwmControllers.sHighSpeed.mTimers.begin(); cit != m_sPwmControllers.sHighSpeed.mTimers.end(); cit++) {
			if ((cit->second != nullptr) && (cit->second->getFrequency() == uiFreqHz)) {
				pTimerFound = cit->second;
				return pTimerFound;
			}
		}
	}
#endif	

	if ((eSpeedMode == LEDC_SPEED_MODE_MAX) || (eSpeedMode == LEDC_LOW_SPEED_MODE)) {
		for (auto cit = m_sPwmControllers.sLowSpeed.mTimers.begin(); cit != m_sPwmControllers.sLowSpeed.mTimers.end(); cit++) {
			if ((cit->second != nullptr) && (cit->second->getFrequency() == uiFreqHz)) {
				pTimerFound = cit->second;
				return pTimerFound;
			}
		}		
	}	
	
	return pTimerFound;
}*/

const ServoController* Esp32LedcRegistry::getServoUsing(uint32_t uiFreqHz, ledc_mode_t eSpeedMode) const {
	
	//MDO_SERVO_DEBUG_PRINTLN(String("Esp32LedcRegistry::getServoUsing - ") + uiFreqHz + ", " + eSpeedMode);
	
	const servos_t* pMap = nullptr;
	const ServoController* pServoFound = nullptr;
	
	if (eSpeedMode == LEDC_LOW_SPEED_MODE) {
		pMap = &m_sServoControllers.sLowSpeed;
	} else {
		pMap = &m_sServoControllers.sHighSpeed;
	}
	
	for (auto cit = pMap->begin(); cit != pMap->end(); cit++) {
		if ((cit->second != nullptr) && (cit->second->getTimerFreqHz() == uiFreqHz)) {
			pServoFound = cit->second;
			MDO_SERVO_DEBUG_PRINTLN("  found matching servo instance");
			return pServoFound;
		}
	}
	MDO_SERVO_DEBUG_PRINTLN("  no matching servo found");
	return pServoFound;
}

bool Esp32LedcRegistry::registerTimer(const LedcTimer* pTimer) {
	if ((pTimer == nullptr) || (isTimerInUse(pTimer))) {
		return false;
	}
	if (pTimer->getSpeedMode() == LEDC_LOW_SPEED_MODE) {
		//MDO_SERVO_DEBUG_PRINTLN("Registering low speed timer");
		m_sPwmControllers.sLowSpeed.mTimers[pTimer->getTimerNr()] = pTimer;
	} else {
		//MDO_SERVO_DEBUG_PRINTLN("Registering high speed timer");
		m_sPwmControllers.sHighSpeed.mTimers[pTimer->getTimerNr()] = pTimer;
	}
	return true;
}

/**
 * unregisters a timer.
 */
void Esp32LedcRegistry::unregisterTimer(const LedcTimer* pTimer) {
	if (pTimer == nullptr) {
		return;
	}
	if (pTimer->getSpeedMode() == LEDC_LOW_SPEED_MODE) {
		auto it = m_sPwmControllers.sLowSpeed.mTimers.find(pTimer->getTimerNr());
		if (it != m_sPwmControllers.sLowSpeed.mTimers.end()) {
			//MDO_SERVO_DEBUG_PRINTLN("Unregistering low speed timer");
			it->second = nullptr;	//ensure the pointer does not get deleted
			m_sPwmControllers.sLowSpeed.mTimers.erase(it);
		}
	} else {
		auto it = m_sPwmControllers.sHighSpeed.mTimers.find(pTimer->getTimerNr());
		if (it != m_sPwmControllers.sHighSpeed.mTimers.end()) {
			//MDO_SERVO_DEBUG_PRINTLN("Unregistering high speed timer");
			it->second = nullptr;	//ensure the pointer does not get deleted
			m_sPwmControllers.sHighSpeed.mTimers.erase(it);
		}
	}
}

bool Esp32LedcRegistry::registerChannel(const LedcChannel* pChannel) {
	if ((pChannel == nullptr) || (isChannelInUse(pChannel))) {
		return false;
	}
	if (pChannel->getSpeedMode() == LEDC_LOW_SPEED_MODE) {
		m_sPwmControllers.sLowSpeed.mChannels[pChannel->getChannelNr()] = pChannel;
	} else {
		m_sPwmControllers.sHighSpeed.mChannels[pChannel->getChannelNr()] = pChannel;
	}
	return true;
}

/**
 * unregisters a channel.
 */
void Esp32LedcRegistry::unregisterChannel(const LedcChannel* pChannel) {
	if (pChannel == nullptr) {
		MDO_SERVO_DEBUG_PRINTLN("Ignore unregisterChannel");
		return;
	}
	if (pChannel->getSpeedMode() == LEDC_LOW_SPEED_MODE) {
		auto it = m_sPwmControllers.sLowSpeed.mChannels.find(pChannel->getChannelNr());
		if (it != m_sPwmControllers.sLowSpeed.mChannels.end()) {
			it->second = nullptr;	//ensure the pointer does not get deleted
			m_sPwmControllers.sLowSpeed.mChannels.erase(it);
		}
	} else {
		auto it = m_sPwmControllers.sHighSpeed.mChannels.find(pChannel->getChannelNr());
		if (it != m_sPwmControllers.sHighSpeed.mChannels.end()) {
			it->second = nullptr;	//ensure the pointer does not get deleted
			m_sPwmControllers.sHighSpeed.mChannels.erase(it);
		}
	}
}

bool Esp32LedcRegistry::registerServo(const ServoController* pServo) {
	if ((pServo == nullptr) || (pServo->getId() == 0xFF) || (isServoInUse(pServo))) {
		MDO_SERVO_DEBUG_PRINTLN("Ignore registerServo");
		return false;
	}
	if (pServo->getSpeedMode() == LEDC_LOW_SPEED_MODE) {
		m_sServoControllers.sLowSpeed[pServo->getId()] = pServo;
	} else {
		m_sServoControllers.sHighSpeed[pServo->getId()] = pServo;
	}
	MDO_SERVO_DEBUG_PRINTLN("Servo registered");
	return true;
}

void Esp32LedcRegistry::unregisterServo(const ServoController* pServo) {
	if ((pServo == nullptr) || (pServo->getId() == 0xFF)) {
		//this does not always indicate an error, so just ignore
		return;
	}
	if (pServo->getSpeedMode() == LEDC_LOW_SPEED_MODE) {
		auto it = m_sServoControllers.sLowSpeed.find(pServo->getId());
		if (it != m_sServoControllers.sLowSpeed.end()) {
			it->second = nullptr;	//ensure the pointer does not get deleted
			m_sServoControllers.sLowSpeed.erase(it);
		}
	} else {
		auto it = m_sServoControllers.sHighSpeed.find(pServo->getId());
		if (it != m_sServoControllers.sHighSpeed.end()) {
			it->second = nullptr;	//ensure the pointer does not get deleted
			m_sServoControllers.sHighSpeed.erase(it);
		}
	}	
}

void Esp32LedcRegistry::setHardwareFadeEnabled() {
	m_bHardwareFadeEnabled = true;
}

/**
 * When a timer is already in use, and the hardware only supports one clock source: the new clock source is already known ;-)
 * in this scenario, will set uiClockSourceOutput to the relevant value
 */
/*	not implemented yet.. Maybe later. Since ledc error messages are quite clear, this might not be needed.
bool Esp32LedcRegistry::isClockSourceFixed(uint8_t& uiClockSourceOutput) const {
	bool bIsClockSourceFixed =	(getNrOfSimultaneousClockSources() == 1) && 
								(getCurrentNrOfTimersInUse() > 0);
	if (bIsClockSourceFixed) {
		//set uiClockSourceOutput
	}
	
	return bIsClockSourceFixed;
}*/

//gives an answer globally (when eSpeedMode is set to LEDC_SPEED_MODE_MAX), or locally LEDC_LOW_SPEED_MODE / LEDC_HIGH_SPEED_MODE when requested
uint8_t Esp32LedcRegistry::getCurrentNrOfTimersInUse(ledc_mode_t eSpeedMode /*= LEDC_SPEED_MODE_MAX*/) const {
	uint8_t uiCount = 0;
	if (eSpeedMode != LEDC_LOW_SPEED_MODE) {
		uiCount += m_sPwmControllers.sHighSpeed.mTimers.size();
	}
	if ((eSpeedMode == LEDC_LOW_SPEED_MODE) || (eSpeedMode == LEDC_SPEED_MODE_MAX)) {
		uiCount += m_sPwmControllers.sLowSpeed.mTimers.size();
	}
	return uiCount;
}

/**
 * Get first available timer for the provided speed mode.
 * Speed mode cannot be LEDC_SPEED_MODE_MAX
 * Returns either the timer number available or LEDC_TIMER_MAX when nothing is available anymore
 */
uint8_t Esp32LedcRegistry::getFirstAvailableTimer(ledc_mode_t eSpeedMode) const {
	uint8_t iFirstAvailableTimerNr = LEDC_TIMER_MAX;
	
#if SOC_LEDC_SUPPORT_HS_MODE
	//so now we can support high speed and low speed
	//'max' in not that, so that's wrong
	if (eSpeedMode != LEDC_SPEED_MODE_MAX) {	
#else
	//now we only support low speed
	if (eSpeedMode == LEDC_LOW_SPEED_MODE) {
#endif

		const uint8_t uiMaxNrOfTimers = (eSpeedMode == LEDC_LOW_SPEED_MODE) ? getNrOfLowSpeedTimers() : getNrOfHighSpeedTimers();
		for (uint8_t i=0; i< uiMaxNrOfTimers; i++) {
			if (!isTimerInUse(i, eSpeedMode)) {
				iFirstAvailableTimerNr = i;
				return iFirstAvailableTimerNr;
			}			
		}
	}

	return iFirstAvailableTimerNr;
}

uint8_t Esp32LedcRegistry::getFirstAvailableChannel(ledc_mode_t eSpeedMode) const {
	uint8_t iFirstAvailableChannelNr = LEDC_CHANNEL_MAX;
	
#if SOC_LEDC_SUPPORT_HS_MODE
	//so now we can support high speed and low speed
	//'max' in not that, so that's wrong
	if (eSpeedMode != LEDC_SPEED_MODE_MAX) {	
#else
	//now we only support low speed
	if (eSpeedMode == LEDC_LOW_SPEED_MODE) {
#endif

		const uint8_t uiMaxNrOfChannels = getNrOfChannels();	//no distinction between low speed and high speed here
		for (uint8_t i=0; i< uiMaxNrOfChannels; i++) {
			if (!isChannelInUse(i, eSpeedMode)) {
				iFirstAvailableChannelNr = i;
				return iFirstAvailableChannelNr;
			}			
		}
	}

	return iFirstAvailableChannelNr;
}

bool Esp32LedcRegistry::isTimerInUse(uint8_t uiTimerNr, ledc_mode_t eSpeedMode) const {
	if (eSpeedMode == LEDC_LOW_SPEED_MODE) {
		return m_sPwmControllers.sLowSpeed.mTimers.find(uiTimerNr)  != m_sPwmControllers.sLowSpeed.mTimers.end();
	} else {
		return m_sPwmControllers.sHighSpeed.mTimers.find(uiTimerNr) != m_sPwmControllers.sHighSpeed.mTimers.end();
	}
}

bool Esp32LedcRegistry::isTimerInUse(const LedcTimer* pTimer) const {
	if (pTimer == 0) {
		MDO_SERVO_DEBUG_PRINTLN("Esp32LedcRegistry::isTimerInUse - parameter error");
		return true;
	}
	return isTimerInUse(pTimer->getTimerNr(), pTimer->getSpeedMode());
}

bool Esp32LedcRegistry::isChannelInUse(uint8_t uiChannelNr, ledc_mode_t eSpeedMode) const {
	if (eSpeedMode == LEDC_LOW_SPEED_MODE) {
		return m_sPwmControllers.sLowSpeed.mChannels.find(uiChannelNr)  != m_sPwmControllers.sLowSpeed.mChannels.end();
	} else {
		return m_sPwmControllers.sHighSpeed.mChannels.find(uiChannelNr) != m_sPwmControllers.sHighSpeed.mChannels.end();
	}
}

bool Esp32LedcRegistry::isChannelInUse(const LedcChannel* pChannel) const {
	if (pChannel == nullptr) {
		MDO_SERVO_DEBUG_PRINTLN("Esp32LedcRegistry::isChannelInUse - parameter error");
		return true;
	}
	return isChannelInUse(pChannel->getChannelNr(), pChannel->getSpeedMode());
}

bool Esp32LedcRegistry::isServoInUse(uint8_t uiId, ledc_mode_t eSpeedMode) const {
	if (eSpeedMode == LEDC_LOW_SPEED_MODE) {
		return m_sServoControllers.sLowSpeed.find(uiId)  != m_sServoControllers.sLowSpeed.end();
	} else {
		return m_sServoControllers.sHighSpeed.find(uiId) != m_sServoControllers.sHighSpeed.end();
	}	
}

bool Esp32LedcRegistry::isServoInUse(const ServoController* pServo) const {
	if (pServo == nullptr) {
		MDO_SERVO_DEBUG_PRINTLN("Esp32LedcRegistry::isServoInUse - parameter error");
		return true;
	}
	return isServoInUse(pServo->getId(), pServo->getSpeedMode());
}

bool Esp32LedcRegistry::isHardwareFadeEnabled() const {
	return m_bHardwareFadeEnabled;
}

uint32_t Esp32LedcRegistry::getServoFrequency() const {
	return m_uiServoFreqHz;
}

//in usec
uint32_t Esp32LedcRegistry::getServoMinPosTime() const {
	return m_uiServoMinPos_usec;
}

//in usec
uint32_t Esp32LedcRegistry::getServoMaxPosTime() const {
	return m_uiServoMaxPos_usec;
}

uint8_t Esp32LedcRegistry::getNrOfHighSpeedTimers() const {
	return m_uiNrOfHighSpeedTimers;
}

uint8_t Esp32LedcRegistry::getNrOfLowSpeedTimers() const {
	return m_uiNrOfLowSpeedTimers;
}

uint8_t Esp32LedcRegistry::getNrOfSimultaneousClockSources() const {
	return m_uiNrOfSimultaneousClockSources;
}

uint8_t Esp32LedcRegistry::getNrOfChannels() const {
	return m_uiNrOfChannels;
}

uint8_t Esp32LedcRegistry::getMaxTimerResolutionBits() const {
	return m_uiMaxTimerResolutionBits;
}

bool Esp32LedcRegistry::setServoParams(uint32_t uiMinPos_usec /*= 1000*/, uint32_t uiMaxPos_usec /*= 2000*/, uint32_t uiFreqHz /*= 50*/, bool bLimitCheck /*= true*/) {
	bool bOk = true;
	
	if ((bLimitCheck) && (uiFreqHz >= 40) && (uiFreqHz <= 200)) {	//values from wikipedia
		m_uiServoFreqHz = uiFreqHz;
	} else {
		bOk = false;
	}
	if ((bLimitCheck) && (m_uiServoMinPos_usec < m_uiServoMaxPos_usec)) {
		//could also limit these based on m_uiServoFreqHz. maybe later
		m_uiServoMinPos_usec	= uiMinPos_usec;
		m_uiServoMaxPos_usec	= uiMaxPos_usec;
	} else {
		bOk = false;
	}
	
	if (!bOk) {
		MDO_SERVO_DEBUG_PRINTLN("WARNING - Esp32LedcRegistry::setServoParams - ignoring parameters based on failed checks");
	}
	return bOk;
}

void Esp32LedcRegistry::begin(uint8_t uiNrOfHighSpeedTimers, uint8_t uiNrOfLowSpeedTimers, uint8_t uiNrOfSimultaneousClockSources, uint8_t uiNrOfChannels, uint8_t uiMaxTimerResolutionBits) {
	m_uiNrOfHighSpeedTimers			= uiNrOfHighSpeedTimers;
	m_uiNrOfLowSpeedTimers			= uiNrOfLowSpeedTimers;
	m_uiNrOfSimultaneousClockSources= uiNrOfSimultaneousClockSources;
	m_uiNrOfChannels				= uiNrOfChannels;
	m_uiMaxTimerResolutionBits		= uiMaxTimerResolutionBits;
}

/*static*/ Esp32LedcRegistry* Esp32LedcRegistry::instance() {
	if (m_pSingleton == nullptr) {
		m_pSingleton = new Esp32LedcRegistry();
	}
	
	return m_pSingleton;
}

Esp32LedcRegistry::Esp32LedcRegistry() {
	m_bHardwareFadeEnabled = false;
	
	m_uiNrOfHighSpeedTimers			= 0;
	m_uiNrOfLowSpeedTimers			= 0;
	m_uiNrOfSimultaneousClockSources= 0;
	m_uiNrOfChannels				= 0;
	m_uiMaxTimerResolutionBits		= 0;
	
	m_uiServoFreqHz = 50;
	m_uiServoMinPos_usec = 1000;
	m_uiServoMaxPos_usec = 2000;
	
}

Esp32LedcRegistry::~Esp32LedcRegistry() {
}

}	//namespace end
}	//namespace end