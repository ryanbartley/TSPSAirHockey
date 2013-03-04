#pragma once

#include "ofMain.h"
#include "ofxBox2d.h"
#include "ofxTSPSReceiver.h"
#include <list>

struct PlayerInfo {
    int     index;
    int     score;
    bool    player;
    bool    isdead;
    bool    hit;
    bool    celebrate;
};

class testApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void setupBall();
    void setupMouseDebug();
    void drawCelebration(ofVec2f, PlayerInfo*);

    void onPersonEntered( ofxTSPS::EventArgs & tspsEvent );
    void onPersonUpdated( ofxTSPS::EventArgs & tspsEvent );
    void onPersonWillLeave( ofxTSPS::EventArgs & tspsEvent );
    void contactStart(ofxBox2dContactArgs &e);
    void contactEnd(ofxBox2dContactArgs &e);
    
    list<ofxBox2dCircle>    players;
    ofxBox2dCircle          ball;
    ofxTSPS::Receiver       tspsReceiver;
    ofxBox2d                box2d;
    ofxBox2dCircle          mouseDebug;
    bool                    hit;
    bool                    debug;
    int                     celebrate;
};
