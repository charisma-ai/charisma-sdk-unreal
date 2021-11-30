# Charisma SDK for Unreal Engine 4

This Unreal Engine plugin is verified to work with C++ projects using engine version `4.26` only. If you find the plugin also works in another version, feel free to submit a drag request to update this!

If you have any questions or need a hand, please reach out to us at [hello@charisma.ai](mailto:hello@charisma.ai)!

## Getting started

**Important:** Before setting up the Charisma SDK for UE4, you’ll need to have created a web or game story (not an app story!), which requires a Charisma licence. Please visit the [licencing docs on our website](https://charisma.ai/docs/licencing) for more info!

1. If you haven’t already, go ahead and create an Unreal **C++** project. Make sure you are using a version of the engine that is supported by this plugin.

2. Head to the [Releases page](https://github.com/charisma-ai/charisma-sdk-ue4/releases) of this GitHub repository and download the latest release. Unzip the contents into the `/Plugins` folder located at the root of your project. If this folder doesn’t exist, create it!

3. Close the UE4 editor and rebuild the project in Visual Studio.

4. When you next open the editor, verify the plugin loaded correctly by navigating to `Settings -> Plugins` and locating the Charisma plugin. It should look something like this:

![StoryId](https://i.ibb.co/6Y5qyK6/charisma-plugin.png)

5. You can now start using Charisma in UE4! 🎉


## Usage

To create playthrough tokens, you’ll first need to find out your story ID, and optionally an API key and version of the story you want to play.

### Story ID

The `StoryId` is the unique ID of the story that you want to play. To find this, navigate to your story on the Charisma website, and copy the ID number after `/stories` in the URL.

![StoryId](https://i.ibb.co/TcxRM8J/story-id.png)

### API key

To create a playthrough that references the draft version of your story, you also need to provide an `ApiKey`. This can be found on the 'Story Overview' page.

**Important:** Please make sure to not share the API key with anyone you do not trust, and strip the API key from any public builds.

![API keys](https://i.ibb.co/sQcDZYg/api-keys.png)

### Story Version

* To play the latest published version, pass `0`.
* To play a specific, published, version of your story, set the number to that particular version.
* To play the draft version of a story, pass `-1` and supply an `ApiKey` to the `Charisma` object before calling the `CreatePlaythroughToken` method.


## Setup

Charisma functionality can be added to any Blueprint actor in your level by creating a Charisma object that lives on the specified actor.

In this example, we are going to create a Blueprint actor to house Charisma in our level. The actor will call start on a story as soon as the level that we place it in starts. Ultimately, how you use Charisma in Unreal is up to you!

* In your project’s `Content` folder create a new folder called `Charisma`. In the Charisma folder create a new Blueprint by right-clicking in the folder and selecting `Blueprint Class`. When asked to pick a parent class, select `Actor`.

* Rename your new actor `BP_Charisma`. If you select the `Charisma` folder your project should now look like this:

![BPCharisma](https://i.ibb.co/xLq07Pj/BP-Charisma.png)

* Open `BP_Charisma` by double-clicking it. You should be greeted by the Blueprint graph containing the `BeginPlay`, `ActorBeginOverlap` and `Tick` events.

* From the `BeginPlay` event, drag an edge out from the execution pin and in the context menu, search for `Create Charisma Object` and select the function called `Create Charisma Object`.

![CharismaObject](https://i.ibb.co/VT39KRL/create-charisma-object.png)

* On the newly-created node, drag out the `Value` pin and from the context menu, select `Promote to variable`. Call this new variable `Charisma`.

* Also drag out from the `Owner` pin and type `Reference to self` into the context menu. Select `Reference to self`.

Your graph should now look like this:

![PromoteToVariable](https://i.ibb.co/HCZpXxS/promote-to-variable.png)

Let’s create a few more variables in `BP_Charisma` that we are going to need later. 

* Create a variable called `PlaythroughToken` and set it to be of type `String`.
* Create a variable called `PlaythroughId` and set it to be of type `Integer`.
* Create a variable called `ConversationID` and set it to be of type `Integer`.

Your variables tab should now look like this.

![Variables](https://i.ibb.co/8mjd9f9/charisma-variables.png)

* Since we are going to play the draft version of our story we also want to set our API Key. 

* From the `Set Charisma` node, drag out from the blue pin on the right-hand side, and search for `Set Api Key`. Select `Set Api Key`. In the field called `Api Key` on the newly-created node, paste the API key associated with your story. For more info on API keys, please see the [API key section of this documentation](#api-key).

* Make sure you also connect the white execution pin into the `Set Api Key` node, like this:

![ApiKey](https://i.ibb.co/M1XcTsz/api-key.png)

* We now need to hook up the events that exist on our `Charisma` variable to our Blueprint. These events notify us of things happening on the server-side.

* To start off with, we need to create a `PlaythroughToken`. First off, click the `Charisma` variable and drag it out into the graph. Select `Get Charisma`.

* From the blue pin on the `Get Charisma` node, drag out, and search for `Assign On Token Created`. Select `Assign On Token Created`.

* You should now see a red event node appear in the graph. Let’s call this new event `OnTokenCreated`.

* The `OnTokenCreated` event returns two variables. A `String` (`Token`) and an `Integer` (`PlaythroughId`). Remember the variables we created earlier? Let’s assign the data coming from the event to these variables so that we cache them for later. Your graph should look like this:

![OnToken](https://i.ibb.co/Y2BG8Ck/on-token-created.png)

* At this point we can create our `PlaythroughToken`. From the `Get Charisma` node, again, drag out from the blue pin and search for the `CreatePlaythroughToken` function. Select `CreatePlaythroughToken`. This function takes two parameters. One `Integer` called `StoryId` and another called `StoryVersion`. For information on how to retrieve your Story Id and select the correct version, please see the [Story Id section of this documentation](#story-id). For information on Story Version, see the [Story Version section of this documentation](#story-version).

* Input your Story Id and Story Version in the `CreatePlaythroughToken` node. 

* From the `BindEventToOnTokenCreated` node, drag the execution pin to the `CreatePlaythroughToken` node. It is important this happens in this order since we want to subscribe to the event before we call the `CreatePlaythroughToken` function. 

Your graph should look like this (a few re-route nodes have been added to clean things up): 

![CreateToken](https://i.ibb.co/m4CBZmM/create-playthrough-token.png)

* At this point we want to create our Conversation Id. Similarly to how we hooked up the `OnTokenCreated` event. We want to drag out from the blue pin on the `Get Charisma` node and search for `Assign On Conversation Created` in the context menu. Select `Assign On Conversation Created`. 

* You should now see a a red event node appear in the graph. Let’s call it `OnConversationCreated`.

* The `OnConversationCreated` event returns on variable of type `Integer`; `ConversationId`. Let’s assign this value to the `ConversationId` variable we created earlier.

Your graph should look like this:

![OnConversationCreated](https://i.ibb.co/pRFq6pp/on-converstaion-created.png)

* At this point we are ready to connect to Charisma.

* Drag out from the blue pin on the `Get Charisma` node and search for `Connect`. Select `Connect`. From the `Set ConversationId` node, drag out on the execution pin and connect it to the `Connect` node.

* The `Connect` node takes two arguments. A `Token` and a `PlaythroughId`. We are going to connect our `PlaythroughToken` and `PlaythroughId` variables. Drag each of them out from the variables list into the graph and select the `Get` function for each of them. Connect them to the corresponding field on the connect node.

* We are not done quite yet though. Before we connect we should subscribe to the `OnConnected` and `OnReady` events. 

* From the `Get Charisma` node, drag out from the blue pin and search for `Assign On Connected` in the context menu. Select `Assign On Connected`. Let’s call the new event `OnConnected`. Drag out from the execution pin on the `Bind Event to On Conversation Created` node and connect it to the `Bind Event to On Connected` node. 

* From the `Get Charisma` node, drag out from the blue pin and search for `Assign On Ready` in the context menu. Select `Assign On Ready`. Let’s call the new event `OnReady`. Drag out from the execution pin on the `Bind Event to On Connected` node and connect it to the `Bind Event to On Ready` node.

Your graph should look like this:

![OnConnect](https://i.ibb.co/KqgTkh5/on-connect.png)

* Navigate back to the folder containing our `BP_Charisma` actor and drag it into your scene. When you hit play, you should see `Connected` as well `Ready to begin play` logged in the top left corner of the viewport.

If you see an error instead, go back and make sure your `API Key`, `Story Id` and `Story Version` are correct.

At this point we are ready to start playing one of our graphs. 

* From the `Get Charisma` node, drag out on the blue pin and search for `Start` in the context menu. Select `Start`. Connect the execution pin of the `OnReady` event with the start node.   

* The `Start` node takes a few arguments. 
  * `ConversationId`: The specific conversation you wish to start. Let’s hook up the `ConversationId` variable we created earlier to this field.
  * `SceneIndex`: If you are referencing a non-game story, you will want to enter the index of the scene you want to start here. Otherwise leave this at `0`.
  * `StartGraphId`: If you are referencing a game story, you can enter the `Graph Id` of the scene you want to start here. You can find the `GraphId` in your browser’s URL when the specific graph is selected. Be aware that the `GraphId` will differ between draft and published versions of your story. If don’t want to have to change the `GraphId` whenever you switch between the draft and published versions of your story it’s recommended you use `StartGraphReferenceId` instead. If you wish to use another method for referencing your graph, leave this field at `0`.
  * `StartGraphReferenceId`: If you are referencing a game story you can use the `StartGraphReferenceId` to reference the scene you want to start. Unlike `StartGraphId`, the `StartGraphReferenceId` is the same across the draft and published versions of your story. To find the `StartGraphReferenceId` of the particular graph you want to start, locate the Subplot in the `Subplots` section of your story page and select rename. This will bring up a textbox containing your `StartGraphReferenceId`. If you wish to use another method for referencing your graph, leave this field empty.
  * `UseSpeech`: This is a `boolean` letting Charisma know whether or not you want to include the character voice data of the node in the messages you receive.

* Once you have found your graph identifier, enter it into the appropriate field on the `Start` node.

* In order for us to see the messages Charisma sends us, we should subscribe to the `OnMessage` event. From the `Get Charisma` node, drag out on the blue pin and search for `Assign On Message` in the context menu. Select `Assign On Message`. 

* Let’s call this new event `OnMessage`. Connect the execution pin of `Bind Event to On Ready` to the `Bind Event to On Message` node.

* The `OnMessage` event returns one variable of type `Charisma Message Event`. Drag out on this pin and release, and in the context menu, select `Break Charisma Message Event`.

Your graph should look like this:

![OnMessage](https://i.ibb.co/GkynQWw/on-message.png)

If you go back to your level and hit play, you should see the text of the first character node in your graph in the top-left corner of the viewport.

The `Charisma Message Event` contains all information about the particular node in your graph that fired the `OnMessage` event. What you choose to do with the data is up to you!

At this point you can start interacting with the graph by sending replies or actions based on input and events that take place in UE4.

### Replying

When you hit a player node in your graph you’ll want the player to be able to send a reply. To do this, call the `Reply` function on the Charisma object.

* The `Reply` function takes two arguments:
  * `ConversationId`: An `Integer`. Id of the specific conversation to reply to. 
  * `Message`: A `String`. The text we want to to send in our reply to Charisma. For example, this message could come from a UE4 text input box or from a speech-to-text service such as 'Google Speech To Text'.

![Reply](https://i.ibb.co/pzPd08s/reply.png)


### Actions

You can use action nodes in your graph to respond to actions the player takes in the game world. 

* The `Action` function takes two arguments:
  * `ConversationId`: An `Integer`. Id of the specific conversation to reply to.
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

* The `Set Memory` function takes two arguments:
  * `PlaythroughToken`: A `String`. Token of the playthrough to set the memory on.
  * `RecallValue`: A `String`. The recall value of the memory.
  * `SaveValue`: A `String`. The value we wish to set the memory to.

![SetMemory](https://i.ibb.co/f9PW2v4/memory.png)


## Reference

All methods and events have a corresponding Blueprint event or function.

**Events:**

* `FTokenDelegate OnTokenCreated;`

* `FConversationDelegate OnConversationCreated;`

* `FConnectionDelegate OnConnected;`

* `FReadyDelegate OnReady;`

* `FTypingDelegate OnTyping;`

* `FMessageDelegate OnMessage;`

* `FErrorDelegate OnError;`

**Methods:**

* `void CreatePlaythroughToken(const int32 StoryId, const int32 StoryVersion = 0) const;`

* `void CreateConversation(const FString& PlaythroughToken) const;`

* `void SetMemory(const FString& PlaythroughToken, const FString& RecallValue, const FString& SaveValue) const;`

* `void RestartFromEventId(const FString& PlaythroughToken, const int64 EventId) const;`

* `void Connect(const FString& Token, const int32 PlaythroughId);`

* `void Disconnect();`

* `void Action(const int32 ConversationId, const FString& ActionName) const;`

* `void Start(const int32 ConversationId, const int32 SceneIndex, const int32 StartGraphId, const FString& StartGraphReferenceId, const bool UseSpeech = false);`

* `void Reply(const int32 ConversationId, const FString& Message) const;`

* `void Resume(const int32 ConversationId) const;`

* `void Tap(const int32 ConversationId) const;`

* `void ToggleSpeechOn();`

* `void ToggleSpeechOff();`