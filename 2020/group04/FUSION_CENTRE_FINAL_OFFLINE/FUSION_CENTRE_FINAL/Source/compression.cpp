/*
  ==============================================================================

    compression.cpp
    Created: 11 Dec 2020 8:41:53am
    Author:  victor Letens

  ==============================================================================
*/

#include "compression.h"


Compression::Compression()
{
}

void Compression::doTheCompressionBaby(const float * inputBuffer,float * outBuffer,int numberOfSamples,int samplingRate,int treshold,float makeUpGain,int kneeWidth,int compressionRatio,int attackTime,int releaseTime){
    float alphaAttack=exp(-1/(attackTime*samplingRate));
    float alphaRelease=exp(-1/(releaseTime*samplingRate));
    
    std::vector<float> sideChain;
    std::vector<float> gainInput;
    std::vector<float> gainOutput;
    std::vector<float> peakInput;
    std::vector<float> peakOutput;
    std::vector<float> sideChainLin;

    for(int i=0;i<numberOfSamples;i++){
        //calculate abs
        sideChain.push_back(std::abs(inputBuffer[i]));
        
        //convert to db
        float iSideChain=sideChain.at(i);
        gainInput.push_back(20*log10(iSideChain));
        
        // Gain computer
        float iGainInput=gainInput.at(i);
        if(iGainInput<-100){
            iGainInput=-100;
        } else if (iGainInput>100){
            iGainInput=100;
        }
        
        if ((2*(iGainInput-treshold))<(-kneeWidth)) {
            gainOutput.push_back(iGainInput);
        } else if ((2*std::abs(iGainInput-treshold))<=kneeWidth){
            gainOutput.push_back(iGainInput+(pow((1/compressionRatio -1)*iGainInput-treshold+kneeWidth/2,2)/(2*kneeWidth)));
        } else {
            gainOutput.push_back(treshold+(iGainInput-treshold)/compressionRatio);
        }

        // input-output gain computer
        peakInput.push_back(gainInput.at(i)-gainOutput.at(i));
        
        // level detector
        if(i==0){
            peakOutput.push_back((1-alphaAttack)*peakInput.at(i));
        }else{
            if(peakInput.at(i)>peakOutput.at(i-1)){
                peakOutput.push_back(alphaAttack*peakOutput.at(i-1)+(1-alphaAttack)*peakInput.at(i));
            } else{
                peakOutput.push_back(alphaRelease*peakOutput.at(i-1)+(1-alphaRelease)*peakInput.at(i));

            }
        }
        
        //substract from makeUpGain
        peakOutput.at(i)=makeUpGain-peakOutput.at(i);
        
        // back to linear
        sideChainLin.push_back(pow(10,peakOutput.at(i))/20);
        outBuffer[i]=sideChainLin.at(i)*inputBuffer[i];
    }
    
}
