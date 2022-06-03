---
layout: default
title: Designing the Boat
nav_order: 3
---

# Designing the Boat

## 1)   Naive design and guidelines for the boat

To build this boat, we needed to define clear guidelines that we had to follow to design our boat, as well as all the challenges we had to undertake. First of all, the major difficulties we had with this project come from the fact we will be using it in a marine/lake environnement. We thus had to think about :

1. &nbsp; [<img src="assets/floatting.png" alt="floating" style="width:50px;"/>&nbsp;&nbsp;&nbsp;&nbsp; Flotation and Balance](#flotation-and-balance)
2. &nbsp;[<img src="assets/tiller.png" alt="floating" style="width:50px;"/>&nbsp;&nbsp;&nbsp;&nbsp;Ability to Move Sail and Rudder ](#ability-to-move-the-sail-and-the-rudder)
3. &nbsp;[<img src="assets/servo.png" alt="floating" style="width:50px;"/>&nbsp;&nbsp;&nbsp;&nbsp;Space for Components](#space-for-components)
4. &nbsp;[<img src="assets/watter.png" alt="floating" style="width:50px;"/>&nbsp;&nbsp;&nbsp;&nbsp;Watertightness for our Components](#watertightness-for-our-components)
{: .fs-4 .fw-500}

### Flotation and Balance
The first step to make a boat is to do something that **floats**. As our project is roughly a small prototype on wich we tried to experiment automation, we based our calculation on simple principles like **Archimede's principle**. With the design given in the repository, we can have **2.5 kg of mass in total**, boat included. This is well enough, as the major source of weight in our boat is the boat itself. The rest of the components only weight 400g.

***



The **balance** of the boat is as much important as the previous consideration. There are three important parts of the boat that will influence its balance :
- The **spread of our components across the boat**
  - Our components aren't that heavy so the balance was not hard to find.
  - However, the biggest components - the battery - was placed as close as possible to the center of the boat.
  - &rArr; It does not cause much problem to have a boat leaning on its rear, as long as it is not close to sink. What should be seek is a lateral balance and avoiding to lean on the front.
- The ***dagger-board*** wich is mandatory, otherwise the boat would drift in the same direction as the wind instead of moving forward. It also helps stabilize the boat as its bearing capacity when moving creates a moment that keeps the boat upright.
- The ***keel*** might be the most precious ally for balance. Epecially when the mast is high and heavy, the keel plays the role of a counterweight creating a moment that prevent the boat from tilting on the side.

<img src="assets/zero-keel-2-scaled.jpg" alt="floating" style="width:300px; display: flex; margin:auto"/>


### Ability to move the sail and the rudder

The water forces us to carefully think about how to connect the servo to the rudder. Indeed, as you will see our first design had a direct connection between the servo and a underwater rudder, but it was raising major concerns. The servo is not waterproof so we need to keep it away from water as much as possible. A ballbearing was not sufficient and adding to that the pressure underwater, we were affraid water would find its way up to the servo. We finally took another approach explained [here](#2-first-design).

To move the sail is another challenge. There are many possibilities : either we use ropes like a sailor would do or we can directly connect a servo to set its position. We had the chance of having a strong servo wich made feasible the second one, taking away some difficulties.

### Space for components

Although not the toughest guideline, making space for components is crucial in the boat design. It was our starting point to determine the overall dimensions. There are plenty of possible arrangements for the components, different ways to design the boat to fit them in. But the simpler the better, as the components need to be manipulated at some point. We also have to take into account cable : space or holes to route the cables, placing components to avoid long cables...
\
\
<img src="assets/components.png" alt="floating" style="width: 80%; display: flex; margin:auto"/>

 
### Watertightness for our components

The main problem with such automation project is the presence of **water**. This really complexify the way we have to think about our design. As previously mentioned, we need to be careful on the rudder axle, but also all the holes our boat has : for the sail, for the wind vane, ... But most of all, to protect our components, a cover over the compartments is mandatory, and should be tighly fixed to the hull.

Another approach could be to have some waterproof components, like the servos, wich was not our case.

### Naive design Based on these guidelines (or almost...)


## 2)	First design
## 3)	Design improvements
## 4)	Printing the boat
## 5)	Assembling the boat
## 6)	The keel issue
## 7)	Finalizing the boat