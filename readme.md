# About DynRPG_Pathfeeder

#### Author
Dominic "DNKpp" Koepke

#### Download

#### Source
The complete source code is available at:
https://github.com/DNKpp/DynRPG_Pathfinder

## Introduction
This is a plugin written for the DynRPG 0.32. It is not possible to use it with previous versions like DynRPG 0.20, because the plugin makes
heavy usage of functions, which were introduced later on.
At first: Unlike my previous pathfinder plugin, I don't move any character myself. You have to handle the movement yourself. The plugin just
hands over the information you'll need.


## Documentation
Before we start with the function documentation itself, please read the following guidelines first:

<ul><li>Parameters starting with <b><code>in_...</code></b> hand over information to the function</li>
<li>Parameters starting with <b><code>out_...</code></b> hand back information from the plugin to the RM2k3 environment. Those out variables simply refer to internal variables or
switches of the RM2k3 via index (e.g. if you pass 3 to a parameter called "out_id", the variable with index 3 in the RM2k3 will be set).</li>
<li>Some parameters will have a clamped <b><code>..._s_...</code></b> in their name. This indicates that this parameter does refer to a switch instead of a variable.</li>
<li>If any of those functions have a parameter named "out_s_success" they are able to fail. When referring to any other out variable of that function, you <b>must</b> check the success
variable first to be sure to receive valid values. The success variable is the only parameter which will be either set to true or false. If the function fails, any other out parameter
won't be touched when the functions fails.</li>
<li>Most (if not all) parameters of the functions in this plugin are integers. To save some space and keep the documentation clean please keep in mind, that you are able to either pass a plain value (like 1, 2, -2, ...) or a indirect value
	through a RPG::variable (v1 refers to the RPG::variable with id 1; v2 to id 2; etc.). You are able to chain those leading 'v', thus you are able to perform multiple indirections (vv1 is looking into variable 1 and using this value as the final id).
	It is important to keep in mind, that, if not stated otherwise, ids starting at 1 (not 0).
</ul>

### Concept
This plugin consists of two parts: the configuration and the "pathfeeding".

Let's start with the interesting one. What is "pathfeeding" and how does this help you? As already stated in the introduction, this plugin won't touch your events. This plugin will simply calculate the best available path from one event to
a given destination. You are able to retrieve any information you will need to move your event; but it is up to you, how you do that. Like already mentioned, you trigger the pathfinding via event comment and after that you are able to
pull the information for the next step out of the plugin when you need it; again via event comment. If you still don't understand, there will be a example Game attached to the download.
One thing to be aware of: Each path you'll generate will be stored internally as long as you do not clear it via event comment or change the map. Every path stores a 4 byte size, and for each of its vertices, another 8 bytes. This means, if you generate
a path of 10 vertices you will consume 84 bytes of memory. If you do that often it quickly sums up to a huge amount of memory. For this reason it's important, especially when your game only runs on one map, to clear each path when you are done with it. 

The second part is about configuring the plugin. Currently you are only able to configure the cost of each terrain id. When you leave everything untouched the plugin will use the terrain id of each tile as its cost value, but you are able to
manipulate this behavior via DynRPG.ini and, again, event comment. The configuration via ini will be initially loaded when you start a new game. Every other modification of those value will be serialized to the save files.


### Interface

#### Find Path
	@pathfeeder_find_path in_target, in_x, in_y out_path_id, out_s_success

##### Details
This function is used to calculate the path from an event to a specific destination (x- and y-coordinate). The cost for each step is determined by the terrain id. To manipulate this cost look at the terrain_cost functions.

**!!Attention!!**  
This function may fail if the destination is not reachable. Please always check the success information before relying to the other output.

##### Params
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|													
| **in_target**		| Accepts event ids in integer style. Accepts also specific tokens for special events (hero, airship, ship, skiff).														|
| **in_x**			| Expects x-coordinate in integer style.																																|
| **in_y**			| Expects y-coordinate in integer style.																																|
| **out_path_id** 	| Expects an RPG::variable id in integer style. It will insert the id of the newly generated path in the variable at the provided index.								|
| **out_s_success**	| Expects RPG::switch id in integer style. It will use the provided value as switch index to hand back the result of the function (false = it failed; true = success).	|



#### Get Path Length
	@pathfeeder_get_path_length in_path_id, out_path_length, out_s_success
	
##### Details
Each path consists of several vertices (the map tiles). This function returns, how many of those tiles are included in this path (inclusive start and end tile).

**!!Attention!!**  
This function may fail if the path is not available. Please always check the success information before relying to the other output.

##### Params
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_path_id** 		| Expects path ids in integer style. Identifies the path.
| **out_length** 		| Expects an RPG::variable id in integer style. It will insert the length of the path at the passed index.
| **out_s_success** 	| Expects RPG::switch id in integer style. It will use the provided value as switch index to hand back the result of the function (false = it failed; true = success).



#### Get Path vertex
	@pathfeeder_get_path_vertex in_path_id, in_vertex_index, out_vertex_x, out_vertex_y, out_s_success

##### Details
This function returns information about one specific vertex. To get the right one, you have to determine which path you want to look at. Each path consists of several vertices (the map tiles), thus you have to tell the plugin
which specific vertex you are interested in. The x- and y-coordinate will be returned into a RPG::variable at the passed id.

**!!Attention!!**  
This function may fail if the path is not available or the vertex index is out of bounds. Please always check the success information before relying to the other output.

##### Params
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_path_id** 	| Expects path ids in integer style. Identifies the path.
| **in_vertex_index**| Expects vertex index (starting at 0) in integer style. Identifies the vertex you will get the components from.
| **out_vertex_x** 	| Expects an RPG::variable id in integer style. It will insert the x-coordinate value of the vertex at the provided index.
| **out_vertex_y** 	| Expects an RPG::variable id in integer style. It will insert the y-coordinate value of the vertex at the provided index.
| **out_s_success**	| Expects RPG::switch id in integer style. It will use the provided value as switch index to hand back the result of the function (false = it failed; true = success).


#### Clear Path
	@pathfeeder_clear_path in_path_id

##### Details
Every path you generated via this plugin will be stored internally to be able to provide the needed information when necessary. When you are done with the path, you should clear it to free memory for further tasks.

##### Params
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_path_id** | Expects path ids in integer style. Identifies the path.


#### Set Terrain Cost
	@pathfeeder_set_terrain_cost in_terrain_id, in_cost

##### Details
The cost for the specified terrain will be used for future path finding tasks. It will be cached internally until you reset the specific terrain or clear everything.

##### Params
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_terrain_id** | Expects terrain id in integer style. Identifies the terrain id the cost will be used for.
| **in_cost**		| Expects value in integer style. This will be used as cost of the given terrain id.


#### Set Terrain Cost Var
	@pathfeeder_set_terrain_cost_var in_terrain_id, in_var_id

##### Details
The RPG::variable id value will be used as dynamical cost for the specified terrain for future path finding tasks.

##### Params
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_terrain_id** | Expects terrain id in integer style. Identifies the terrain id the cost will be used for.
| **in_var_id**		| Expects an RPG::variable id in integer style. This will be used to dynamically lookup the cost value of the given terrain id.


#### Reset Terrain Cost
	@pathfeeder_reset_terrain_cost in_terrain_id

##### Details
The cost for the specified terrain will be reset to default (terrain id as cost).

##### Params
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_terrain_id** | Expects terrain id in integer style. Identifies the terrain id to be reset.


#### Clear Terrain Costs
	@pathfeeder_clear_terrain_costs

##### Details
Clears every cached cost value or variable.


#### Get Terrain Cost
	@pathfeeder_get_terrain_cost in_terrain_id, out_cost

##### Details
Returns the current cached terrain cost into the RPG::variable with the specified id.

##### Params
|					|																																										|
|-------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|	
| **in_terrain_id** | Expects terrain id in integer style. Identifies the terrain id to be reset.
| **out_cost**		| Expects an RPG::variable id in integer style. It will insert the cost of the terrain at the passed id


### Ini Layout
An .ini file consists of multiple section elements and multiple subordinated key/value pairs. The section name <b>pathfeeder</b> is reserved by this plugin. You configure the costs for your terrain ids, treat the ids as key and
the costs as values. You can also refer to a variable cost, all you have to do is to use a negative value (which will be treated internally as absolute id).

	[pathfeeder]
		5=10	// terrain id 5 has cost of 10
		6=-5	// terrain id 6 will use the value of RPG::variable id 5 as cost


### Technical Details
Due to the antique version of the gcc used by all the other plugins I tried my best to get around this. To be honest gcc was a pain to me, so I tried 2 other compilers: msvc and clang. Due to the different asm syntax I wasn't able to compile
the library with msvc; but clang did the job. As a result I was able to update the c++ version to c++17, which offers some huge conveniences. After some progress I realized, that clangs optimizer was to aggressive and simply kicked some code
from the binary, because it thought this wasn't necessary. But I was finally able to tell it via compiler flag <b>/Oy-</b> to be more patient and now it seems fine. This means, if you encounter anything weird, please don't hesitate to contact me.