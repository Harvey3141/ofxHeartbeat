#include "ofxHeartbeat.h"

void ofxHeartbeat::setup(string id, float heartbeatRate, bool isSend, bool isReceive) {
	 _age = 0;
	 _heartbeatRate = heartbeatRate;
	 _id = id;
	 _isReceive = isReceive;
	 _isSend = isSend;

	 // Loads sending port(s) and receiving port
	 load("heartbeatSettings.xml");

	 ofAddListener(ofEvents().update, this, &ofxHeartbeat::update);		

	 // OSC
	 if (_isSend)
		setupSending();
	 if (_isReceive)
		 setupReceiving();
}

void ofxHeartbeat::setupSending () {
	// TODO : Set up multicast - currently requires making changes to ofxOSC
	if (_ports.size() > 0) {
		for (int i=0 ; i<_ports.size();i++) {
			ofxOscSender* newSender = new ofxOscSender;
			int port = _ports.at(i);
			newSender->setup(_broadcastAddress,_ports.at(i));
			_senders.push_back(newSender);
			cout << "broadcasting heartbeat messages on "<< _broadcastAddress << " to port " << port << "\n";
		}
	} else _isSend = false;
}

void ofxHeartbeat::setupReceiving () {
	_receiver.setup(_receivePort);
	_current_msg_string = 0;
	cout << "listening for heartbeat messages on port " << _receivePort << "\n";
}

void ofxHeartbeat::update(ofEventArgs& args) {

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

		// check for heartbeat messages
		updateReceiveOSC();

		// check for stopped hearts
		for (int i=0; i<_trackedHeartbeats.size(); i++) {
			if (_trackedHeartbeats[i].age > _trackedHeartbeats[i].ageLimit && _trackedHeartbeats[i].coronerNotified == false) {
				ofNotifyEvent(_onStoppedHeart,_trackedHeartbeats[i].id);
				_trackedHeartbeats[i].coronerNotified = true;
			}	
		}
		
		// check for resuscitated hearts
		for (int i=0; i< _trackedHeartbeats.size(); i++) {
			if (_trackedHeartbeats[i].age < _trackedHeartbeats[i].ageLimit && _trackedHeartbeats[i].coronerNotified == true) {
				ofNotifyEvent(_onRestaredHeart,_trackedHeartbeats[i].id);
				_trackedHeartbeats[i].coronerNotified = false;
			}	
		}
	}

}

void ofxHeartbeat::sendHeartbeat() {
	ofxOscMessage msg;
	msg.setAddress("/heartbeat");
	msg.addStringArg(_id);

	// send to all senders
	for (int i=0; i< _senders.size(); i++) {
		_senders.at(i)->sendMessage(msg);
	}
}

void ofxHeartbeat::registerHeartbeat (string id, float ageLimit) {
	TrackedHeartbeat newHeartbeat;
	newHeartbeat.age = 0;
	newHeartbeat.id = id;
	newHeartbeat.ageLimit = ageLimit;
	newHeartbeat.coronerNotified = false;

	_trackedHeartbeats.push_back(newHeartbeat);

	cout << "registered new heartbeat lisitener :" << ofToString(_trackedHeartbeats.size()) << " - " << id << endl;
}

void ofxHeartbeat::receiveHeartbeat (string id) {
	if (getTrackedHeartbeat(id) != NULL)
		getTrackedHeartbeat(id)->age = 0;

	else registerHeartbeat(id,5.0f);
}

void ofxHeartbeat::updateReceiveOSC () {

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
			try {
				logSleep(isSleep);
			}
			catch (exception& e) {
				 cout << "ofxHeartbeat - logging - Standard exception: " << e.what() << endl;
			}
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

void ofxHeartbeat::load (string path) {
	bool isLoaded = xml.load(path);
	if (isLoaded) {
		// load receive port
		_receivePort = xml.getValue("settings:receivePort",0);

		// load broadcast address
		_broadcastAddress = xml.getValue("settings::broadcastAddress","192.168.1.255");

		// load send ports
		_ports.clear();
		xml.pushTag("settings");
		xml.pushTag("sendPorts");

		int numPorts = xml.getNumTags("port");
		for (int i=0; i< numPorts; i++) {
			int port = xml.getValue("port",0,i);
			_ports.push_back(port);
		}
		xml.popTag(); // pop sendPorts
		xml.popTag(); // pop settings
	} else 
		cout<< "[ERROR] Heartbeart - cannot load heartbeatSettings.xml" << endl;
}

TrackedHeartbeat* ofxHeartbeat::getTrackedHeartbeat (string id) {
	for (int i=0; i<_trackedHeartbeats.size();i++) {
		if (_trackedHeartbeats[i].id ==id)
			return &_trackedHeartbeats[i];
	}
	return NULL;
}

void ofxHeartbeat::sendSleepMessage (string id, bool state) {
	ofxOscMessage msg;
	msg.setAddress("/sleep");
	msg.addStringArg(id);
	msg.addIntArg(state);

	cout << "Sending sleep message to " << id << ": " << ofToString(state) << endl;

	// send to all senders
	for (int i=0; i< _senders.size(); i++) {
		_senders.at(i)->sendMessage(msg);
	}
}

void ofxHeartbeat::setIsSend (bool state) {
	_isSend = state;
	cout << "Heartbeat - isSend set to  " << state << endl;
}

void ofxHeartbeat::logSleep (bool sleepState) {
	ofFile file;
	file.open(ofToDataPath("heartbeatLog.csv"), ofFile::ReadWrite, false);

	if (file) {
		ofBuffer buffer = file.readToBuffer();  
		bool requireNewLine = false;
		// get last line
		string currentLine;  
		int lines = 0;
		while (! buffer.isLastLine()) {  
			currentLine = buffer.getNextLine();
			lines ++;
		}
		buffer.resetLineReader();

		if (!currentLine.empty()) {
			// date, sleep count, wake count
			vector<string> splitString;
			splitString = ofSplitString(currentLine, ",");

			if (splitString.size() == 4) {
				// get parameters
				string bufferDate = ofToString (splitString.at(0));
				int bufferSleepCount = ofToInt(splitString.at(1));
				int bufferWakeCount = ofToInt(splitString.at(2));
				
				cout << "Heartbeat - buffer date: " << bufferDate << " , sleep count: " << bufferSleepCount
				<< " , wake count: " << bufferWakeCount << endl;
				
				// get current date
				char date_c[9];
				_strdate(date_c);
				string date = (string)date_c;
				
				bool isToday;
				(date == bufferDate) ? isToday = true : isToday = false;
				
				// overwrite last line
				if (isToday) {
					ofBuffer newBuffer;
					int newBufferLines = 0;
					while (newBufferLines < lines -1) {  
						newBuffer.append(buffer.getNextLine() + "\n");
						newBufferLines ++;
					}
				
					sleepState ? bufferSleepCount += 1 : bufferWakeCount += 1;
				
					string newLine = date + "," + ofToString(bufferSleepCount) + "," + ofToString(bufferWakeCount) + ",";
					newBuffer.append(newLine + "\n");
					bool fileWritter = ofBufferToFile("heartbeatLog.csv",newBuffer);
				} 
				else requireNewLine = true;
			}
			else requireNewLine = true;
		}
		else requireNewLine = true;

		if (requireNewLine) {
			// get current date
			char date_c[9];
			_strdate(date_c);
			string date = (string)date_c;

			if (sleepState) {
				string line = date + ",1,0,";
				buffer.append(line + "\n");
			}
			else {
				string line = date + ",0,1,";
				buffer.append(line + "\n");
			}
			bool fileWritter = ofBufferToFile("heartbeatLog.csv",buffer);
		}
	}

	// append to new line with current date
	else if (!file) {
		ofBuffer newBuffer;
		// get current date
		char date_c[9];
		_strdate(date_c);
		string date = (string)date_c;

		newBuffer.append("date, sleep, awake, \n");

		if (sleepState) {
			string line = date + ",1,0,";
			newBuffer.append(line + "\n");
		}
		else {
			string line = date + ",0,1,";
			newBuffer.append(line + "\n");
		}
		bool fileWritter = ofBufferToFile("heartbeatLog.csv",newBuffer);
	}
}

