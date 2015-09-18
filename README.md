# ofxHeartbeat

ofxHeartbeat
============



Usage
-----

Add heartbeatSettings.xml to data folder of your app and specifiy - send port(s) , receive port, broadcast address

#import "ofxHeartbeat.h"


* Create ofxHeartbeat object

	ofxHeartbeat _heartbeat;

**** Set up event listeners ****
	
(ofApp.h)
void onStoppedHeart 		(string & id);
void onRestartedHeart 	(string & id);
void onSleep 			(bool & state);

	.....

(ofApp.cpp)
void ofApp::onStoppedHeart (string & id) {
	// do something here
}

void ofApp::onRestartedHeart(string & id) {
	// do something here
}

void ofApp::onSleep(bool & state) {
	// do something here
}


**** Initialise (e.g in ofApp.cpp) ****

_heartbeat.setup("id", HeartRate (float),Send (bool),Receive (bool));
e.g
_heartbeat.setup("baseApp",5.0f,true,true);

ofAddListener(_heartbeat._onStoppedHeart,this, &ofApp::onStoppedHeart);
ofAddListener(_heartbeat._onStoppedHeart,this, &ofApp::onStoppedHeart);
ofAddListener(_heartbeat._onSleep,this, &ofApp::onSleep);


**** sending sleep message ****

_heartbeat.sendSleepMessage("id",true);


