#pragma once
#include <M5StickC.h>
#include <vector>
#include "utils.h"

struct SmokeSession
{
  int startTime;
  int duration;
};

struct SmokeSimulatorSettings
{
  double simSpeed; //speed of simulation, 1 is real time
  
  //Smoke
  double defVal;
  double minVal;
  double maxVal;
  double addSpeed; //per second
  double minRemSpeed;
  double maxRemSpeed;
  double speedErr; //for add, remove speed
  double readErr; //readings fluctuate
  double faultProb; //faulty reading probability
  
  //Heart Rate
  double restingHR;
  double targetHR;
  double incSpdHR; //to target
  double decSpdHR; //to resting
  double readErrHR;

  //ToF
  unsigned short restDist;
  unsigned short smokeDist;
  double readErrDist;
};

class SmokeSimulator
{
public:
  SmokeSimulator( SmokeSimulatorSettings settings_, std::vector<SmokeSession> sessions_) : 
  sessions(sessions_),
  settings(settings_),
  curSmoke(settings_.minVal),
  curHR(settings_.restingHR),
  curDist(settings_.restDist)
  {
    if(!sessions.size())
    {
      Serial.println("Cannot start without smoking sessions.");
    }
    srand(analogRead(0));
  }

  void start(unsigned long time)
  {
    if(isRunning)
    {
      Serial.println("Already running.");
      return;
    }

    isRunning = true;
    startTime = time;
    lastStepTime = time;
    curSessionId = 0;
  }

  void stop()
  {
    if(!isRunning)
    {
      Serial.println("Already stopped.");
      return;
    }

    isRunning = false;
    areSessionsDone = false;
    startTime = 0;
    lastStepTime = 0;
    curSessionId = -1;
  }

  unsigned long getTime()
  {
    return curTime;
  }

  unsigned short getCurDist()
  {
    return fluctuate(curDist, settings.readErrDist);
  }

  double getCurSmoke()
  {
    if(fRand(0,1) < settings.faultProb) return getFaultyValue();

    return constrain( fluctuate(curSmoke, settings.readErr), settings.minVal, settings.maxVal);
  }

  double getCurHR()
  {
     return fluctuate(curHR, settings.readErrHR);
  }

  void step(unsigned long time)
  {
    if(!isRunning) return;

    curTime = time * settings.simSpeed;
    auto sessionStart = sessions[curSessionId].startTime;
    auto sessionEnd = sessions[curSessionId].startTime + sessions[curSessionId].duration;
    bool isSmoking = false;

    if(curTime > sessionStart && curTime < sessionEnd )
    {
      isSmoking = true;
    }
    else if(curTime > sessionEnd && !areSessionsDone )
    {
      nextSession();
    }

    if(isSmoking)
    {
      curDist = settings.smokeDist;
      incSmoke();
      incHR();
    }
    else
    {
      curDist = settings.restDist;
      decHR();
    }

    decSmoke();
    lastStepTime = curTime;
  }

private:
  double getFaultyValue()
  {
    switch (iRand(0,2))
      {
      case 0:
        return settings.minVal;
      case 1:
        return settings.maxVal;
      default:
        return fRand(settings.minVal, settings.maxVal);
      }
  }

  void nextSession()
  {
    if( curSessionId < sessions.size()-1 )
    {
      Serial.println("Proceeding to next session.");
      ++curSessionId;
    }
    else
    {
      areSessionsDone = true;
      Serial.println("Finished all sessions.");
    }
  }

  void incHR()
  {
    curHR += settings.incSpdHR * deltaTime();
    if(curHR > settings.targetHR) curHR = settings.targetHR;
  }

  void decHR()
  {
    curHR -= settings.decSpdHR * deltaTime();
    if(curHR < settings.restingHR) curHR = settings.restingHR;
  }

  void incSmoke()
  {
    curSmoke += fluctuate( settings.addSpeed * deltaTime(), settings.speedErr); 
  }

  void decSmoke()
  {
    auto curRemSpeed = (curSmoke - settings.minRemSpeed) / (double)(settings.maxRemSpeed - settings.minRemSpeed);
    curSmoke -= fluctuate(curRemSpeed * deltaTime(), settings.speedErr);
    
    if(curSmoke < settings.defVal ) curSmoke = settings.defVal;
  }

  double deltaTime()
  {
    return (curTime - lastStepTime)/1000.0;
  }

  const SmokeSimulatorSettings settings;
  const std::vector<SmokeSession> sessions;

  double curSmoke;
  double curHR;
  unsigned short curDist;

  bool isRunning = false;
  bool areSessionsDone = false;
  unsigned long startTime = 0;
  unsigned long curTime = 0;
  unsigned long lastStepTime = 0;
  int curSessionId = -1;
};
