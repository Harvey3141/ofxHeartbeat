
ofxHeartbeat
============

A system of tracking and sending the state of apps also using ofxHeartbeat over OSC 

Requires ofxOSC , ofxXmlSettings

Usage
-----

Add heartbeatSettings.xml to data folder of your app and specifiy the - send port(s) , receive port and broadcast address



* Create ofxHeartbeat object

	ofxHeartbeat _heartbeat;

* Set up event listeners

```
//ofApp.h

void onStoppedHeart 		(string & id);
void onRestartedHeart 	(string & id);
void onSleep 			(bool & state);

//ofApp.cpp
void ofApp::onStoppedHeart (string & id) {
	// do something here
}

void ofApp::onRestartedHeart(string & id) {
	// do something here
}

void ofApp::onSleep(bool & state) {
	// do something here
}

```	


* Initialise

```
// ofApp.cpp

// (id (string), heartrate (float), send (bool), receive(bool)
_heartbeat.setup("baseApp",5.0f,true,true);

ofAddListener(_heartbeat._onStoppedHeart,this, &ofApp::onStoppedHeart);
ofAddListener(_heartbeat._onStoppedHeart,this, &ofApp::onStoppedHeart);
ofAddListener(_heartbeat._onSleep,this, &ofApp::onSleep);
```



* sending sleep message

```
_heartbeat.sendSleepMessage("id",true);
```



