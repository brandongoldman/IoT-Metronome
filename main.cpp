#include "mbed.h"
#include "utils.hpp"

#include "EthernetInterface.h"
#include "frdm_client.hpp"

#include "metronome.hpp"


//NOTE: Uncomment the below #define for Part 2
//#define IOT_ENABLED


namespace active_low
{
    const bool on = false;
    const bool off = true;
}

DigitalOut g_led_red(LED1);
DigitalOut g_led_green(LED2);
DigitalOut g_led_blue(LED3);

InterruptIn g_button_mode(SW3);
InterruptIn g_button_tap(SW2);

// Ticker internal class is useful for toggling Play mode to relay Learn data
// Thanks Wyatt for the help :)
Ticker toggle;

bool change_mode = false;
bool been_tapped = false;
float tempo = 0;

//Change mode from learn to play and vice-versa
void on_mode()
{
    // Change modes
    change_mode = true;
}

//log beats during learn mode
void on_tap()
{
    // Receive a tempo tap
    been_tapped = true;
}

//Play learned beats during play mode
void toggleGreen()
{
    g_led_green = active_low::on;   // green on
    wait(0.1f);
    g_led_green = active_low::off;  // green off
}

void toggleRed()
{
    g_led_red = active_low::on;     // red on
    wait(0.1f);
    g_led_red = active_low::off;    // red off
}

int main()
{
    // Seed the RNG for networking purposes
    unsigned seed = utils::entropy_seed();
    srand(seed);

    // LEDs are active LOW - true/1 means off, false/0 means on
    // Use the constants for easier reading
    g_led_red = active_low::off;
    g_led_green = active_low::off;
    g_led_blue = active_low::off;

    // Button falling edge is on push (rising is on release)
    g_button_mode.fall(&on_mode);
    g_button_tap.fall(&on_tap);

    // Needed for part 2 - ignore for now
#ifdef IOT_ENABLED
    // Turn on the blue LED until connected to the network
    g_led_blue = active_low::off;

    // Need to be connected with Ethernet cable for success
    EthernetInterface ethernet;
    if (ethernet.connect() != 0)
        return 1;

    // Pair with the device connector
    frdm_client client("coap://api.connector.mbed.com:5684", &ethernet);
    if (client.get_state() == frdm_client::state::error)
        return 1;

    // The REST endpoints for this device
    // Add your own M2MObjects to this list with push_back before client.connect()
    M2MObjectList objects;

    M2MDevice* device = frdm_client::make_device();
    objects.push_back(device);

    // Publish the RESTful endpoints
    client.connect(objects);

    // Connect complete; turn off blue LED forever
    g_led_blue = active_low::on;
#endif

/************************ BEGIN PERSONAL CODE: ************************/
    metronome metro = metronome();
    metro.start_timing();
    
    while (true) {
#ifdef IOT_ENABLED
        if (client.get_state() == frdm_client::state::error)
            break;
#endif

        // Insert any code that must be run continuously here
        if (change_mode == true) 
        {
            change_mode = false;
            if (metro.is_timing()) 
            {
                metro.stop_timing();
                tempo = metro.get_beat();
                toggle.attach(&toggleGreen, tempo);
            }
            
            else 
            {
                toggle.detach();
                metro.start_timing();
            }
        }

        if (been_tapped == true) 
        {
            metro.tap();
            been_tapped = false;

            if (metro.is_timing()) 
            {
                toggleRed();
            }
        }
    }

#ifdef IOT_ENABLED
    client.disconnect();
#endif

    return 1;
}