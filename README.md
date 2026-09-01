IMPORTANT NOTE FOR OUTPOST REVIEWERS: please ignore the pcb stuff in my BOM.CSV and files and stuff. I do not have a pcb design yet, so I will not be requesting funding for that part of this project, and will request for it as a sepreate project at a later time. 
NEW NOTE: I have edited the BOM.csv file to exclude pcb stuff. 

IMPORTANT NOTE FOR HORIZONS REVIEWERS:
NOTE: i am ONLY trying to submit the CONTROLLER pcb right now. all my hours have been working on that, do not reject my project under the robot pcb stuff not being in here, thats because it isnt finished yet, ill submit that at a later date, just worry about the controller stuff. 
UPDATE: as of tuesday july 28 1:18 am, I have finished the controller pcb and uploaded the final designs file to the repo. it is now ready to be purchased.
<img width="1470" height="956" alt="Screenshot 2026-07-28 at 10 36 25 AM" src="https://github.com/user-attachments/assets/b71b5da1-416d-4a28-b90e-c40c97f113d9" />
<img width="1470" height="956" alt="Screenshot 2026-07-28 at 10 36 18 AM" src="https://github.com/user-attachments/assets/054d95ed-29d8-49c6-b9c8-6c71b57ae685" />
<img width="1470" height="956" alt="Screenshot 2026-07-28 at 10 37 19 AM" src="https://github.com/user-attachments/assets/e89ec79b-334d-4463-b576-3a13a0c1f00d" />
8/31/26 UPDATE
after more work, I have resubmitted for hour aproval. since my last update to this repo, I have ordered my finished boards for manufacturing and recieved them in the mail. here are some images. I have begun assembly, first by harvesting some chips from old devboards and breakout modules. after that I hit a wall when I realized that salvaging tiny capacitors and resistors would be impossible, so now I am working on getting all my parts ordered so I can assemble. hopefully I get my hours aprovved soon so I can buy digikey grants, because that would seriously help me greatly. 
<img width="3024" height="4032" alt="IMG_1530" src="https://github.com/user-attachments/assets/571bee08-e54c-47a5-bb12-8d9cb1149eb7" />

<img width="3024" height="4032" alt="IMG_1531" src="https://github.com/user-attachments/assets/a285e2ba-9cb1-4a60-aac2-5f4c3609aad7" />

UPDATE: as of monday july 27th 11:36 pm, I have uploaded new gerbers that are of the pcb fully routed and corrected against the DRC. if wanted, the pcb could now be purchased and it would function, but the silkscreen labels would be a mess. im now going to arrange the silkscreen, then begin on the swerve drive brain pcb. 
NEW NOTE AFTER SECOND REJECTION: 
my pcb wasnt routed yet because this project is currently a work in progress. I am submitting it fairly early on because I want to qualify intime to go to horizons polaris (I have an extension to qualify until july 31st.) I have done work on my pcb and almost finished the routing, it is pretty close to done, just gotta double check everything and fix some small issues. just please understand that this pcb and its gerber files arent finsished, which explains the missing routing and anything you may find in this file. I am actively working on this project, so please give me grace with any structural errors that may be in the pcb and help me make the polaris deadline. im attaching the newer gerbers. 
my project for horizons is the pcbs that go with this robot, I am submitting them as a seperate hardware project that I will not need funding for. I am documenting my hours for the pcbs seperate from the code file I used on the robots cad and coding, however I will be using the same github for both projects just to keep things simple. so ignore all the cad and code stuff, and just look at the PCB stuff, because thats my horizons project. I am currently in the process of tracing the controller PCB, so all I have right now as I submit is the schematic and maybe a unfinished gerber file, but I will keep working on the project and add to my repo as I go. if you have any questions just reach me on the hackclub slack. 
ALSO, the BOM.CSV you see when you open my repo isnt the one for this project, that is for my outpost project. please look in my controller pcb stuff file, you will find my horizons BOM.CSV file in there. 
STEPS TO REPRODUCE:
when pcbs are finished, someone can recreate my PCBS by going online to a pcb manufacturing website, uploading the gerber files for my controller and robot control board I will eventually put in my repo, and ordering them, then either choosing to assemble the pcbs themselves or buying it preassembled. if they wanted recreate the FULL ROBOT (NOT INCLUDED WITH MY HORIZONS SUBMISSION, INCLUDING FYI) they would then flash the code in my repo to their PCBs, then 3d print the cad files with either their own 3d printer or through a manufacturing sight, assembling based off of the cad images ive attached for the project (i think I will add an assembly tutorial eventually), then plugging in batteries and then they would have my robot! 
NEWER 3D RENDERS AND PCB EDITOR SCREENSHOTS AS OF SUN JUL 26 2026
<img width="1470" height="956" alt="Screenshot 2026-07-26 at 7 36 01 PM" src="https://github.com/user-attachments/assets/a55b2485-331b-43c4-9581-89f4ee70ce55" />
<img width="1470" height="956" alt="Screenshot 2026-07-26 at 7 36 13 PM" src="https://github.com/user-attachments/assets/3b809d97-b5d4-4775-b166-e988cb43c2fe" />
<img width="1470" height="956" alt="Screenshot 2026-07-26 at 7 39 59 PM" src="https://github.com/user-attachments/assets/6792e90c-e59a-4a52-8f59-4d59cae57229" />




3D RENDERS (PLEASE NOTE THAT THE CONTROLLER PCB ISNT FINISHED, AND I HAVENT STARTED THE ROBOT PCB.[swervecontrollerPCBv1.pdf](https://github.com/user-attachments/files/30342656/swervecontrollerPCBv1.pdf)
<img width="1470" height="956" alt="Screenshot 2026-07-23 at 6 22 36 AM" src="https://github.com/user-attachments/assets/c5f2b0bd-dd14-45d6-8c25-482454f0edcf" />
<img width="1470" height="956" alt="Screenshot 2026-07-24 at 6 09 05 AM" src="https://github.com/user-attachments/assets/da188812-f2c1-4bf5-b7f6-cbe7a07eeddd" />
<img width="1470" height="956" alt="Screenshot 2026-07-24 at 6 09 14 AM" src="https://github.com/user-attachments/assets/98af6821-ccd7-4822-a2ee-e83853990cf1" />


hi!

im making a mini swerve drive train inspired by FRC swerve drive trains, because they just make me so happy lol.
if you cant tell already idk how to use github at all.
I hope to figure it out as I work on this project. 
if you need anything or have any questions about my project, id be happy to help! just contact me however that works on git idk.

thank you for your time,
-NIKOLAI!
# Mini Swerve Drive Train

inspired by my love of FRC swerve modules and drive trains, they're so freaking cool. 
this project is documented on forge, stardance, and horizons. 
I would most reccomend checking out stardance, because that is where this project is most actively docdumented. 
https://stardance.hackclub.com/projects/25609
ive logged around 25 hours as I write this, however I only began logging maybe two weeks ago, and in reality this project has taken MUCH much longer than that lol. 

AI declaration for hackclub

I worked with claude to turn my google docs parts list into a BOM.CSV file. I also use gemini or other chatbots to help me write code and understand data protocols. I never copy and paste exactly what is given to me without understanding what it is or why, but instead use AI as a learning tool to help me better learn and understand programming. especially as a new programmer, I am entirely self taught using online resources, and AI has been a very useful tool for my learning. 

here is a 3d demo of my cad, stl files are available in this repository. 

https://a360.co/4gOPGcb

<img width="573" height="521" alt="Screenshot 2026-07-09 at 3 30 32 AM" src="https://github.com/user-attachments/assets/b358df51-cc4b-4cd3-b457-2ad10c9ba314" />


<img width="658" height="471" alt="Screenshot 2026-07-09 at 3 30 18 AM" src="https://github.com/user-attachments/assets/d16bb19c-7df7-4f01-84ec-a5cd4b8bd13f" />


