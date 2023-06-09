#include "daisy.h"
#include "daisysp.h"


namespace daisysp {

    class Pooper {

        public:
            void init(float * buffer, int32_t size);
            void process();
            void setDelayTime(float seconds);
            void setSpeed(float speed);
            void setOffset(float offset);
            const float read();
            const float readf(float pos, int32_t offset);

        private:
            float * m_buffer;
            float * m_rPtr;
            int32_t m_numSamples;
            uint32_t m_length;
            float m_speed;
            float m_frac;
            float m_pos;
            int32_t m_offset;
            float prevSample;
            float window[8];  
    };

}
