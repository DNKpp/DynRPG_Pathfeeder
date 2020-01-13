/*!
	\mainpage About DynRPG_Pathfeeder

	\author Dominic "DNKpp" Koepke

	\section download Download
	<b><a href=""></a></b>

	\section source Source
	The complete source code is available at:
	<b>https://github.com/DNKpp/DynRPG_Pathfinder<a href="https://github.com/DNKpp/DynRPG_Pathfinder"></a></b>

	\section contact Contact
	If you encounter any issues or bugs, feel free to contact me at <b>DNKpp2011@gmail.com<a href="DNKpp2011@gmail.com"></a></b>.

	\section introduction Introduction
	This is a plugin written for the DynRPG 0.32. It is not possible to use it with previous versions like DynRPG 0.20, because the plugin makes
	heavy usage of functions, which were introduced later on.
	At first: Unlike my previous pathfinder plugin, I don't move any character myself. You have to handle the movement yourself. The plugin just
	hands over the information you'll need.

	<hr>

	\section documentation Documentation
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

	\subsection Concept
	This plugin consists of two parts: the configuration and the "pathfeeding".

	Let's start with the interesting one. What is "pathfeeding" and how does this help you? As already stated in the introduction, this plugin won't touch your events. This plugin will simply calculate the best available path from one event to
	a given destination. You are able to retrieve any information you will need to move your event; but it is up to you, how you do that. Like already mentioned, you trigger the pathfinding via event comment and after that you are able to
	pull the information for the next step out of the plugin when you need it; again via event comment. If you still don't understand, there will be a example Game attached to the download.
	One thing to be aware of: Each path you'll generate will be stored internally as long as you do not clear it via event comment or change the map. Every path stores a 4 byte size, and for each of its vertices, another 8 bytes. This means, if you generate
	a path of 10 vertices you will consume 84 bytes of memory. If you do that often it quickly sums up to a huge amount of memory. For this reason it's important, especially when your game only runs on one map, to clear each path when you are done with it. 

	The second part is about configuring the plugin. Currently you are only able to configure the cost of each terrain id. When you leave everything untouched the plugin will use the terrain id of each tile as its cost value, but you are able to
	manipulate this behavior via DynRPG.ini and, again, event comment. The configuration via ini will be initially loaded when you start a new game. Every other modification of those value will be serialized to the save files.

	<hr>

	\subsection Interface

	\subsubsection find_path Find Path
	\code @pathfeeder_find_path in_target, in_x, in_y out_path_id, out_s_success
	\endcode

	\details This function is used to calculate the path from an event to a specific destination (x- and y-coordinate). The cost for each step is determined by the terrain id. To manipulate this cost look at the terrain_cost functions.

	\attention This function may fail if the destination is not reachable. Please always check the success information before relying to the other output.

	@param in_target Accepts event ids in integer style. Accepts also specific tokens for special events (hero, airship, ship, skiff).
	@param in_x Expects x-coordinate in integer style.
	@param in_y Expects y-coordinate in integer style.
	@param out_path_id Expects an RPG::variable id in integer style. It will insert the id of the newly generated path in the variable at the provided index.
	@param out_s_success Expects RPG::switch id in integer style. It will use the provided value as switch index to hand back the result of the function (false = it failed; true = success).



	\subsubsection get_path_length Get Path Length
	\code @pathfeeder_get_path_length in_path_id, out_path_length, out_s_success
	\endcode

	\details Each path consists of several vertices (the map tiles). This function returns, how many of those tiles are included in this path (inclusive start and end tile).

	\attention This function may fail if the path is not available. Please always check the success information before relying to the other output.

	@param in_path_id Expects path ids in integer style. Identifies the path.
	@param out_length Expects an RPG::variable id in integer style. It will insert the length of the path at the passed index.
	@param out_s_success Expects RPG::switch id in integer style. It will use the provided value as switch index to hand back the result of the function (false = it failed; true = success).



	\subsubsection get_path_vertex Get Path vertex
	\code @pathfeeder_get_path_vertex in_path_id, in_vertex_index, out_vertex_x, out_vertex_y, out_s_success
	\endcode

	\details This function returns information about one specific vertex. To get the right one, you have to determine which path you want to look at. Each path consists of several vertices (the map tiles), thus you have to tell the plugin
		which specific vertex you are interested in. The x- and y-coordinate will be returned into a RPG::variable at the passed id.

	\attention This function may fail if the path is not available or the vertex index is out of bounds. Please always check the success information before relying to the other output.

	@param in_path_id Expects path ids in integer style. Identifies the path.
	@param in_vertex_index Expects vertex index (starting at 0) in integer style. Identifies the vertex you will get the components from.
	@param out_vertex_x Expects an RPG::variable id in integer style. It will insert the x-coordinate value of the vertex at the provided index.
	@param out_vertex_y Expects an RPG::variable id in integer style. It will insert the y-coordinate value of the vertex at the provided index.
	@param out_s_success Expects RPG::switch id in integer style. It will use the provided value as switch index to hand back the result of the function (false = it failed; true = success).


	\subsubsection clear_path Clear Path
	\code @pathfeeder_clear_path in_path_id
	\endcode

	\details Every path you generated via this plugin will be stored internally to be able to provide the needed information when necessary. When you are done with the path, you should clear it to free memory for further tasks.

	@param in_path_id Accepts path ids as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Identifies the path.


	\subsubsection set_terrain_cost Set Terrain Cost
	\code @pathfeeder_set_terrain_cost in_terrain_id, in_cost
	\endcode

	\details The cost for the specified terrain will be used for future path finding tasks. It will be cached internally until you reset the specific terrain or clear everything.

	@param in_terrain_id Expects terrain id in integer style. Identifies the terrain id the cost will be used for.
	@param in_cost Expects value in integer style. This will be used as cost of the given terrain id.


	\subsubsection set_terrain_cost_var Set Terrain Cost Var
	\code @pathfeeder_set_terrain_cost_var in_terrain_id, in_var_id
	\endcode

	\details The RPG::variable id value will be used as dynamical cost for the specified terrain for future path finding tasks.

	@param in_terrain_id Expects terrain id in integer style. Identifies the terrain id the cost will be used for.
	@param in_var_id Expects an RPG::variable id in integer style. This will be used to dynamically lookup the cost value of the given terrain id.


	\subsubsection reset_terrain_cost Reset Terrain Cost
	\code @pathfeeder_reset_terrain_cost in_terrain_id
	\endcode

	\details The cost for the specified terrain will be reset to default (terrain id as cost).

	@param in_terrain_id Expects terrain id in integer style. Identifies the terrain id to be reset.


	\subsubsection clear_terrain_costs Clear Terrain Costs
	\code @pathfeeder_clear_terrain_costs
	\endcode

	\details Clears every cached cost value or variable.


	\subsubsection get_terrain_cost Get Terrain Cost
	\code @pathfeeder_get_terrain_cost in_terrain_id, out_cost
	\endcode

	\details Returns the current cached terrain cost into the RPG::variable with the specified id.

	@param in_terrain_id Expects terrain id in integer style. Identifies the terrain id to be reset.
	@param out_cost Expects an RPG::variable id in integer style. It will insert the cost of the terrain at the passed id

	<hr>


	\subsection technical_details Technical Details
	Due to the antique version of the gcc used by all the other plugins I tried my best to get around this. To be honest gcc was a pain to me, so I tried 2 other compilers: msvc and clang. Due to the different asm syntax I wasn't able to compile
	the library with msvc; but clang did the job. As a result I was able to update the c++ version to c++17, which offers some huge conveniences. After some progress I realized, that clangs optimizer was to aggressive and simply kicked some code
	from the binary, because it thought this wasn't necessary. But I was finally able to tell it via compiler flag <b>/Oy-</b> to be more patient and now it seems fine. This means, if you encounter anything weird, please don't hesitate to contact me.
*/