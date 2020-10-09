# charisma-sdk-ue4
Charisma SDK for Unreal Engine 4.

## Getting started

* Add the following to your projects .Build.cs file:
```C#
PublicDependencyModuleNames.AddRange(new string[] { "HTTP", "SocketIOClient", "SocketIOLib", "Json", "JsonUtilities", "SIOJson"});

PublicDefinitions.Add("WITH_OGGVORBIS");
```
* Copy the contents of /Source to your projects /Source folder.
* In Charisma.h, Change CHARISMA_API to <YOUR_PROJECT_NAME>_API.
* Move the contents of /Third Party to your projects /Plugins folder.
* Rebuild your project solution in Visual Studio.
* You can now use the plugin in your project.
