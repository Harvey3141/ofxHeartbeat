# ofxHeartbeat

ofxHeartbeat
============


Usage
-----

    	#import "ofxHeartbeat.h"


Create ofxHeartbeat object

	ofxHeartbeat _heartbeat;

Set up listeners
	
(ofApp.h)
void onStoppedHeart (string & id);
void onRestartedHeart (string & id);

	.....

(ofApp.cpp)
void ofApp::onStoppedHeart (string & id) {
	// do something here
}

void ofApp::onRestartedHeart(string & id) {
	// do something here
}


Initialise listeners and Heartbeat (e.g in ofApp.cpp)

	_heartbeat.setup("nameOfApp",5.0f,true,true);

	ofAddListener(_heartbeat._onStoppedHeart,this, &ofApp::onStoppedHeart);

	ofAddListener(_heartbeat._onStoppedHeart,this, &ofApp::onStoppedHeart);



