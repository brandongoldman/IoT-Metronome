#pragma once

#include "mbed.h"

class metronome
{
public:
    enum { beat_samples = 3 };

public:
    metronome()
        : m_timing(false), m_beat_count(0), m_prev(0), m_curr (0), bpm(0) {}
    ~metronome() {}

public:
    // Call when entering "learn" mode
    void start_timing();
    // Call when leaving "learn" mode
    void stop_timing();

    // Should only record the current time when timing
    // Insert the time at the next free position of m_beats
    void tap();

    bool is_timing() const { return m_timing; }
    // Calculate the BPM from the deltas between m_beats
    // Return 0 if there are not enough samples
    size_t get_bpm() const;
    
    // driver function to get tempo from tapping
    float get_beat();
    
    // will find delta between [tap - 1] & [tap - 2]
    void calculateBPM(float time, int count);

private:
    bool m_timing;
    Timer m_timer;

    // Insert new samples at the end of the array, removing the oldest
    size_t m_beats[beat_samples];
    size_t m_beat_count;
    size_t m_prev;
    size_t m_curr ;
    float bpm;
    float avg;
};

//starts the embedded timer
void metronome::start_timing()
{ 
    //reset values
    m_beat_count = 0;
    m_prev = 0;
    m_curr = 0;
    bpm = 0.0;
    
    m_timing = true;
    m_timer.start(); 
}

//used to stop the embedded timer
void metronome::stop_timing()
{
    m_timing = false;
    m_timer.stop();
}

// Function to log beat count and calculate delta
void metronome::tap()
{
   if(m_timing)
   {
      float elapsedTime = m_timer.read();
      m_beat_count++;
      
      // calculate delta between taps
      calculateBPM(elapsedTime, m_beat_count);
   }
}

// calculates delta of four MOST recent beats
// uses [BEAT - 1] and [BEAT - 2]
void metronome::calculateBPM(float time, int count)
{
    float recentBeat2 = count - 2;
    float recentBeat1 = count - 1;

    if(m_prev == 0)
    {
        m_prev = time;
    }

    else
    {
        m_curr = time;
        
        // delta = time between last two beats
        // update order of taps
        float diff = m_curr  - m_prev;
        m_prev = m_curr ;
 
        float total_time = bpm * recentBeat2 + diff;
        bpm = total_time / recentBeat1;
        float bpm_val = 60 * bpm;
        printf("BPM for last 4 beats is: %f",bpm_val);
        printf("\r\n");
    }   
}

// returns tempo
float metronome::get_beat()
{
  int minBeats = 4;
  
  if(m_beat_count < minBeats)
  {
    return 0;
  }

  else 
  {
    return bpm;
  } 
}
