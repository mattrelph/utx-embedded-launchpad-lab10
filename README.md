# utx-embedded-launchpad-lab10
UTX Embedded - Lab 10 - Traffic Light

Purpose
This lab has these major objectives: 1) the understanding and implementing of indexed data structures; 2) learning how to create a segmented software system; and 3) the study of real-time synchronization by designing a finite state machine controller. Software skills you will learn include advanced indexed addressing, linked data structures, creating fixed-time delays using the SysTick timer, and debugging real-time systems. Please read the entire lab before starting.

System Requirements
Consider a 4-corner intersection as shown in Figure 10.1. There are two one-way streets are labeled South (cars travel South) and West (cars travel West). There are three inputs to your LaunchPad, two are car sensors, and one is a pedestrian sensor. The South car sensor will be true (3.3V) if one or more cars are near the intersection on the South road. Similarly, the West car sensor will be true (3.3V) if one or more cars are near the intersection on the West road. The Walk sensor will be true (3.3V) if a pedestrian is present and he or she wishes to cross in any direction. This walk sensor is different from a walk button on most real intersections. This means when you are testing the system, you must push and hold the walk sensor until the FSM recognizes the presence of the pedestrian. Similarly, you will have to push and hold the car sensor until the FSM recognizes the presence of the car. In this simple system, if the walk sensor is +3.3V, there is pedestrian to service, and if the walk sensor is 0V, there are no people who wish to walk. The walk sensor and walk light will service pedestrians who wish to cross in any direction. This means the roads must both be red before the walk light is activated. In a similar fashion, when a car sensor is 0V, it means no cars are waiting to enter the intersection. The don't walk light should be on while cars have a green or yellow light.

You will interface 6 LEDs that represent the two Red-Yellow-Green traffic lights, and you will use the PF3 green LED for the “walk” light and the PF1 red LED for the “don’t walk” light. When the “walk” condition is signified, pedestrians are allowed to cross in any direction. When the “don’t walk” light flashes (and the two traffic signals are red), pedestrians should hurry up and finish crossing. When the “don’t walk” condition is on steady, pedestrians should not enter the intersection.

Traffic should not be allowed to crash. I.e., there should not be a green or yellow on one road at the same time there is a green or  yellow LED on the other road. In other words, while traffic is flowing in one direction, there should be a red light in the other direction.  You should exercise common sense when assigning the length of time that the traffic light will spend in each state; so that the grading engine can complete the testing in a reasonable amount of time. Each traffic light pattern must be on for at least ½ second but for at most 5 seconds. During simulation grading, we suggest you make all times between ½ and  1 second. Cars should not be allowed to hit the pedestrians; both reds on the roads should be on whenever the walk light is on. The walk sequence should be realistic, showing three separate conditions: 1) “walk”, 2) “hurry up” using a flashing LED, and 3) “don’t walk”. You may assume the three sensors remain active for as long as service is required. The “hurry up” flashing should occur at least twice but at most four times.

The automatic grader checks for function. In other words the grader sets the three inputs, and then checks to see of the output pattern is appropriate. In particular, the grader performs these checks:

0) At all times, there should be exactly one of the {red, yellow, green} traffic lights active on the south road. At all times, there should be exactly one of the {red, yellow, green} traffic lights active on the west road. To switch a light from green to red it must sequence green, yellow then red . The grader checks for the proper sequence of output patterns but does not measure the time the FSM spends in each state. The “walk” and “don’t walk” lights should never both be on at the same time.

1) Do not allow cars to crash into each other. This means there can never be a green or yellow on one road at the same time as a green or yellow on the other road. Engineers do not want people to get hurt.

2) Do not allow pedestrians to walk while any cars are allowed to go. This means there can never be a green or yellow on either road at the same time as a “walk” light. Furthermore, there can never be a green or yellow on either road at the same time as the “don’t walk” light is flashing. If a green light is active on one of the roads, the “don’t walk” should be solid red. Engineers do not want people to get hurt.

3) If just the south sensor is active (no walk and no west sensor), the lights should adjust so the south has a green light within 5 seconds (I know this value is unrealistically short, but it makes the grading faster). The south light should stay green for as long as just the south sensor is active.

4) If just the west sensor is active (no walk and no south sensor), the lights should adjust so the west has a green light within 5 seconds. The west light should stay green for as long as just the west sensor is active.

5) If just the walk sensor is active (no west and no south sensor), the lights should adjust so the “walk” light is green within 5 seconds. The “walk” light should stay green for as long as just the walk sensor is active.

6) If all three sensors are active, the lights should go into a circular pattern in any order with the west light green, the south light green, and the “walk” light is green. Of course, the road lights must sequence green-yellow-red each time.

The grading engine can only check for function, not for the quality of your software. This section describes, in qualitative manner, what we think is good design. There is no single, “best” way to implement your system. A “good” solution will have about 9 to 30 states in the finite state machine, and provides for input dependence. Try not to focus on the civil engineering issues. I.e., first build a quality computer engineering solution that is easy to understand and easy to change, and then adjust the state graph so it passes the functional tests of the automatic grader. Because we have three inputs, there will be 8 next state links. One way to draw the FSM graph to make it easier to read is to use X to signify don’t care. For example, compare the two FSM graphs in Figure 10.2. Drawing two arrows labeled 01 and 11 is the same as drawing one arrow with the label X1. When we implement the data structure, however, we will expand the shorthand and explicitly list all possible next states.



Figure 10.2. FSM from Chapter 10 redrawn with a short hand format.

The following are some qualitative requirements, which we think are important, but for which the automatic grader may or may not be able to evaluate.

0) The system provides for input dependence. This means each state has 8 arrows such that the next state depends on the current state and the input. This means you cannot solve the problem by simply cycling through all the states regardless of the input.

1) Because we think being in a state is defined by the output pattern, we think you should implement a Moore and not a Mealy machine. However, your scheme should use a table data structure stored in ROM.

2) There should be a 1-1 mapping between FSM graph and data structure. For a Moore machine, this means each state in the graph has a name, an output, a time to wait, and 8 next state links (one for each input). The data structure has exactly these components: a name, an output, a time to wait, and 8 next state indices (one for each input). There is no more or no less information in the data structure then the information in the state graph.

3) There can be no conditional branches in program, other than the while in SysTick_Wait and the for in SysTick_Wait10ms. This will simplify debugging the FSM engine. The main loop of your program should be similar to the traffic FSM described in this chapter.

4) The state graph defines exactly what the system does in a clear and unambiguous fashion. In other words, do not embed functionality (e.g., flash 3 times) into the software that is not explicitly defined in the state graph.

5) Each state has the same format of each state. This means every state has exact one name, one 8-bit output (could be stored as one or two fields in the struct), one time to wait, and 8 next indices.

6) Please use good names and labels (easy to understand and easy to change). Examples of bad state names are S0 and S1.

7) There should be 9 to 30 states with a Moore finite state machine. If your machine has more than 30 states, you have made it more complicated than we had in mind. Usually students with less than 9 states did not flash the “don’t walk” light, or they flashed the lights using a counter.  Counters and variables violate the “no conditional branch” requirement.

In real products that we market to consumers, we put the executable instructions and the finite state machine data structure into the nonvolatile memory such as flash EEPROM. A good implementation will allow minor changes to the finite machine (adding states, modifying times, removing states, moving transition arrows, changing the initial state) simply by changing the data structure, without changing the executable instructions. Making changes to executable code requires you to debug/verify the system again. If there is a 1-1 mapping from FSM to linked data structure, then if we just change the state graph and follow the 1-1 mapping, we can be confident our new system operate the new FSM properly. Obviously, if we add another input sensor or output light, it may be necessary to update the executable part of the software, re-assemble and retest the system.

The grader will activate the PLL so the system runs at 80 MHz, you must not modify this rate.

https://youtu.be/qNl7vayZEz0
