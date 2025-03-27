# Smart-Cigarette-Counter

# Device:

The primary preferred problem topic is “Smart Cigarette Tracking Device”. Generally, few people smoke tobacco several times in a single day and it is really a disaster for their health. So, the people in this statement from now on calling them as smokers. They don’t even recognize how many cigarettes and amount of smoke they consumed, gradually after a period they do regret that. Smoking causes a lot of problems to name a few like cancer, hypertension, respiration breathing problems. To overcome this problem a smart cigarette tracking device is needed, and smokers can check the data, like count how many cigarettes they smoked and get notified about how bad is the current situation with the data given by user initially while setting up device (including hypertension, BMI, Diabetes….). The user can check this data in mobile phone application and decide the better way

# Sensor Types Used:

For Our Project, we would be using Grove - Dust Sensor (PPD42NS) to monitor the Air Quality around the person, who’s smoking a cigarette. The Change in Air quality can be an indication of smoking activity being performed. To measure the distance of the wrist (where the M5StickC watch would be worn) from Human Body (Neck) we would be using The VL53L0X is a new generation Time-of-Flight (ToF) laser-ranging module. We would also be using ALLNET4DUINO B29_2 pulse sensor, to measure the heart rate.

# Examples on how sensor's can be used found in following:

https://m5stack.hackster.io/kiraku-labo/balance-robot-9009db

https://create.arduino.cc/projecthub/infoelectorials/project-010-arduino-grove-dust-sensor-ppd42ns-project-ab5f5e

https://hackaday.io/project/12522-realtime-cigarette-smoke-detector

https://hackaday.io/project/169609-lazydoro

# Cloud Integration:

#Overall System Functionality
The Smart Cigarette Counter will send the value of the sensor when triggering the smoke to the end user via mail and update the google sheet. The google sheet will keep a count of the cigarettes along with the chemical constituents of cigarettes. The google sheet will be updated in real-time with IFTTT services, and the collected data will be displayed on the dashboard. For displaying on the dashboard, we will be using Google Sheet. The Threshold for Groove Sensor will be smoke detection. It will be triggered as soon as there are changes in AQI values nearby the person who’s smoking. For Laser Sensor, the threshold will be in a range of distances between the wrist and the mouth. The range of distances will be a fixed range.

# Included Cloud System
We are going to use IFTTT cloud-based service with HTTP protocol for data transfer. The services that we are going to use are mail and google sheet. As the event is triggered in the M5StickC and with the sensors, the mail will be sent to the end-user with a link to the dashboard. The google sheet data will automatically be updated and the user can check the dashboard via the link send on the email.

# Caution: 
One should change the project key of IFTTT in code and WIFI Details

