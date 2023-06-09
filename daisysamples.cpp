#include "daisy_patch.h"
#include "daisysp.h"
#include "pooper.h"
#include "splash.h"

using namespace daisy;
using namespace daisysp;


struct looperparams {
    float speed = 1.f;
    float size = 1.f;
    float offset = 0.f;
    float volume = 1.f;
}; 

enum DISPLAYVALS {
    LOOPER,
    REVERB
};

DaisyPatch patch;
const int32_t BUFFER_SIZE = 960000;
const int NUM_LOOPERS = 3;
Pooper poopers[3];
looperparams loopers[NUM_LOOPERS];
ReverbSc rev;
DISPLAYVALS display = LOOPER;



float DSY_SDRAM_BSS buffer[BUFFER_SIZE];
uint32_t n = 0;
bool gate = false;
float lastOffset = 0;
int looper = 0;
float lastCtrlSpeed, lastCtrlSize, lastCtrlOffset, lastCtrlVol;
float lastCtrlRev, lastCtrlFreq;
void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

	for (size_t i = 0; i < size; i++)
	{
		out[0][i] = 0.f;
		out[1][i] = 0.f;
		out[2][i] = 0.f;
        out[3][i] = 0.f;
    }
    float revL, revR;
    for(size_t i = 0; i < size; ++i) {
        if (gate) {
        buffer[n] = in[0][i];
        }
        n = (n + 1) % BUFFER_SIZE;
        out[0][i] = loopers[0].volume * poopers[0].read();        
        out[0][i] += loopers[1].volume * poopers[1].read();        
        out[0][i] += loopers[2].volume * poopers[2].read();        
        rev.Process(out[0][i], out[0][i], &revL, &revR);
        out[1][i] = out[0][i];
        out[0][i] += revL;
        out[1][i] += revR;
    }
}
void UpdateControls() {

    patch.ProcessAllControls();

    gate = patch.gate_input[0].State();
            
    if(patch.encoder.RisingEdge())
        display = DISPLAYVALS((int(display) + 1 ) % 2);
    
    switch (display) {

        case LOOPER: 
        {
            float ctrlSpeed = patch.GetKnobValue((DaisyPatch::Ctrl)0)*2.f;
            float ctrlSize = patch.GetKnobValue((DaisyPatch::Ctrl)1)*2.f;
            float ctrlOffset = patch.GetKnobValue((DaisyPatch::Ctrl)2);
            float ctrlVol = patch.GetKnobValue((DaisyPatch::Ctrl)3);


            if (abs(lastCtrlSpeed-ctrlSpeed) > 0.001) {
                lastCtrlSpeed = ctrlSpeed;
                loopers[looper].speed = ctrlSpeed;
                poopers[looper].setSpeed(ctrlSpeed); 
            }

            if (abs(lastCtrlSize-ctrlSize) > 0.001) {
                lastCtrlSize = ctrlSize;
                loopers[looper].size = ctrlSize;
                poopers[looper].setDelayTime(ctrlSize); 
            }

            if (abs(lastCtrlOffset-ctrlOffset) > 0.001) {
                lastCtrlOffset = ctrlOffset;
                loopers[looper].offset = ctrlOffset;
                poopers[looper].setOffset(ctrlOffset); 
            }

            if (abs(lastCtrlVol-ctrlVol) > 0.001) {
                lastCtrlVol = ctrlVol;
                loopers[looper].volume = ctrlVol;
                //poopers[looper].setVolume(ctrlVol); 
            }



            looper += patch.encoder.Increment();
            looper = ((looper % 3) + 3) % 3;
            break;
        }
        
        case REVERB:
        { 
            float ctrlFreq = patch.GetKnobValue((DaisyPatch::Ctrl)1) * 10000;
            float ctrlRev = patch.GetKnobValue((DaisyPatch::Ctrl)0);
            if (abs(lastCtrlFreq-ctrlFreq) > 0.001) {
                lastCtrlFreq = ctrlFreq;
                rev.SetLpFreq(ctrlFreq); 
                
            }
            if (abs(lastCtrlRev-ctrlRev) > 0.001) {
                lastCtrlRev = ctrlRev;
                rev.SetFeedback(ctrlRev); 
                
            }
            
            break;
        }

        default:
            break;
    }
}

void UpdateOled() {

    patch.display.Fill(false);

    switch (display) {
   
        case LOOPER: 
        {
            patch.display.SetCursor(0, 0);
            std::string str  = "Pooper";
            char*       cstr = &str[0];
            patch.display.WriteString(cstr, Font_6x8, true);

            patch.display.SetCursor(42, 0);
            str = std::to_string(looper);
            patch.display.WriteString(cstr, Font_6x8, true);
         
            patch.display.SetCursor(0, 15);
            str = std::to_string(int(loopers[looper].speed * 100)) + "%";
            patch.display.WriteString(cstr, Font_6x8, true);

            patch.display.SetCursor(30, 15);
            str = std::to_string(int(loopers[looper].size * 1000)) + "ms";
            patch.display.WriteString(cstr, Font_6x8, true);

            patch.display.SetCursor(70, 15);
            str = std::to_string(int(loopers[looper].offset * 1000)) + "ms";
            patch.display.WriteString(cstr, Font_6x8, true);

            patch.display.SetCursor(70, 0);
            str = std::to_string(int(loopers[looper].volume * 100)) + "%";
            patch.display.WriteString(cstr, Font_6x8, true);
    
            //patch.display.SetCursor(0, 20);
            int f = BUFFER_SIZE / 128;
            for (int i = 0; i < 128; i++) {
                patch.display.DrawPixel(i, buffer[f*i]*8 + 40, true);
            }

            patch.display.DrawLine(loopers[looper].offset*128, 30, loopers[looper].offset*128, 50,true);
            patch.display.DrawLine(128*(loopers[looper].offset+loopers[looper].size), 50, 128*(loopers[looper].offset+loopers[looper].size), 30 ,true);
            break;
        }
    
        case REVERB: 
        {
            patch.display.SetCursor(0, 0);
            std::string str  = "FX";
            char*       cstr = &str[0];
            patch.display.WriteString(cstr, Font_6x8, true);

            patch.display.SetCursor(0, 40);
            str = std::to_string(int(lastCtrlRev*100)) + "%";
            patch.display.WriteString(cstr, Font_6x8, true);

            patch.display.SetCursor(30, 40);
            str = std::to_string(int(lastCtrlFreq)) + "Hz";
            patch.display.WriteString(cstr, Font_6x8, true);

            break;
        }
    
        default:
            break;

    } 
    
    patch.display.Update();
}
void DrawSplash() {

        uint32_t i, b, j;
        uint32_t currentX_ = 10;
        uint32_t currentY_ = 8;

        for(i = 0; i < 808; i++)
        {
            b = tema[i];
            uint32_t f = i/104;
              for(j = 0; j < 8; j++)
              {
                  if((b << j) & 0x80) {
                    patch.display.DrawPixel(currentX_ + i%104, currentY_ + 8*f-j, true);
                  }
                  else {
                    patch.display.DrawPixel(currentX_ + i%104, currentY_ + 8*f-j, false);
                  }
              }
        }
}

int main(void)
{
	patch.Init();
    memset((float *)&buffer[0],0.f, BUFFER_SIZE);
	patch.SetAudioBlockSize(16); // number of samples handled per callback
	patch.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    rev.Init(48000);
    rev.SetLpFreq(10000); 
   
    DrawSplash();
    patch.display.Update(); 
    patch.DelayMs(5000);

    poopers[0].init(&buffer[0], 960000);
    poopers[1].init(&buffer[0], 960000);
    poopers[2].init(&buffer[0], 960000);
	patch.StartAdc();
	patch.StartAudio(AudioCallback);
	while(1) {
    UpdateControls(); 
    UpdateOled();
    }
}
