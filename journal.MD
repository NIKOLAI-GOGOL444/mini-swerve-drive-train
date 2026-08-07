all logs from forge that predate my stardance documentation are here! if you want to see more recent logs, please visit my stardance page. the link to that is in the readme.md file. 

FOR HORIZONS REVIEWERS
https://lapse.hackclub.com/user/@nikolai_gogol444
that is the link to my lapse page. all pcb time lapses are easily found beacause they are conviniently named. the EARLIEST time lapse that Is part of my horizons project and is documented on the code file that I am attaching my horizons time lapses to is schematic design from july 20, 2026 at 3:22am. any timelapse that predates that Is asociated with my stardance project and can be ignored. I Will start documenting journal entries for my horizons project here.
ENTRY 2 FRIDAY AUG 7


alot of work since my last entry! after tracing the 50 ohm traces on my controller, I ran it against the DRC and fixed any issues that came up, then I organized the silkscreens, and added some decals! here is the finished pcb design, that is available in the controller pcb stuff file. 

after that, I began working on the controller pcb. I reused the schematic for my controller, then deleted the joysticks and battery setup, and kept the things like the rf schematic that I would need for my controll pcb. this pcb required me to keep within a tight 52 by 52 mm square in order to fit in the chasis of my robot. I acheived this by being clever with part placement, optimizing my esp chips pinout and using the backside of my pcb. my pcb includes a mpu6050, a tca multiplexer, test points to connect to my motors and encoders, drv8833 motor drivers, usbc ports for serial data and flashing, and a charge system that allows power to come from either the battery or the usbc ports, and steps down some of that voltage to 3.3v to run logic parts. I used a 4 layer pcb to better isolate the rf signal, and allow for two signal planes. I plan to purchase these pcbs from pcbbuy using my own money when I get the chance. 

ENTRY 1:
I didnt know I was supposed to do journal entries, so Ill do one big one here to document the past 3 or so days of pcb work, and go consistently from there.
I first began by doing a quick 20 minute KICAD tutorial video, as I do before using any new software, to figure out how to use it. then, I just kind of began working and learning the software on the way. I started by looking up schematics for the breakout boards that I had used to breadboard my project, so I could figure out what chips and components id need. I then went to the data sheet for the chips and replicated the ideal pcb setups shown in each datasheet on my board, and using google gemini for input and questions as I went. I wouldnt reccomend entirely trusting AI to help with pcb design, because It was very hard getting the AI to interpret my schematic correctly, and it would identify things that were perfectly fine as errors, vice versa. after that, I had a complete controller schematic with two joysticks, a 2.4ghz RF system based on the NRF24L01 PA LNA breakout board, a power distribution system that charges a 3.7v li po battery from onboard usbc ports I included for charging, serial data and code flashing, and powers the board using said battery, and boot and reset buttons. a picture of my schematic is available in the controller pcb stuff file. 
I then went to begin routing, when I got some errors, which I eventually solved by adding pads that were missing in the schematics, usually EPADs and ground pads, and routing them to where they were supposed to go, and changing my ESP32 S3 WROOM N16R8 schematic to the generic ESP32 S3 WROOM 1 schematic, which was identical to the custom footprint I got on the internet and was native to KICAD, which fixed the errors. I then began organizing my components on the board, and today I will be designing the trace on the RF stuff on the board to have exactly 50 OHMs of resistance and routing all my components, new entry coming soon. 


# 2026-06-25: some pretty important CAD fixes, and a chasis!

hi! I finally did the final changes on CAD, im pretty sure, ill know if theyre perfect when I print the fixed parts, should work though cad looks good. 

I fixed the CAD issues previously stated in a earlier post, which included the uncentered bevel gear, loosely meshed drive gears, and loosely meshing bevel gear in the drive system. all the gears in fusion mesh properly now, and the meshing is good in the drive bevel system at all rotations, I just made sure. Im going to update the GITHUB fusion files. 

also, I made a chasis for my swerve modules. its made to be pretty lightweight, as this whole project is, Im trying to stay under 3lbs. one swerve module including electronics is 3.7 oz, and the chasis is 2.1 oz, so im well below that weight limit currently. I will also include the chasis in the new    CAD update. 
![Screenshot 2026-06-25 at 12.38.27 AM.png](https://cdn.hackclub.com/019efd13-34e1-7dce-a0f7-82d5298bfcb3/Screenshot%202026-06-25%20at%2012.38.27%E2%80%AFAM.png)

![Screenshot 2026-06-25 at 12.35.18 AM.png](https://cdn.hackclub.com/019efd13-8c22-79ff-86ed-d85c031c2d19/Screenshot%202026-06-25%20at%2012.35.18%E2%80%AFAM.png)

# 2026-06-17: origins


I thought it would be a good idea to mention how I started this project, as I only started documenting it here once I heard about this website. my first steps were going to the fab shop at robotics club, and looking at swerve modules, aswell as taking photos and videos. I moved around the wheel and rolled it, to get a better understanding of the mechanisms that allowed for the consecutive steering and driving. I also refrenced 3d models online, and I defenitley reccomend checking out NERDSPARK 9312s nerd swerve modules, their ingenious idea of using two gear sets with the same ratio, but one smaller than the other, to create a equal spinning motion the encoders could measure, is truly revolutionary. prior to cadding my modules, I chose electronics based off of what would be found on a real frc bot, that were also cost effective. I also plan to design a pcb when I have a complete breadboarded system, and plan to keep these parts for breadboarding. 

heres the part list for those who are curious.

esp32 s3 3.3v(3pcs ESP32-S3-DevKitC-1-N16R8 Development Board Integrated 2.4GHz WiFi Bluetooth 5.0 ESP32-S3-WROOM-1 Microcontroller Processor Dual Type C Interface for Arduino)

drive controllers 7.4v (5PCS DC DRV8833CPWR Motor Drive Board 2 Channel 3-10V 1.5A H Bridge DC Gear Motor Driver Controller Small Volume Board for DIY Projects)

drive motors 7.4v from controllers for motors, 3.3v for encoders (FABULETTA 2 Pack N20 Encoder DC Gear Motor 6V Metal High Torque D-Shaft for Robotics GA12-N20 Replacement 160-3000RPM Reduction(500RPM))

steer motors 7.4v from controllers(4X JA12-N20 Model DC 12V 100RPM Torque Mini Motor Silver+Gold)

steer shaft encoders 3.3v(WWZMDiB 4Pcs AS5600 Magnetic Encoder 3.3V 12bit high Precision Magnetic Induction Angle Measurement Sensor Module，Mainly Used to Obtain Information Such as Progressive Motor Speed)

breakout board 3.3v(PCA9548A TCA9548A I2C IIC 8-Channel Multiplexer Breakout Board - 1.65-5.5V 400KHz for Arduino, Address 0x70-0x77)

Gyroscope 3.3v(HiLetgo GY-521 MPU-6050 MPU6050 3 Axis Accelerometer Gyroscope Module 6 DOF 6-axis Accelerometer Gyroscope Sensor Module 16 Bit AD Converter Data Output IIC I2C for Arduino)

Bearings(6900-2RS Deep Groove Bearings, ID 10mm x OD 22mm x Width 6mm Double Rubber Sealed Ball Bearings, Pre-Lubricated (GCr15) Chrome Steel 4pcs)

smaller bearings(20Pcs 6700ZZ Deep Groove Ball Bearings, 10x15x4mm Double Metal Shielded Ball Bearing Chrome Steel P0 Z2, Thin Section Miniature Skateboard Bearings for Industrial Machine, Power Tools)

even smaller bearings(uxcell MR105-2RS Deep Groove Ball Bearings 5mm Inner Dia 10mm OD 4mm Bore Double Sealed Chrome Steel Blue Seal Z2 20pcs)

bearings for inner drive mechanism (uxcell 12Pcs MR117ZZ Small Bearing, 7mm ID 11mm OD 3mm Width Double Shielded Deep Groove Ball Bearings for Electric Motor Skateboards 3D Printer, (ABEC 5))

7.4v lipo battery
![IMG_0574.jpeg](https://cdn.hackclub.com/019ed38f-6e11-753c-bd25-719dae422219/IMG_0574.jpeg)
![IMG_0575.jpeg](https://cdn.hackclub.com/019ed38f-3cc2-757a-8b3b-e9c616ed3ecf/IMG_0575.jpeg)

# 2026-06-16: code modifications!


after coding the motors to move with joystick input, I came across some issues:

- motors getting too hot
- motors moving when joystick isnt touched
- severe delay between joysticks and motor movement. 

after working with gemini, I was able to solve these issues. 

- setting the motor frequency to 2000 prevented overheating
- first, I calculated the average displacement of the joysticks from the center, when untouched. I used these calculations to create a range, and then programmed my joysticks to output a reading of 2048 when within this range. that helped alot, but I still saw some movement when untouched. I then added cubic math to my motor output, replacing the linear mapping I had previously. this scales the motor movement to a cubic function, this combination eliminated any motor movement when joysticks werent touched.

-to fix the great delay between my joysticks and motors, I did a few changes in my code. 
  - added watchdog timers, to ignore data that takes more than 10 miliseconds to return, to prevent 12c jams. 
  - moved my joystick and motor function calls to the top of my 
    loop function so they would be prioritized
  - changed the read frequency of my sensors to every 50                   
    miliseconds and print frequecy of my serial monitor to every  
    250 miliseconds.
these changes cleaned up my 12c bus, which allowed the important data needed for motor decisions to happen much more efficiently, which reduced the delay to an unoticeable ammount. Im intrested to see how else I will have to alter the 12c bus to reduce delay when I incorperate my radio reciever and transmitter for joystick to robot communication. 
![Screenshot 2026-06-16 at 12.28.41 AM.png](https://cdn.hackclub.com/019ecec4-f614-75c9-aa7f-687b6be1f1e1/Screenshot%202026-06-16%20at%2012.28.41%E2%80%AFAM.png)
![Screenshot 2026-06-16 at 12.28.50 AM.png](https://cdn.hackclub.com/019ecec4-bdc9-791e-8079-3272805df56a/Screenshot%202026-06-16%20at%2012.28.50%E2%80%AFAM.png)

![Screenshot 2026-06-16 at 12.28.54 AM.png]
(https://cdn.hackclub.com/019ecec4-96c8-762d-a563-3aacbec99962/Screenshot%202026-06-16%20at%2012.28.54%E2%80%AFAM.png)


# 2026-06-16: breadboarding, coding, and cad problems!

I began to breadboard the electronics for my swerve modules, and I also did some coding to get them to work. code screenshots coming soon. after assembling v5 of my swerves, I unfortunatley discovered that my gears were skipping and were unable to drive when rotated at some points. after inspecting my cad file, I discovered that my bevel gear was not perfectly centered, and my two gears that translate that bevel motion to spin the wheel werent meshing close enough. oops!! so, yesterday I fixed the cad, But I plan to use autodesk's motion features to test my gears before printing, to save materials and time :-). I also want to look into methods of reducing friction, to achieve higher speeds. I want this thing to be super fast lol. [Screenshot 2026-06-15 at 11.27.08 PM.png](https://cdn.hackclub.com/019ece78-a1a3-7471-b8bf-baedbc948f1d/Screenshot%202026-06-15%20at%2011.27.08%E2%80%AFPM.png)![Screenshot 2026-06-15 at 11.26.52 PM.png](https://cdn.hackclub.com/019ece78-b89e-7fa0-bde1-0f52f0c8cffd/Screenshot%202026-06-15%20at%2011.26.52%E2%80%AFPM.png)

# 2026-06-10: swerve module cad V5 done!


after 5 versions, the cad is finally ready to print! (for software and breadboarding atleast, will make changes to the housing later so i can attach it to a chasis, once I know how big my pcb will be.) STL files are available on the github repository. will start on software and breadboarding shortly!!![Screenshot 2026-06-09 at 10.08.25 PM.png](https://cdn.hackclub.com/019eaf53-339b-7378-833b-d1d1aa4056f7/Screenshot%202026-06-09%2
