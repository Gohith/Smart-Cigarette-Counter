#pragma once

struct SmokeDetectorSettings
{
    double concTH;//concentration threshold
    unsigned short HRTH;//Heart Rate threshold
    unsigned short distTH;//Distance threshold
    unsigned long waitTime;//Time between smokes
};

typedef void (*detect_t)(unsigned long, double, unsigned short, unsigned short);//callback type for detection

class SmokeDetector
{
public:
    SmokeDetector(SmokeDetectorSettings settings_, detect_t detectCB_ ) : 
    settings(settings_),
    detectCB(detectCB_)
    {

    }

    void setConc(double conc_, unsigned long time)
    {
        conc = conc_;
        isConcSet = true;
        analyze(time);
    }

    void setHR(unsigned short hr_, unsigned long time)
    {
        hr = hr_;
        isHRSet = true;
        analyze(time);
    }

    void setDist(unsigned short dist_, unsigned long time)
    {
        dist = dist_;
        isDistSet = true;
        analyze(time);
    }

private:
    void analyze(unsigned long time)
    {
        //TODO: check if data is up to date

        //check if there is sufficient data
        if(!isConcSet || !isHRSet || !isDistSet ) return;

        //check if it's too soon
        if( detected && (time - lastDetected) < settings.waitTime ) return;
        
        //check if smoking
        if( (conc >= settings.concTH) 
            && (dist <= settings.distTH) 
            && (hr >= settings.HRTH) )
        {
            detected = true;
            lastDetected = time;
            detectCB(time, conc, hr, dist);
        }
    }

    SmokeDetectorSettings settings;
    detect_t detectCB;

    //values
    double conc;
    unsigned short hr;
    unsigned short dist;
    
    bool isConcSet = false;
    bool isHRSet = false;
    bool isDistSet = false;

    bool detected = false;
    unsigned long lastDetected;
};
