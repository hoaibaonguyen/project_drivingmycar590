Drive My Car - Final Project ECE590
===

Simple Car is a final project for ECE590. It will be an event loop and manager process to control the car. The car can be geared up and down. Brake will be applied to the car. It will bring the car from higher speed to lower speed and will stop the car completely. 

Goal
===
This project will help me to get more understand on using process and manager to implement an example of driving a car

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

Change Gear
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

Milestones
===

- Create a change gear class and a way to identify which gear is in (Completed)
- Create a State Machine will control the car whether the car is low in fuel or having issue (Complete)
- Create a Brake bring the velocity down (Completed)
- Create an effect of wind velocity to a car (Expected 3/18)
- Create a simulation test with different environments (Half way)
- Documented project with Doxygen (Expected 3/19)
- Runable with test and submit the project (Expected 3/21)
