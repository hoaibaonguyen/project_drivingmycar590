Drive My Car - Final Project ECE590
===

Simple Car is a final project for ECE590. It will be an event loop and manager process to control the car. The car can be geared up and down. Brake will be applied to the car. It will also have an implementation of the signal lights such as left signal, right signal, and emergency signal. 

Goal
===
This project will help me to get more understand on using process and manager as an implementation example of driving a car from speeding up by changing gear, slowing down by pushing brake, and turn left and right using the correct light signal.

How to run
===
The source code for Drive My Car [is on Github](https://github.com/hoaibaonguyen/project_drivingmycar590.git).

From Dockerhub
---

To get started, you will need a C++ build environment. Then following the instruction below

    git clone git clone https://github.com/hoaibaonguyen/project_drivingmycar590.git
    cd simpleCar_ECE590
    docker run -v $PWD:/source -it klavins/elma:latest bash
    make
    examples/bin/drivemycar

Car using Finite State Machines
===

<img src="images/carfinitemachine.png" width="600"></image>

If unable to see the image. Please find the image in folder [images](https://github.com/hoaibaonguyen/project_drivingmycar590/blob/master/images/carfinitemachine.PNG)

This implementatin will control the car through all automatical step to ensure the car is being droven in the good condition.

```c++
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
```

Change Gear
===
This class will help to get the car into the desired gear. The car will contain 6 speeds. It will turn from 1 to 2 if velocity is greater than the expected or get from 2 to 1 if velocity is decreasing below the expected velocity, etc. This will be contained in function "update" in Gear class

```c++
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
            } 
            ...
            start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            emit(Event("gear", gear));
            channel("Velocity").send(start_speed);
            std::cout << milli_time() << ","
                    << start_speed << " \n";
        }
```
Graph from simulation to show the velocity is incrasing from zero to near max 60 while car is changing gear from 1 to 6. Image can be found @[image](https://github.com/hoaibaonguyen/project_drivingmycar590/blob/master/images/gearup.PNG)

<img src="images/gearup.png" width="600"></image>


Apply Brake
===
This class is designed to bring down the car's speed. It is designed to start at a specific speed. When brake is pushed, velocity will be going down. Also, it will bring back the gear to the initial condition. As the car completely stops, velocity will be down to zero.

```c++
if (brake_on == true ){
                if (start_speed > 50){
                    start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
                    gear = "5";
                } 
                ...
            } else {
                brake_on = true;
            }
            
            start_speed += ( delta() / 1000 ) * ( - k * start_speed + force ) / m;
            emit(Event("gear", gear));
            channel("Velocity").send(start_speed);
            std::cout << milli_time() << ","
                    << start_speed << " \n";
```

Graph from simulation to show the velocity is decreasing to zero. Image can be found @[image](https://github.com/hoaibaonguyen/project_drivingmycar590/blob/master/images/brake.PNG)

<img src="images/brake.png" width="600"></image>

Signal Lights
===
This class is an implementation of the car lights. It will send the signal to the correct light when driver want to make a turn. Also, when driver finds an issue, an emergency light can also be turned on.

```c++
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
```
The specific of each signal light work can be found in folder [examples/drivemycar.cc](https://github.com/hoaibaonguyen/project_drivingmycar590/tree/master/examples)

Improvements
===

This project is fun. This is my first time to learn C and C++. There are a lot of things for me to improve in general. To this project, there are still issues that I have faced during running the simulation. Here are some challenges that I need to learn and improve in the future:
- Have gear and brake be able to run together.
- Design an user interface for driver to select the signal light buttons.
- Learn about document the project efficiently

Milestones
===

- Create a change gear class and a way to identify which gear is in (Completed)
- Create a State Machine will control the car whether the car is low in fuel or having issue (Complete)
- Create a Brake bring the velocity down (Completed)
- Create a Signal Lights for left turn, right turn, and emergency (Completed)
- Create a simulation test with different environments (Completed)
- Documented project with Doxygen (Completed)
- Runable with test and submit the project (Upload to GitHub on 3/21)
