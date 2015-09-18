#pragma once
#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

#define NUM_MSG_STRINGS 20

struct TrackedHeartbeat {
	string id;
	float age;
	float ageLimit;
	bool coronerNotified;
}; 

class ofxHeartbeat {
public:
	void	setup (string id, float heartbeatRate, bool isSend, bool isReceive);
	void	update ();
	void	load (string path);
	void	sendSleepMessage (string id, bool state);

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

	ofxXmlSettings xml;
	vector<TrackedHeartbeat>	_trackedHeartbeats;
	TrackedHeartbeat*			getTrackedHeartbeat (string id);

	// osc - sending
	void					setupSending ();
	vector<ofxOscSender*>	_senders;
	vector<int>				_ports;
	int						_receivePort;
	bool					_isReceive;
	string					_broadcastAddress;

	// osc - receiving
	void			setupReceiving ();
	ofxOscReceiver	_receiver;
	int				_current_msg_string;
	string			_msg_strings[NUM_MSG_STRINGS];
	float			_timers[NUM_MSG_STRINGS];
	bool			_isSend;
};