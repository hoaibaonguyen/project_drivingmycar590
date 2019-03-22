#include <iostream>
#include <chrono>
#include <string>
#include "elma.h"
#include "gtest/gtest.h"

//! \file

using namespace std::chrono;
using std::vector;
using namespace elma;

namespace driving_example {

    //! Example: Driving car simulation process. See examples/drivingmycar.cc.
    //! This example will reuse the driving.cc from Prof. Eric Klavins
    //! This example will add few more process such as change gear, break, wind velocity, and find gas station as low fuel.

    //! See the file examples/driving.cc for usage.
    class Car : public Process {
        public:

        //! Wrap the base process class
        //! \param name The name of the car    
        Car(std::string name) : Process(name) {}

        //! Nothing to do to initialize
        void init() {}

        //! To start a new simulation, this process sets
        //! the car's velocity to zero kph.    
        void start() {}

        //! The update method gets the latest force from the 
        //! Throttle Channel, if any. Then it updates the 
        //! car's velocity, and sends it out on the Velocity
        //! Channel.     
        void update() {
            emit(Event("change gear", start_speed));
        }

        //! Nothing to do to stop    
        void stop() {}

        private:
        double start_speed;
        bool brake_on = true;
        bool hand_brake = true;
        
    };  

    //! Example: A cruise controller for a Car process.  See examples/drivemycar.cc.

    //! See the file examples/drivemycar.cc for usage.
    class CruiseControl : public Process {

        public:

        //! Wrap the base process class
        //! \param name The name of the controller      
        CruiseControl(std::string name) : Process(name) {}

        //! Watch for events that change the desired speed.
        void init() {
            watch("desired speed", [this](Event& e) {
                desired_speed = e.value();
            });
        }

        //! Nothing to do to initialize    
        void start() {}

        //! Get the velocity from the Velocity Channel, compute
        //! a simple proportional control law, and send the result
        //! to the Throttle channel.    
        void update() {
            if ( channel("Velocity").nonempty() ) {
                speed = channel("Velocity").latest();
            }
            channel("Throttle").send(-KP*(speed - desired_speed));
        }

        //! Nothing to do to stop    
        void stop() {}

        private:
        double speed = 0;
        double desired_speed = 0.0;
        const double KP = 314.15;
                    vector<double> _v;
    };

    //! Example: A simulated driver, who keeps cycling between 50 and 60 kph.  See examples/driving.cc.
    class Driver : public Process {

        public: 

        //! Wrap the base process class
        //! \param name The name of the controller       
        Driver(std::string name) : Process(name) {}

        //! initialize the desired speed
        void init() {
            desired_speed = 60;
        }

        //! Nothing to do to start
        void start() {}

        //! If the desired speed is 50, change to 60,
        //! otherwise change to 50.
        void update() {
            if ( desired_speed == 50 ) {
                desired_speed = 60;
            } else {
                desired_speed = 50;
            }
            emit(Event("desired speed", desired_speed));
        }

        //! Nothing to do to stop
        void stop() {}

        private:
        double desired_speed;

    };

    //! Starting for the Drive My Car Projects
    class DriveState : public State {
        
        public:

            DriveState() : State("Check") {}
            DriveState(string name) : State(name) {}
            void entry(const Event& e) {}
            void during() {} 
            void exit(const Event& e) {}

    };

    class Drive : public StateMachine {
    public:
        //!
        //! This will implement the car to have the ability to find the gas station when the fuel is low.
        //! Also this car will automatically stop when issues are found.
        //! 
        //! States will be using for the car with EXACT name
        //! 1/ Those are in use to find the gas station
        //! "Check", "On", "FindStation", "FillGas"
        //! 
        //! 2/ Those are in use when car is having issues
        //! "Drive", "Off", "Fix"
        //! 
        //! Car will be able to respond to events with the EXACT names:
        //! "key in", "fuel low", "find gas station", "fuel full", "start drive", "issue", "find issue", "issue fixed"
        
        Drive(string name) : StateMachine(name), _check("Check"), _on("On"), _find_station("Find Station"), _fill_gas("Fill Gas"), _drive("Drive"), _off("Off"), _fix("Fix") {
            set_initial(_check);
            add_transition("key in", _check, _on);
            add_transition("star drive", _on, _drive);
            add_transition("fuel low", _drive, _find_station);
            add_transition("find gas station", _find_station, _fill_gas);
            add_transition("fuel full", _fill_gas, _check);
            add_transition("issue", _drive, _off);
            add_transition("find issue", _off, _fix);
            add_transition("issue fixed", _fix, _check);
        }
        
        private:
            DriveState _check,
                       _on,
                       _find_station,
                       _fill_gas,
                       _drive,
                       _off,
                       _fix; 
    };

    //! Example: A gear changing controller for a Car process.  See examples/drivemycar.cc.
    class Gear : public Process {

        public: 

        //! Wrap the base process class
        //! \param name The name of the controller       
        Gear(std::string name) : Process(name) {}

        //! initialize the desired speed
        void init() {
            watch("change gear", [this](Event& e) {
                running = true;
            });
            watch("brake on", [this](Event& e) {
                brake_on = true;
            });
            watch("hand brake", [this](Event& e) {
                hand_brake = true;
            });      
        }

        //! Nothing to do to start
        void start() {
            start_speed = 0;
            //brake_on = true;
            //hand_brake = true;
        }

        //! Car will have 6 speed.
        //! Increament from one gear to one gear will be 10
        void update() {
            if ( channel("Throttle").nonempty() ) {
                force = channel("Throttle").latest();
            }

            if (gear == "P" && brake_on == true && hand_brake == true){
                gear = "1";
                brake_on = false;
                hand_brake = false;
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed >= 10 && gear == "1" && brake_on == false && hand_brake == false){
                gear = "2";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed < 10 && gear == "2" && brake_on == false && hand_brake == false){
                gear = "1";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed >= 20 && gear == "2" && brake_on == false && hand_brake == false){
                gear = "3";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed < 20 && gear == "3" && brake_on == false && hand_brake == false){
                gear = "2";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed >= 30 && gear == "3" && brake_on == false && hand_brake == false){
                gear = "4";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed < 30 && gear == "3" && brake_on == false && hand_brake == false){
                gear = "3";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed >= 40 && gear == "4" && brake_on == false && hand_brake == false){
                gear = "5";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed < 40 && gear == "4" && brake_on == false && hand_brake == false){
                gear = "4";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed >= 50 && gear == "5" && brake_on == false && hand_brake == false){
                gear = "6";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed < 50 && gear == "6" && brake_on == false && hand_brake == false){
                gear = "5";
                start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            } else if (start_speed > 60) {
                start_speed =60;
            }
            start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            emit(Event("gear", gear));
            channel("Velocity").send(start_speed);
            std::cout << milli_time() << ","
                    << start_speed << " \n";
        }

        //! Nothing to do to stop
        void stop() {}

        private:
        double start_speed;
        bool brake_on, hand_brake, running;
        double force;
        string gear = "P";
        const double k = 0.02;
        const double m = 1000;

    };

    //! Example: A break will be slowing down speed of a Car for a Car process.  See examples/drivemycar.cc.
    class Brake : public Process {

        public: 

        //! Wrap the base process class
        //! \param name The name of the controller       
        Brake(std::string name) : Process(name) {}

        //! initialize the initial condition
        void init() {
            watch("brake", [this](Event& e) {
                running = true;
                brake_on = true;
            });  
        }

        //! Nothing to do to start
        void start() {
            brake_on = true;
            if ( channel("Velocity").nonempty() ) {
                start_speed = channel("Velocity").latest();
            } else {
                start_speed = 60;
            }
            //brake_on = true;
            //hand_brake = true;
        }

        //! Car will have 6 speed.
        //! Increament from one gear to one gear will be 10
        void update() {
            if ( channel("Throttle").nonempty() ) {
                force = channel("Throttle").latest();
            }

            if (brake_on == true ){
                if (start_speed > 50){
                    start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
                    gear = "5";
                } else if (start_speed > 40){
                    start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
                    gear = "4";    
                } else if (start_speed > 30){
                    start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
                    gear = "3";    
                } else if (start_speed > 20){
                    start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
                    gear = "2";    
                } else if (start_speed > 10){
                    start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
                    gear = "1";    
                } else {
                    start_speed = 0;
                }
            } else {
                brake_on = true;
            }
            
            start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            emit(Event("gear", gear));
            channel("Velocity").send(start_speed);
            std::cout << milli_time() << ","
                    << start_speed << " \n";
        }

        //! Nothing to do to stop
        void stop() {}

        private:
        double start_speed;
        bool brake_on, hand_brake, running;
        double force;
        string gear = "P";
        const double k = 0.02;
        const double m = 1000;

    };
    //! Example: A signal light will allow the car make a caution whenever it tries to turn left, right or in emergency.  See examples/drivemycar.cc.
    class SignalOn : public Process {

        public:
        SignalOn() : Process("signalon") {}
        void init() {
            watch("left on", [this](Event& e) {
                lefton = true;
                righton = false;
            });
            watch("right on", [this](Event& e) {
                lefton = false;
                righton = true;
            });
            watch("emergency on", [this](Event& e) {
                lefton = true;
                righton = true;
            });
            watch("signal off", [this](Event &e) {
                lefton = false;
                righton = false;
            });
            watch("light on", [this](Event& e) {
                light_on_power = e.value();
            });
        }
        void start() {
            lefton = false;
            righton = false;
            light_on_power = 0;
        }
        void update() {}
        void stop() {}

        bool lefton,
            righton;
        double light_on_power;

    };

    //! A class for left signal
    class SignalLeft : public State {
        public:
        SignalLeft() : State("left off") {}
        void entry(const Event& e) {
            if ( e.name() == "left light on" ) {
                emit(Event("light on", e.value()));
            }
        }
        void during() {} 
        void exit(const Event& e) {
            if ( e.name() == "left signal button on" ) {
                emit(Event("left on"));
            }        
        }
    };

    //! A class for right signal
    class SignalRight : public State {
        public:
        SignalRight() : State("right off") {}
        void entry(const Event& e) {
            if ( e.name() == "right light on" ) {
                emit(Event("light on", e.value()));
            }
        }
        void during() {} 
        void exit(const Event& e) {
            if ( e.name() == "right signal button on" ) {
                emit(Event("right on"));
            }        
        }
    };

    //! A class for emergency signal
    class Emergency : public State {
        public:
        Emergency() : State("emergency light off") {}
        void entry(const Event& e) {
            if ( e.name() == "emergency light on" ) {
                emit(Event("light on", e.value()));
            }
        }
        void during() {} 
        void exit(const Event& e) {
            if ( e.name() == "emergency signal button on" ) {
                emit(Event("emergency on"));
            }        
        }
    };

    //! A class to turn the light signal off
    class SignalOff : public State {
        public:
        SignalOff() : State("Signal light off") {}
        void entry(const Event& e) {}
        void during() {} 
        void exit(const Event& e) {
            emit(Event("signal off"));
        }
    };
}

int main() {

    Manager m;

    driving_example::Car car("Car");
    driving_example::Gear gear("ChangeGear");
    driving_example::CruiseControl cc("Control");
    driving_example::Brake brake("GetOnBrake");
    driving_example::Driver driver("Steve");
    Channel throttle("Throttle");
    Channel velocity("Velocity");

    m.schedule(car, 100_ms)
    .schedule(gear,100_ms)
    .schedule(cc, 100_ms)
    //.schedule(brake, 100_ms)
    .schedule(driver, 5_s)
    .add_channel(throttle)
    .add_channel(velocity)
    .init()
    .run(40_s);
}
