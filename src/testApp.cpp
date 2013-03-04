#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){

    ofSetVerticalSync(true);
    
    //setting up opentsps and adding the listeners
    tspsReceiver.connect(12000);
    ofxAddTSPSListeners(this);
    ofAddListener(box2d.contactStartEvents, this, &testApp::contactStart);
    ofAddListener(box2d.contactEndEvents, this, &testApp::contactEnd);
    
    //setting up box2d
    box2d.init();
    box2d.createBounds();
    box2d.setGravity(0, 0);
    box2d.setFPS(30);
    box2d.registerGrabbing();
    celebrate = 0;
    
    //setup the air hockey puck
    setupBall();
    
    //setting up testing with the mouse and forgoing the TSPS
    debug = false;
    setupMouseDebug();      //<-- Comment this out to run tsps
    
}

//--------------------------------------------------------------
void testApp::update()
{
    //update box 2d
    box2d.update();
    
    //since mouse is a static body update it's position from input
    if (debug) {
        mouseDebug.setPosition(mouseX, mouseY);
    }
    

    
}

//--------------------------------------------------------------
void testApp::draw()
{
    ofClear(0, 0, 0);
    
    //TODO: Player hit true draw circles...and reset true;
    if (!debug) {
        
        for (list<ofxBox2dCircle>::iterator i = players.begin(); i != players.end(); ++i) {
        
            PlayerInfo * info = (PlayerInfo*) i->getData();
            
            ofVec2f position = i->getPosition();
            
            ofSetColor(255, 255, 255);
            ofFill();
            ofCircle(position, 30);
            ofSetColor(255, 0, 0);
            ofDrawBitmapString(ofToString(info->score), position);
            
            //if there's a reason to celebrate celebrate
            if (info->celebrate) {
                drawCelebration(position, info);
            }
        }
    } else {
        PlayerInfo * info = (PlayerInfo*) mouseDebug.getData();
        
        ofVec2f position = mouseDebug.getPosition();
        
        ofSetColor(255, 255, 255);
        ofFill();
        ofCircle(position, mouseDebug.getRadius());
        ofSetColor(255, 0, 0);
        ofDrawBitmapString(ofToString(info->score), position);
    
        if (info->celebrate) {
            cout << "i'm celebrating" << endl;
            drawCelebration(position, info);
        }
    }
    //drawing the soccer ball last of all
    ofSetColor(255, 0, 0);
    ofCircle(ball.getPosition(), ball.getRadius());
    
}

void testApp::setupBall() {
    //setup the physics of the ball
    ball.setPhysics(3, .53, 1);
    ball.setup(box2d.getWorld(), ofGetWidth()/2, ofGetHeight()/2, 20);
    
    //set the user data so we can figure out if it is the ball
    ball.setData(new PlayerInfo);
    PlayerInfo* ballinfo = (PlayerInfo*) ball.getData();
    ballinfo->player = false;
    ballinfo->isdead = false;
    ballinfo->score  = 0;
    ballinfo->index  = -1;
    cout << "setup the ball structure " << (ball.isBody() ? "true" : "false") << endl;
    
}

void testApp::setupMouseDebug() {
    
    debug = true;
    
    //setup the physics of the ball
    ball.setPhysics(0, .53, 1);
    mouseDebug.setup(box2d.getWorld(), mouseX, mouseY, 30);
    
    //set the user data so we can figure out if it is a mouse
    mouseDebug.setData(new PlayerInfo);
    PlayerInfo * mouseInfo = (PlayerInfo*) mouseDebug.getData();
    mouseInfo->player = true;
    mouseInfo->isdead = false;
    mouseInfo->score  = 0;
    mouseInfo->index  = 1;
    mouseInfo->hit    = false;
    
}

//celebrate good times come on
void testApp::drawCelebration(ofVec2f it, PlayerInfo * info) {
    if (celebrate <= 10) {
        ofSetColor(ofRandom(255), ofRandom(255), ofRandom(255));
        ofFill();
        ofCircle(it.x, it.y, celebrate*10);
        celebrate++;
    } else {
        hit = false;
        info->celebrate = false;
        celebrate = 0;
    }
    
}


//OPENTSPS EVENT
void testApp::onPersonEntered( ofxTSPS::EventArgs & tspsEvent ){
    
    bool found = false;
    
    for (list<ofxBox2dCircle>::iterator i = players.begin(); i != players.end(); ++i) {
        //if we found the person on the last go through get out of here
        if (found) break;
        //get the user data as we come in for each player so that we can see the info
        PlayerInfo * info = (PlayerInfo*) i->getData();
        
        //check to see if it was a blob detect problem and reassign the point if so
        if ( info->isdead && abs(tspsEvent.person->centroid.x - i->getPosition().x) <= 100 &&
            abs(tspsEvent.person->centroid.y - i->getPosition().y) <= 100 ) {
            
            i->setPosition(tspsEvent.person->centroid);
            info->index = tspsEvent.person->pid;
            info->isdead = false;
            found = true;
            i->alive = true;
        }
       
    }
    
    //if we didn't find it then create it
    if (!found) {
        
        //create the circle
        ofxBox2dCircle c;
        c.setPhysics(0, .5f, .9);
        c.setup(box2d.getWorld(), tspsEvent.person->centroid.x*ofGetWidth(), tspsEvent.person->centroid.y*ofGetHeight(), 100);
        
        //set the circles data
        c.setData(new PlayerInfo);
        PlayerInfo* info = (PlayerInfo*) c.getData();
        info->index = tspsEvent.person->pid;
        info->score = 0;
        info->player = true;
        info->isdead = false;
        info->hit    = false;
        
        //push it into the list
        players.push_back(c);
    }


}
//OPENTSPS EVENT
void testApp::onPersonUpdated( ofxTSPS::EventArgs & tspsEvent ){
    //update the positions
    
    for (list<ofxBox2dCircle>::iterator i = players.begin(); i != players.end(); i++) {
        PlayerInfo * info = (PlayerInfo*) i->getData();
        if (tspsEvent.person->pid == info->index)
            i->setPosition(tspsEvent.person->centroid.x*ofGetWidth(), tspsEvent.person->centroid.y*ofGetHeight());
    }
    
}
//OPENTSPS EVENT
void testApp::onPersonWillLeave( ofxTSPS::EventArgs & tspsEvent ){
    //turn them off
    //TODO: Figure out when to delete them
    for (list<ofxBox2dCircle>::iterator i = players.begin(); i != players.end(); ++i) {
        PlayerInfo * info = (PlayerInfo*) i->getData();
        if ( tspsEvent.person->pid == info->index) {
            info->isdead = true;
            i->alive = false;
            info->hit = false;
        }
    }
    
}

//BOX2D EVENT
void testApp::contactStart(ofxBox2dContactArgs &e) {
    cout << "contact has started" << endl;
//    if(e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle) {
//        if(e.a != NULL && e.b != NULL) {
//            PlayerInfo * infoA = (PlayerInfo*) e.a->GetBody()->GetUserData();
//            PlayerInfo * infoB = (PlayerInfo*) e.a->GetBody()->GetUserData();
//            
//            if (infoA->player == false || infoB->player == false) {
//                if (infoA->player == true) {
//                    e.a->GetBody()->SetLinearVelocity(b2Vec2(5,5));
//                }
//                if (infoB->player == true) {
//                    e.b->GetBody()->SetLinearVelocity(b2Vec2(5,5));
//                }
//            }
//        }
//    }

}
//BOX2D EVENT
void testApp::contactEnd(ofxBox2dContactArgs &e) {
    
    //check to see if the types are correct
    if(e.a->GetType() == b2Shape::e_circle && e.b->GetType() == b2Shape::e_circle) {
        
        //make sure that neither of them are null
        if(e.a != NULL && e.b != NULL) {
            
            //get the info out of each
            PlayerInfo * infoA = (PlayerInfo*) e.a->GetBody()->GetUserData();
            PlayerInfo * infoB = (PlayerInfo*) e.b->GetBody()->GetUserData();
            
            //if either of them are players then let's add some points and say
            //that something's been hit
            if (infoB->player == true) {
                infoB->score+=1;
                ofVec2f position = ofVec2f(e.b->GetBody()->GetPosition().x, e.b->GetBody()->GetPosition().y);
                ball.addRepulsionForce(position, 100);
                infoB->celebrate = true;
                
            } else if (infoA->player == true) {
                infoA->score+=1;
                ofVec2f position = ofVec2f(e.a->GetBody()->GetPosition().x, e.a->GetBody()->GetPosition().y);
                ball.addRepulsionForce(position, 10);
                infoA->celebrate = true;
            }
            
            
        }
    }

}






