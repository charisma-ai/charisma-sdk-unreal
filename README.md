# Charisma SDK for Unreal Engine 4

Support: `oscar@charisma.ai`

The plugin is verified for engine versions: `[4.26]`

**NOTE: This document is incomplete. To read about the usage of Charisma in UE4 blueprints, please refer to this document:** https://docs.google.com/document/d/1z64Xhe9ij9hpjum1bOzfNEQ8bhwmtHLdiXOlgYsjwFY/edit?usp=sharing

## Get Started

* Before getting started with the Charisma SDK for UE4, you will need a story to interact with. To create a story, please visit: https://charisma.ai to get started.

* If you dont have a project to add the plugin to, create one.

* Move the contents of this folder to the `/Plugins` located at the root of your project. If this folder doesn't exit, create it.

* Close the UE4 editor and rebuild the project in Visual Studio.

* When you next open the editor, verify the plugin loaded correctly by navigating to `/Settings/Plugins` and locate the Charisma plugin. It should looks something like this:


![StoryId](https://i.ibb.co/6Y5qyK6/charisma-plugin.png)

* You can now start using Charisma in UE4.

## Story Credentials

In order to play Charisma stories from UE4 you will need a both a `storyId` as well as an `apiKey` (the API-Key is only required to play the draft( unpublished) version of your story).

### Story Id

The `storyId` is the unique id of the story that you want to play. To find this, navigate to the Charisma website and the story you want to play from UE4. The `storyId` can be found in the URL as shown in the image below.

![StoryId](https://i.ibb.co/sPqS9n2/StoryId.png)

### API-Key

To play the draft version of your story you have to supply the `Charisma` object with your stories apiKey before acquiring the `playthroughToken`.

An `apiKey` should now be used for authentication for playthrough token creation instead of `draftToken`. This is now the recommended way to authenticate as API keys do not expire (unless regenerated) and are more secure than the `draftToken` if compromised. `draftToken `should no longer be used. However, please make sure to not share the API key with anyone you do not trust, and strip the key from any public builds as before.

![API key](https://i.ibb.co/X86bNVK/API-key.png)

### Starting A Story

When starting a Charisma story from within UE4 you will first have to generate a `playthroughToken`. In the `createPlaythroughToken` method, you have to pass in both the `storyId` This is the version of the story you want to play.

* To play the latest published version, keep this at 0.

* To play a specific, published, version of your story, set the number to that particular version.

* To play the draft version, set the number to -1. To do this, you must also supply your API-key to the `Charisma` object before calling the `createPlaythroughToken` method.

