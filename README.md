# BreakoutVK

by Dan Moody

Breakout clone that uses a Vulkan-based renderer.

The game only runs on 64-bit windows machines.

If you have any feedback/criticism of the code then please let me know at: daniel.guy.moody@gmail.com. I want to learn from my mistakes.


Project goals:
==============
+ Make a game, no matter how simple, that uses a custom vulkan engine/renderer that I wrote (just make something!).
+ Have local high-scores. It requires bringing together several different sections of the codebase to one place, how well can you handle this?
+ Make a mock standard library and learn more about what exactly STL is doing under the hood.
+ Learn as much as possible (within the scope of the project) about all of the systems of a game that I don't have much/any experience working with (binary file serialization over ascii, for example) and implement them as cleanly as possible.


Post-mortem
===========
Obviously there's a lot that could be done much better here. This project was really only done so I could make a game using an engine I wrote. It may be one of the most basic and rudimentary games to make, but I suppose it's a starting point (additionally, given the state some of the code is in it won't exactly be hard to do worse).

Renderer
--------
In retrospect the decision to use SDL was a mistake. Next time I'll be rolling my own window handler/io sub-system. SDL just started making me follow more weird rules as time went on to the extent that doing it myself would be much more beneficial. I've already half-abandoned use of SDL with things like my own file io handling. The library size itself is fairly big in comparison with the rest of the game (especially considering how much of it I don't use/call directly).

Overall, the Vulkan backend code came out well enough. I went through 3 different iterations of architecture. One C-style, the current C++ style, and a C/C++ mix which was much more akin to a D3D11-style interface with regards to how it accessed data. I think I made the right choice sticking with a C++-style implementation. There's a minimal additional memory overhead cost with additional class references and the code is much easier to work with/understand from a user's perspective. It is much more intuitive.

At one point I wanted to separate the main Vulkan context and the swap chain code in two, but the code became much uglier with duplicate data references in several places so I abandoned this approach. The final code is much simpler and cleaner than this. Abandoning that approach was a good decision.

Ideally, the draw calls for rendering the blocks would be batched, but I'm not sure how nicely this would fit (if at all) with storing uniform data in one large dynamic uniform buffer.

The render state/uniform layout code is probably the worst code in the entire Vulkan backend. This is definitely one of the highest priorities of things to fix/improve for the next game. Whatever that system is it needs to handle creation, swapping, and destruction of render states and let the user specify what they want via individual settings. However, there is benefit in things like manual handling of shader state (as Vulkan throws errors if you try to allocate the same shader twice). The main idea of separating uniform memory/resources from the main pipeline itself is still beneficial because it's entirely possible to have 2 separate pipelines pool the exact same uniform memory. The proposed changes here would be that the new manager that gets implemented would just handle some this internally to make things easier when dealing with sub-resources.

Game code
---------
The main game code itself is pretty crap on the whole, but that's because a lot of my focus was spent on the actual Vulkan backend as well as the mock standard library. This was predominantly a learning exercise and to see how good of a job I could do. I think I got a lot of value out of doing it and I'm enjoying the results.

Both the input and sound manager code sucks terribly. Both of those need to be done a hell of a lot better next time. The input manager needs to be able to support individual keys that can be re-bound at runtime. The sound manager needs to just support more things that you would expect a sound system to support (stems, for example).

A better implementation of the game entities is definitely doable. It is not flexible/future-proof enough. But the entities in this game are so similar and basic that I decided to just have one class for all base data and just call different functions in the main game class depending on what behaviour I wanted from the entities (if any). It works well enough, but isn't robust enough to be carried on in any serious capacity.

The scores manager lacks from not having a proper file serialization setup. Currently it's all done inline and therefore probably gives way to a large margin for error/tripping over myself - another thing to add to the list of things to improve.

Continuous collision detection for the ball is probably needed ultimately because technically the ball is capable of speeding up indefinitely. This is a potential bug in the game right now.

Ideally calls to the UI wouldn't be made alongside calls to ImGui. The UI would abstract ImGui away and be as library agnostic as possible. This needs to change for the next project because it's very ugly.

There's probably a lot more wrong, but I can't think of what those things are right now. I'm open to any feedback criticism you may have. Given the amount of flaws I've found from just a quick skim over I probably would've benefitted from spending maybe another couple weeks on this just to iron-out these things and do more things right.


Special Thanks to:
==================
+ Adrian Hirst
+ Dustin H. Land
+ Sascha Willems