# Charisma SDK for Unreal Engine 4

This Unreal Engine plugin is verified to work with C++ projects using engine version `4.26` only. If you find the plugin also works in another version, feel free to submit a pull request to update this!

If you have any questions or need a hand, please reach out to us at [hello@charisma.ai](mailto:hello@charisma.ai)!

## Getting started

**Important:** Before setting up the Charisma SDK for UE4, you’ll need to have created a web or game story (not an app story!), which requires a Charisma licence. Please visit the [licencing docs on our website](https://charisma.ai/docs/licencing) for more info!

1. If you haven’t already, go ahead and create an Unreal **C++** project. Make sure you are using a version of the engine that is supported by this plugin.

2. Head to the [Releases page](https://github.com/charisma-ai/charisma-sdk-ue4/releases) of this GitHub repository and download the latest release. Unzip the contents into the `/Plugins` folder located at the root of your project. If this folder doesn’t exist, create it!

3. Close the UE4 editor and rebuild the project in Visual Studio.

4. When you next open the editor, verify the plugin loaded correctly by navigating to `Settings -> Plugins` and locating the Charisma plugin. It should look something like this:

![StoryId](/Docs/EnablePlugin.png)

5. You can now start using Charisma in UE4! 🎉


## Usage

To create playthrough tokens, you’ll first need to find out your story ID, and optionally an API key and version of the story you want to play.

### Story ID

The `StoryId` is the unique ID of the story that you want to play. To find this, navigate to your story on the Charisma website, and copy the ID number after `/stories` in the URL.

![StoryId](/Docs/StoryId.png)

### Story version

* To play the latest published version, pass `0`.
* To play a specific, published version of your story, set the number to that particular version.
* To play the draft version of a story, pass `-1` and supply an `ApiKey` to the `Charisma` object before calling the `CreatePlaythroughToken` method.

### API key

To create a playthrough that references the draft version of your story, you also need to provide an `ApiKey`. This can be found on the 'Story Overview' page.

**Important:** Please make sure to not share the API key with anyone you do not trust, and strip the API key from any public builds.

![API keys](/Docs/ApiKey.png)


## Quick start

This SDK provides a template Game Mode Blueprint which can help you get started quickly with Charisma. The template performs some basic steps, such as creating a playthrough token, conversation, and connecting to Charisma. It can then be customised further to suit your own project's needs!

1. To use the template, open up "Window" -> "World Settings" and change the "GameMode Override" to `BP_CharismaGameModeTemplate`.

2. Open up the `BP_CharismaGameModeTemplate` Blueprint, and head to the Event Graph. To connect Unreal Engine with your story in Charisma, find the "Create Playthrough Token" node and enter the "Story Id", "Story Version" and "API Key" (as described above).

3. *(Only if you are using a game story, not a web story)*, in the "Start" node on the far-right, enter a "Start Graph Reference Id" corresponding to the graph in Charisma you want to start from. The reference ID can be found by clicking the three dots next to the subplot's name in the Charisma editor, and selecting "Rename".

4. If you play the scene now, you should find that the messages in your story start coming through as on-screen debug messages!

Next, we can place some actors in the scene, one for each character in your story, and use the "Charisma Actor Component" to connect them to the active playthrough in the Game Mode. This enables actors to automatically play any voice clips that are received from Charisma.

The SDK provides a template actor to do this:

1. Open up the content browser, and make sure "Show plugin content" is selected in the "View Options" menu in the bottom-right corner.

2. Select the "charisma-ue4 Content" folder and drop the template actor inside the scene.

3. In the "Details" panel, select the "CharismaActor" component, and enter the "Character Name" as it is in the Charisma editor. Then check "Play audio".

4. Now, when you play, the actor (character) will play all of its relevant lines on its own audio component!

This should be all you need to get started with Charisma. For further customisation of the Charisma integration in your experience, please see the documentation below!


## Concepts

### Starting

To start playing a particular story or graph, use the `Start` node, which accepts the following parameters:

* `ConversationUuid`: The specific conversation you wish to start. This comes from the output pin of a `CreateConversation` node.
* `SceneIndex`: If you are referencing a non-game story, you will want to enter the index of the scene you want to start here. Otherwise leave this at `0`.
* `StartGraphReferenceId`: If you are referencing a game story you can use the `StartGraphReferenceId` to reference the scene you want to start. Unlike `StartGraphId`, the `StartGraphReferenceId` is the same across the draft and published versions of your story. To find the `StartGraphReferenceId` of the particular graph you want to start, locate the Subplot in the `Subplots` section of your story page and select rename. This will bring up a textbox containing your `StartGraphReferenceId`. If you wish to use another method for referencing your graph, leave this field empty.
* `UseSpeech`: This is a `boolean` letting Charisma know whether or not you want to include the character voice data of the node in the messages you receive.

### Receiving messages

The `Charisma Message Event` contains all information about the particular node in your graph that fired the `OnMessage` event. What you choose to do with the data is up to you!

At this point you can start interacting with the graph by sending replies or actions based on input and events that take place in UE4.

### Replying

When you hit a player node in your graph you’ll want the player to be able to send a reply. To do this, call the `Reply` function on the Charisma object.

The `Reply` function accepts two parameters:

* `ConversationUuid`: A `String`. The UUID of the specific conversation to reply to. 
* `Message`: A `String`. The text we want to to send in our reply to Charisma. For example, this message could come from a UE4 text input box or from a speech-to-text service such as 'Google Speech To Text'.

![Reply](https://i.ibb.co/pzPd08s/reply.png)


### Actions

You can use action nodes in your graph to respond to actions the player takes in the game world. 

The `Action` function accepts two parameters:

* `ConversationUuid`: A `String`. The UUID of the specific conversation to reply to. 
* `ActionName`: A `String`. The name of the action.

For example the `open-door` action in this graph;

![ActionGraph](https://i.ibb.co/qJW4tfc/action-graph.png)

would be triggered like this from UE4:

![Action](https://i.ibb.co/xqM0WdJ/action.png)


### Character voices

You may have assigned voices to your characters that you wish to play in-engine. To do this you will want to drag out on the `Message` pin on the `Charisma Message Event` and select `Break Message`.

From the `Message` node you will want to drag out on the `Speech` pin and select `Break Speech` from the context menu. 

Drag out on the byte array called `Audio` and search for `Create Sound From Bytes` in the context menu. Select `Create Sound From Bytes`. This function returns a `Sound Wave` that you can play from any audio source in UE4.

![Speech](https://i.ibb.co/mqFtL4k/speech.png)


### Setting memories

You can set memories in your playthrough straight from within Unreal. When setting memories we want to make sure that we pass the correct `RecallValue` to the `SetMemory` function. To find a memories `RecallValue` select the memory on your stories page and look for the field called `Memory Recall Value`.

![SetMemory](https://i.ibb.co/RDy6XHv/memory-graph.png)

The `Set Memory` function accepts three parameters:

* `Token`: A `String`. Token of the playthrough to set the memory on.
* `RecallValue`: A `String`. The recall value of the memory.
* `SaveValue`: A `String`. The value we wish to set the memory to.

![SetMemory](/Docs/SetMemory.png)


## Reference

All methods and events have a C++ and corresponding Blueprint event or function.

For methods relating to the Charisma HTTP API, consult the [APIBlueprints](/Source/CharismaModule/Public/APIBlueprints) folder. This currently includes:

- `CreateConversation`
- `CreatePlaythroughToken`
- `GetMessageHistory`
- `GetPlaythroughInfo`
- `RestartFromEventId`
- `SetMemory`

For most other methods relating to the Charisma play API (runtime chat events), consult the [Playthrough.h](/Source/CharismaModule/Public/Playthrough.h) file.
