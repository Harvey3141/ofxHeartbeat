#include "ofxHeartbeat.h"

void ofxHeartbeat::setup(string id, float heartbeatRate, bool isReceive, bool isSend) {
	 _age = 0;
	 _heartbeatRate = heartbeatRate;
	 _id = id;
	 _isReceive = isReceive;
	 _isSend = isSend;

	 // OSC
	 _port = PORT;
	 if (_isSend)
		setupSending();
	 if (_isReceive)
		 setupReceiving();
}

void ofxHeartbeat::setupSending () {
	// TODO : Set up multicast - currently requires making changes to ofxOSC
	_sender.setup(BROADCAST_ADDRESS, PORT);
	cout << "listening for osc heartbeat messages on port " << PORT << "\n";
}

void ofxHeartbeat::setupReceiving () {
	_receiver.setup(PORT);
	_current_msg_string = 0;
}

void ofxHeartbeat::update() {

	// sending heartbeat
	if (_isSend) {
		_age += ofGetLastFrameTime();
		if (_age > _heartbeatRate) {
			sendHeartbeat();
			_age=0;
		}
	}

	// receiving heartbeats
	if (_isReceive) {

		// increase age of tracked hearts
		for (int i=0; i<_trackedHeartbeats.size(); i++) {
			_trackedHeartbeats[i].age += ofGetLastFrameTime();
		}

		updateReceiveOSC();

		// check for stopped hearts
		for (int i=0; i<_trackedHeartbeats.size(); i++) {
			if (_trackedHeartbeats[i].age > _trackedHeartbeats[i].ageLimit && _trackedHeartbeats[i].coronerNotified == false) {
				_trackedHeartbeats[i].coronerNotified = true;
				ofNotifyEvent(_onStoppedHeart,_trackedHeartbeats[i].id);
			}	
		}
		
		// check for resuscitated hearts
		for (int i=0; i< _trackedHeartbeats.size(); i++) {
			if (_trackedHeartbeats[i].age < _trackedHeartbeats[i].ageLimit && _trackedHeartbeats[i].coronerNotified == true) {
				_trackedHeartbeats[i].coronerNotified = false;
				ofNotifyEvent(_onRestaredHeart,_trackedHeartbeats[i].id);
			}	
		}
	}

}

void ofxHeartbeat::sendHeartbeat() {
	ofxOscMessage msg;
	msg.setAddress("/heartbeat");
	msg.addStringArg(_id);

	_sender.sendMessage(msg);
}

void ofxHeartbeat::registerHeartbeat (string id, float ageLimit) {
	TrackedHeartbeat newHeartbeat;
	newHeartbeat.age = 0;
	newHeartbeat.id = id;
	newHeartbeat.ageLimit = ageLimit;
	newHeartbeat.coronerNotified = false;

	_trackedHeartbeats.push_back(newHeartbeat);
}

void ofxHeartbeat::receiveHeartbeat (string id) {
	if (getTrackedHeartbeat(id) != NULL)
		getTrackedHeartbeat(id)->age = 0;

	else registerHeartbeat(id,5.0f);
}

void ofxHeartbeat::updateReceiveOSC () {
	// hide old messages
	for(int i = 0; i < NUM_MSG_STRINGS; i++){
		if(_timers[i] < ofGetElapsedTimef()){
			_msg_strings[i] = "";
		}
	}

	// check for waiting messages
	while(_receiver.hasWaitingMessages()){
		// get the next message
		ofxOscMessage m;
		_receiver.getNextMessage(&m);
        
		// check for heartbeat  message
		if(m.getAddress() == "/heartbeat"){
			string id = m.getArgAsString(0);
			// ignore own heartbeat
			if (id != _id)
				receiveHeartbeat(id);
		}

		else if(m.getAddress() == "/sleep"){
			string id = m.getArgAsString(0);
			bool isSleep = m.getArgAsInt32(1);
			if (id == _id)
				ofNotifyEvent(_onSleep,isSleep);
		}

		else{
			// unrecognized message: display on the bottom of the screen
			string msg_string;
			msg_string = m.getAddress();
			msg_string += ": ";
			for(int i = 0; i < m.getNumArgs(); i++){
				// get the argument type
				msg_string += m.getArgTypeName(i);
				msg_string += ":";
				// display the argument - make sure we get the right type
				if(m.getArgType(i) == OFXOSC_TYPE_INT32){
					msg_string += ofToString(m.getArgAsInt32(i));
				}
				else if(m.getArgType(i) == OFXOSC_TYPE_FLOAT){
					msg_string += ofToString(m.getArgAsFloat(i));
				}
				else if(m.getArgType(i) == OFXOSC_TYPE_STRING){
					msg_string += m.getArgAsString(i);
				}
				else{
					msg_string += "unknown";
				}
			}
			// add to the list of strings to display
			_msg_strings[_current_msg_string] = msg_string;
			_timers[_current_msg_string] = ofGetElapsedTimef() + 5.0f;
			_current_msg_string = (_current_msg_string + 1) % NUM_MSG_STRINGS;
			// clear the next line
			_msg_strings[_current_msg_string] = "";
		}

	}
}


TrackedHeartbeat* ofxHeartbeat::getTrackedHeartbeat (string id) {
	for (int i=0; i<_trackedHeartbeats.size();i++) {
		if (_trackedHeartbeats[i].id ==id)
			return &_trackedHeartbeats[i];
	}
	return NULL;
}