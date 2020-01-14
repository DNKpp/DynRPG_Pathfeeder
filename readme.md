# About DynRPG_Pathfeeder

#### Author:
Dominic "DNKpp" Koepke 

#### Credits:
Documentation edited by Tor_Heyerdal

#### Download:
https://drive.google.com/open?id=1QUdEN8cP1tuUf4Y6wYeFs3sjN4ZHEWtO

#### Source:
The complete source code is available at:
https://github.com/DNKpp/DynRPG_Pathfinder

## Introduction:
This is a plugin written for DynRPG 0.32. It is not possible to use it with previous versions, like DynRPG 0.20, because the plugin makes heavy usage of functions which were introduced later on. Unlike my previous pathfinder plugin, this plugin does not move any characters itself. You have to handle the movement yourself. The plugin just hands over the information that you'll need.


## Documentation:
Before we start with the function documentation itself, please read the following guidelines first:

<ul><li>Parameters starting with <b><code>in_...</code></b> hand over information to the function for the plugin to use.</li>
<li>Parameters starting with <b><code>out_...</code></b> retrieve information from the plugin and feed it to the RM2k3 environment. Those "out" values simply refer to internal variables or switches of RM2k3 via index (e.g. if you pass 3 to a parameter called "out_id", the variable with index 3 (ie, var[0003]) in RM2k3 will be set).</li>
<li>Some parameters will have a clamped <b><code>..._s_...</code></b> in their name. This indicates that this parameter refers to a switch instead of a variable.</li>
<li>When any of these functions have a parameter named "out_s_success", they are able to fail. When referring to any other "out" variable of that function, you <b>must</b> check the success switch first to be sure that you will receive valid values. The success switch is the only parameter which will be set to either true or false. If the function fails, any other "out" parameter won't be touched and the function will abort.</li>
<li>Most (if not all) parameters of the functions in this plugin are integers. To save some space and keep the documentation clean, please keep in mind that you are able to either pass a plain value (like 1, 2, -2, ...) or an indirect value through an RPG::variable (v1 refers to the RPG::variable with ID 1; v2 to ID 2; etc.). You are able to chain those leading "v" prefixes, thus you are able to perform multiple indirections (vv1 looks into variable 1 and uses its value as the final ID). It is important to keep in mind that, unless stated otherwise, IDs start at 1 (not 0).
</ul>

### Concept:
This plugin consists of two parts: the configuration and the "pathfeeding".

Let's start with the interesting one. What is "pathfeeding" and how does this help you? As previously stated in the introduction, this plugin won't touch your events. This plugin will simply calculate the best available path from one event to a given destination and "feed" you that information. You are able to retrieve any information you will need to move your event, but it is up to you how you choose to do that. You trigger the pathfinding process via event comment and, after that, you are able to pull the information for the next step out of the plugin whenever you need it — again, via event comment. If you still don't understand, there will be an example game attached to the download. One thing to be aware of: Each path you'll generate will be stored internally as long as you do not clear it via event comment or change the map. Every path stores a 4 byte size, and for each of its vertices (ie, tiles), another 8 bytes. This means, if you generate a path of 10 vertices you will consume 84 bytes of memory. If you do this often, it quickly sums up to a huge amount of memory. That's why it's important, especially when your game only runs on one map, to clear each path when you are done with it.

The second part is about configuring the plugin. Currently, you are only able to configure the cost of each terrain ID. When you leave everything untouched, the plugin will use the terrain ID of each tile as its cost value, but you are able to manipulate this behavior via DynRPG.ini and, again, event comment. The configuration via .ini will be initially loaded when you start a new game. Every other modification of those values will be serialized to the save files. For additional information regarding the .ini configuration, see the Ini Layout section at the bottom of this document.


### Interface:

#### Find Path:
	@pathfeeder_find_path in_target, in_x, in_y out_path_id, out_s_success

##### Details:
This function is used to calculate the path from an event to a specific destination (x- and y-coordinates). The cost for each step is determined by the terrain ID. To manipulate this cost, look at the terrain_cost functions below.

**!!Attention!!**  
This function may fail if the destination is not reachable. Please, always check the success information before relying on any other output.

##### Params:
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|													
| **in_target**		| Expects event IDs as an integer. Also accepts specific tokens for special events (hero, airship, ship, skiff). The event for whom the path will be calculated.														|
| **in_x**			| Expects the X coordinate of the destination tile as an integer.																																|
| **in_y**			| Expects the Y coordinate of the destination tile as an integer.																																|
| **out_path_id** 	| Expects an RPG::variable ID as an integer. It will insert the ID of the newly generated path in the variable at the provided index.								|
| **out_s_success**	| Expects an RPG::switch ID as an integer. It will use the provided value as the switch index at which to feed you the function's result (false = it failed; true = success).	|



#### Get Path Length:
	@pathfeeder_get_path_length in_path_id, out_path_length, out_s_success
	
##### Details:
Each path consists of several vertices (the map tiles). This function returns how many of those tiles are included in this path (including the start and end tiles).

**!!Attention!!**  
This function may fail if the path is not available. Please, always check the success information before relying on any other output.

##### Params:
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_path_id** 		| Expects a path ID as an integer. Identifies the path whose length we are retrieving.
| **out_length** 		| Expects an RPG::variable ID as an integer. The length of the path will be fed to the specified index.
| **out_s_success** 	| Expects an RPG::switch ID as an integer. It will use the provided value as the switch index at which to feed you the function's result (false = it failed; true = success).



#### Get Path vertex:
	@pathfeeder_get_path_vertex in_path_id, in_vertex_index, out_vertex_x, out_vertex_y, out_s_success

##### Details:
This function returns information about one specific vertex along the generated path. To get the right one, you have to determine which path you want to look at by providing the plugin with the path ID of the path you wish to query. Each path consists of several vertices (the map tiles), thus you have to tell the plugin which specific vertex you are interested in. Note that vertex 0 is the starting tile from which the generated path is begun, vertex 1 is the tile of the first step, vertex 2 is the tile of the second step, and etc. The X and Y coordinates of the queried vertex will be fed into RPG::variables at the specified IDs.

**!!Attention!!**  
This function may fail if the path is not available or the specified vertex index is out of bounds. Please, always check the success information before relying on any other output.

##### Params:
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_path_id** 	| Expects a path ID as an integer. Identifies the path which contains the vertex we wish to query.
| **in_vertex_index**| Expects a vertex index (starting at 0) as an integer. Identifies which vertex (ie, which tile along the generated path) to retrieve information about.
| **out_vertex_x** 	| Expects an RPG::variable ID as an integer. It will use the provided value as the variable index at which to feed you the X coordinate value of the specified vertex.
| **out_vertex_y** 	| Expects an RPG::variable ID as an integer. It will use the provided value as the variable index at which to feed you the Y coordinate value of the specified vertex.
| **out_s_success**	| Expects an RPG::switch ID as an integer. It will use the provided value as the switch index at which to feed you the function's result (false = it failed; true = success).


#### Clear Path:
	@pathfeeder_clear_path in_path_id

##### Details:
Every path you generate via this plugin will be stored internally so as to be able to provide the needed information when necessary. When you are done with the path, you should clear it to free memory for further tasks.

##### Params:
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_path_id** | Expects a path ID as an integer. Identifies the path to be cleared from memory.


#### Set Terrain Cost:
	@pathfeeder_set_terrain_cost in_terrain_id, in_cost

##### Details:
Changes the cost of a terrain ID. The cost for the specified terrain will be used for future pathfinding tasks. It will be cached internally until you reset the specific terrain or clear everything.

##### Params:
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_terrain_id** | Expects a terrain ID as an integer. Identifies the ID of the terrain type whose cost should be changed.
| **in_cost**		| Expects a value as an integer. This will become the new cost of the specified terrain ID.


#### Set Terrain Cost Var:
	@pathfeeder_set_terrain_cost_var in_terrain_id, in_var_id

##### Details:
Permanently associates the specified terrain ID's cost with the specified RPG::variable ID. The designated RPG::variable will be used as a dynamic cost for the specified terrain in future pathfinding tasks until specified otherwise with another <code>set_terrain_cost</code> or <code>set_terrain_cost_var</code> command. If the value of the associated RPG::variable ever changes, so too will the cost of the specified terrain. 

##### Params:
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_terrain_id** | Expects a terrain ID as an integer. Identifies the ID of the terrain type whose cost should become associated with the variable.
| **in_var_id**		| Expects an RPG::variable ID as an integer. This variable will be used to dynamically look up the cost value of the specified terrain ID.


#### Reset Terrain Cost:
	@pathfeeder_reset_terrain_cost in_terrain_id

##### Details:
The cost for the specified terrain will be reset to its default value (terrain ID as cost).

##### Params:
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_terrain_id** | Expects a terrain ID as an integer. Identifies the terrain ID to be reset.


#### Clear Terrain Cost:
	@pathfeeder_clear_terrain_costs

##### Details:
Feeds the current cached cost of the specified terrain ID into the specified RPG::variable ID.


#### Get Terrain Cost:
	@pathfeeder_get_terrain_cost in_terrain_id, out_cost

##### Details
Returns the current cached terrain cost into the RPG::variable with the specified id.

##### Params
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_terrain_id** | Expects a terrain ID as an integer. Identifies the terrain ID whose cost should be retrieved.
| **out_cost**		| Expects an RPG::variable ID as an integer. The cost of the specified terrain will be fed to the specified variable index.


### Ini Layout:
An .ini file consists of multiple section elements and multiple subordinated key/value pairs. The section name "pathfeeder" is reserved by this plugin. You can configure the costs for your terrain IDs here. Treat the IDs as keys and the costs as values. You can also refer a value to a variable cost. All you have to do is to use a negative value (which will be treated internally as absolute ID).

	[pathfeeder]
		5=10	// terrain ID 5 has cost of 10
		6=-5	// terrain ID 6 will use the value of RPG::variable ID 5 as cost


### Technical Details:
The version of the GCC which is used by all the other plugins is very antique. I tried my best to get around this. To be honest, I've found GCC to be a pain. So I tried 2 other compilers: MSVC and Clang. Due to the different ASM syntax, I wasn't able to compile the library with MSVC, but Clang did the job. As a result, I was able to update the C++ version to C++17, which offers some huge conveniences. After some progress, I realized that Clang's optimizer was too aggressive and outright kicked some code from the binary, because it thought it wasn't necessary. But I was finally able to tell it via compiler flag /Oy- to be more patient, and now it seems fine. But, just in case, if you encounter anything weird, please don't hesitate to contact me.