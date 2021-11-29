# Charisma SDK for Unreal Engine 4

This Unreal Engine plugin is verified to work for engine version `4.26` only. If you find the plugin also works in another version, feel free to submit a pull request to update this!

If you have any questions or need a hand, please reach out to us at [hello@charisma.ai](mailto:hello@charisma.ai)!

## Getting started

**Important:** Before setting up the Charisma SDK for UE4, you’ll need to have created a web or game story (not an app story!), which requires a Charisma licence. Please visit the [licencing docs on our website](https://charisma.ai/docs/licencing) for more info!

1. If you haven’t already, go ahead and create an Unreal project! Make sure you are using a version of the engine that is supported by this plugin.

2. Move the contents of this repository to the `/Plugins` folder located at the root of your project. If this folder doesn’t exist, create it!

3. Close the UE4 editor and rebuild the project in Visual Studio.

4. When you next open the editor, verify the plugin loaded correctly by navigating to `Settings -> Plugins` and locating the Charisma plugin. It should look something like this:

![StoryId](https://i.ibb.co/6Y5qyK6/charisma-plugin.png)

5. You can now start using Charisma in UE4! 🎉

## Usage (Blueprints)

We’re working on a new version of the documentation for this plugin. In the meanwhile, to read about the usage of Charisma in UE4 blueprints, [please refer to this document](https://docs.google.com/document/d/1z64Xhe9ij9hpjum1bOzfNEQ8bhwmtHLdiXOlgYsjwFY/edit?usp=sharing).

In order to play Charisma stories from UE4 you’ll need a `StoryId`, and to play the draft (unpublished) version of your story, you’ll also need an `ApiKey`.

These parameters are then passed to the `CreatePlaythroughToken` method to create a new playthrough for each of your players to play the story within!

### Story ID

The `StoryId` is the unique ID of the story that you want to play. To find this, navigate to your story on the Charisma website, and copy the ID number after `/stories` in the URL.

![StoryId](https://i.ibb.co/sPqS9n2/StoryId.png)

### API key

To create a playthrough that references the draft version of your story, you also need to provide an `ApiKey`. This can be found on the 'Story Overview' page.

**Important:** Please make sure to not share the API key with anyone you do not trust, and strip the API key from any public builds.

![API key](https://i.ibb.co/X86bNVK/API-key.png)

### Playing a story

To start playing a Charisma story from within UE4 you’ll first need to generate a playthrough token using `CreatePlaythroughToken`, which has the following parameters.

- `StoryId` (`int32`): The `id` of the story that you want to create a new playthrough for. The story must be published, unless a Charisma.ai user token has been passed and the user matches the owner of the story.
- `Version` (`int32`, optional): The `version` of the story that you want to create a new playthrough for.
  - To play the latest published version, pass `0`.
  - To play a specific, published, version of your story, set the number to that particular version.
  - To play the draft version of a story, pass `-1` and supply an `ApiKey` to the `Charisma` object before calling the `CreatePlaythroughToken` method.
