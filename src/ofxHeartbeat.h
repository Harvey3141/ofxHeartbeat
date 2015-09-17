#pragma once
#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

#define BROADCAST_ADDRESS "192.168.1.255"
#define PORT 9010
#define NUM_MSG_STRINGS 20

struct TrackedHeartbeat {
	string id;
	float age;
	float ageLimit;
	bool coronerNotified;
}; 

class ofxHeartbeat {
public:
	void	setup (string id, float heartbeatRate, bool isReceive, bool isSend);
	void	update ();
	ofEvent<string>	_onStoppedHeart;
	ofEvent<string>	_onRestaredHeart;
	ofEvent<bool>	_onSleep; // true == sleep, false = awake

private:
	void	sendHeartbeat ();
	void	receiveHeartbeat (string id);
	void	registerHeartbeat (string id, float ageLimit);
	void	updateReceiveOSC ();
	float	_age;
	float	_heartbeatRate;
	string	_id;

	
	vector<TrackedHeartbeat>	_trackedHeartbeats;
	TrackedHeartbeat*			getTrackedHeartbeat (string id);

	// osc - sending
	void			setupSending ();
	ofxOscSender	_sender;
	int				_port;
	bool			_isReceive;

	// osc - receiving
	void			setupReceiving ();
	ofxOscReceiver	_receiver;
	int				_current_msg_string;
	string			_msg_strings[NUM_MSG_STRINGS];
	float			_timers[NUM_MSG_STRINGS];
	bool			_isSend;
};