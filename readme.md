# Getting Started

---

## OPUS

OPUS is a powerful tool that utilizes procedural modeling techniques to generate synthetic data. By exposing its pipeline as an API, OPUS empowers users to create their own parametrizable high-quality 3D assets. These assets can be utilized in a wide range of applications, including simulations, games, and other relevant domains.

## Plugin

* This plugin allows you to easily create, customize, and retrieve assets from the OPUS API using the Unreal Engine's editor interface with a UI.
* Only avaliable for Windows.
* You can access the plugin from both C++ and blueprint projects.
* There are no additional depedencies. No need to include libraries or anything.

## Installation

* Download and extract the UE5_OPUS.rar.
* Copy the exctracted plugins folder into your projects folder.


![1695645297069](image/readme/1695645297069.png)

## Features

* Make requests into OPUS API's endpoints using a graphical user interface.
* Every asset name automaticly visible from the dropdown menu.
* Convenient search bars for customizing desired attributes.
* Queue screen for monitoring jobs, also serving as a history.
* Caching of the assets.
* Automatic addition of assets into Unreal Engine.

<br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>

# User Guide

---

## Login Screen

### Logging In

* Go to Rapid API page of the opus.
* Copy your API key.
* Paste it into the 'Rapid Key' section and login.

![1695645283654](image/readme/1695645283654.png)


<br><br>

## UI Screen


![1695645385022](image/readme/1695645385022.png)


1. Choose the desired asset from here.
2. Create the aseet from here.
3. Switch to the queue screen from here.
4. Logout from here.
5. Choose desired format from here.
6. Remove specific attribute from here.
7. Remove every attribute inside the table.
8. Apply the atributes chosen from the searchboxes into table.
9. Enter the desired input for the parameter. (Only visible when a parameter is chosen.)
10. Search for tags and parameters from here.

<br>

## Queue Screen

![1695645318149](image/readme/1695645318149.png)

1. Return to the main UI screen.
2. Empty the queue list below.
3. Empty caching folder inside the projects saved folder called 'ZippedContents'.
4. '+' is for adding the asset into the Unreal Engine. 'x' is for removing the related asset from the queue list.
