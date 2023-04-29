#include "pooper.h"

using namespace daisysp;

void Pooper::init(float * buffer, int32_t numSamples) {
    m_buffer = buffer;
    m_rPtr = buffer;
    m_numSamples = numSamples;
    m_length = numSamples/4;
    m_speed = 1.0f;
    prevSample = 0.f;
}

void Pooper::process() {
    m_pos = fmod(m_pos + m_speed,m_length);
}

void Pooper::setDelayTime(float t) {
    m_delayTime = t;
    m_length = static_cast<uint32_t>(t*48000.f);
}

void Pooper::setSpeed(float speed) {
    m_speed = speed;
}

void Pooper::setOffset(float offset) {
    float offs = offset * float( m_numSamples - m_length); 
    int32_t int_offs = static_cast<int32_t>(offs);
    float frac = offs - static_cast<float>(int_offs);
    m_offset = m_offset + static_cast<int32_t>(frac*static_cast<float>(int_offs - m_offset));
}

const float Pooper::read() {
    process();
    return readf(m_pos); 
}

const float Pooper::readf(float pos) {
    int32_t int_pos = static_cast<int32_t>(pos);
    m_frac = pos  - static_cast<float>(int_pos);
    
    
    float a = m_rPtr[((int_pos) % m_length) + m_offset];
    float b = m_rPtr[(int_pos + 1) % m_length + m_offset];        
    prevSample = m_frac * (b-prevSample) + a; 
    return prevSample;

}
