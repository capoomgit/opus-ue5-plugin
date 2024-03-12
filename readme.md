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

1. Download the latest .zip file of [OPUS Unreal Plugin](https://github.com/capoomgit/opus-ue5-plugin/releases)
2. Extract the .zip file
3. Find the folder containing your Unreal Project. The default location is:
    C:\Program Files\Documents\Unreal Projects\[Your Project Name]
4. Create a folder called "Plugins" and copy the "OPUS" folder inside it
5. Open up your Unreal Project. Restart it if it was already open
6. Go to Window->OPUS or press the OPUS button on your level editor to pull up OPUS window
![OPUS Plugin Button](/ReadmeAssets/UnrealPluginButton.png "OPUS plugin button")
7. Now you will need a Rapid API key to log in. Create an account at: [RapidAPI](https://rapidapi.com/)
8. Subscribe to OPUS API at: [OPUS API Subscribe](https://rapidapi.com/genel-gi78OM1rB/api/opus5/pricing)
9. Get your Rapid API key at [OPUS API](https://rapidapi.com/genel-gi78OM1rB/api/opus5)
![Rapid API Key](/ReadmeAssets/RapidAPIKey.png "Rapid API Key")


## Features

* Make requests into OPUS API's endpoints using a graphical user interface.
* Every asset name automaticly visible from the dropdown menu.
* Convenient search bars for customizing desired attributes.
* Queue screen for monitoring jobs, also serving as a history.
* Caching of the assets.
* Automatic addition of assets into Unreal Engine.

<br><br>

# User Guide

---

## Login Screen

![Login screen](/ReadmeAssets/UnrealLoginScreen.png "Login Screen")
1. Rapid key field to input your Rapid API Key
2. Login button to log in with the provided key

<br><br>

## Creation Screen

![Creation screen](/ReadmeAssets/UnrealCreationScreen.png "Creation Screen")

1. Job queue button to see jobs screen
2. Logout button
3. Model select box, choose the type of model you want to generate using Opus
4. File type box, choose the file format of the asset to be generated
5. Texture size box, choose the texture size in pixels for the textures on the model
6. Select tag search box, click on the box to list all tags. Tags are different style options for the components of the model Click on a tag to add it to the customization table. Type anything to filter them.
7. Select template search box, click on the box to list all templates. Templates are specific attributes for the model, which set certain parameters of model components. Click on a template to add it to the customization table. Type anything to filter them.
8. Select parameter search box, Click on the box to list all parameters. Parameters are the smallest customizations, specifying a value for a specific component. Click on a parameter to add it to the customization table. Type anything to filter them.
9. Customization table, here you will see all the customizations you have selected.
10. Remove customization button, click this button to remove the customization.
11. Parameter value, set the value for the parameter by moving the slider handle or typing a number directly. The value has to be within a certain range. Ranges can differ between parameters.

<br><br>

## Jobs Screen

![Jobs screen](/ReadmeAssets/UnrealJobsScreen.png "Jobs Screen")

1. Back button, click to go back to the creation page.
2. Empty list button, click to empty all jobs from the list. You will not have access to those jobs after you delete them.
3. Jobs list, see all jobs with name, date, time, and status listed here.
4. Download button, click to download the generated model.
5. Remove job button, click to remove that job from the list. Once removed you will no longer have access to that job

<br><br>

## Important notes
- After downloading the model successfully you will be shown an import window. Make sure to set “Import Uniform Scale” to 100.0 in order to see the objects ın propper size.
![Import settings](/ReadmeAssets/UnrealImportSettings.png "Import settings")