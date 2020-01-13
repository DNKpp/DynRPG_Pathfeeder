/*!
	\mainpage About DynRPG_Pathfinder

	\author Dominic "DNKpp" Koepke

	\section download Download
	<b><a href=""></a></b>

	\section contact Contact
	If you encounter any issues or bugs, feel free to contact me at <b><a href="DNKpp2011@gmail.com"></a></b>.

	\section introduction Introduction
	This is a plugin written for the DynRPG 0.32. It is not possible to use it with previous versions like DynRPG 0.20, because the plugin makes
	heavy usage of functions, which were introduced later on.
	At first: Unlike my previous pathfinder plugin, I don't move any character myself. You have to handle the movement yourself. The plugin just
	hands over the information you'll need.

	\section documentation Documentation
	Before we start with the function documentation itself, please read the following guidelines first:

	<ul><li>Parameters starting with <b><code>in_...</code></b> hand over information to the function</li>
	<li>Parameters starting with <b><code>out_...</code></b> hand back information from the plugin to the RM2k3 environment. Those out variables simply refer to internal variables or
	switches of the RM2k3 via index (e.g. if you pass 3 to a parameter called "out_id", the variable with index 3 in the RM2k3 will be set).</li>
	<li>Some parameters will have a clamped <b><code>..._s_...</code></b> in their name. This indicates that this parameter does refer to a switch instead of a variable.</li>
	<li>If any of those functions have a parameter named "out_s_success" they are able to fail. When referring to any other out variable of that function, you <b>must</b> check the success
	variable first to be sure to receive valid values. The success variable is the only parameter which will be either set to true or false. If the function fails, any other out parameter
	won't be touched when the functions fails.</li>
	</ul>

	\subsection Interface

	\subsubsection find_path Find Path
	\code @pathfeeder_find_path in_target, in_x, in_y out_path_id, out_s_success
	\endcode

	\details This function is used to calculate the path from an event to a specific destination (x- and y-coordinate). The cost for each step is determined by the terrain id. To manipulate this cost look at the terrain_cost functions.

	\attention This function may fail if the destination is not reachable. Please always check the success information before relying to the other output.

	@param in_target Accepts event ids as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Accepts also specific tokens for special events (hero, airship, ship, skiff).
	@param in_x Accepts x-coordinate as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...).
	@param in_y Accepts y-coordinate as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...).
	@param out_path_id Accepts an id for RPG::variables as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). It will insert the id of the newly generated path in the variable at the provided index.
	@param out_s_success Accepts id for RPG::switches as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). It will use the provided value as switch index to hand back the result of the function (false = it failed; true = success).


	\subsubsection get_path_length Get Path Length
	\code @pathfeeder_get_path_length in_path_id, out_path_length, out_s_success
	\endcode

	\details Each path consists of several vertices (the map tiles). This function returns, how many of those tiles are included in this path (inclusive start and end tile).

	\attention This function may fail if the path is not available. Please always check the success information before relying to the other output.

	@param in_path_id Accepts path ids as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Identifies the path.
	@param out_length Accepts an id for RPG::variables as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). It will insert the length of the path at the passed index.
	@param out_s_success Accepts id for RPG::switches as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). It will use the provided value as switch index to hand back the result of the function (false = it failed; true = success).


	\subsubsection get_path_vertex Get Path vertex
	\code @pathfeeder_get_path_vertex in_path_id, in_vertex_index, out_vertex_x, out_vertex_y, out_s_success
	\endcode

	\details This function returns information about one specific vertex. To get the right one, you have to determine which path you want to look at. Each path consists of several vertices (the map tiles), thus you have to tell the plugin
		which specific vertex you are interested in. The x- and y-coordinate will be returned into a RPG::variable at the passed id.

	\attention This function may fail if the path is not available or the vertex index is out of bounds. Please always check the success information before relying to the other output.

	@param in_path_id Accepts path ids as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Identifies the path.
	@param in_vertex_index Accepts vertex index (starting at 0) as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Identifies the path of which you will get the length for.
	@param out_vertex_x Accepts an id for RPG::variables as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). It will insert the x-coordinate value of the vertex at the provided index.
	@param out_vertex_y Accepts an id for RPG::variables as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). It will insert the y-coordinate value of the vertex at the provided index.
	@param out_s_success Accepts id for RPG::switches as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). It will use the provided value as switch index to hand back the result of the function (false = it failed; true = success).


	\subsubsection clear_path Clear Path
	\code @pathfeeder_clear_path in_path_id
	\endcode

	@param in_path_id Accepts path ids as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Identifies the path.

	\details Every path you generated via this plugin will be stored internally to be able to provide the needed information when necessary. When you are done with the path, you should clear it to free memory for further tasks.


	\subsubsection set_terrain_cost Set Terrain Cost
	\code @pathfeeder_set_terrain_cost in_terrain_id, in_cost
	\endcode

	\details The cost for the specified terrain will be used for future path finding tasks. It will be cached internally until you reset the specific terrain or clear everything.

	@param in_terrain_id Accepts value as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Identifies the terrain id the cost will be used for.
	@param in_cost Accepts value as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). This will be used as cost of the given terrain id.


	\subsubsection set_terrain_cost_var Set Terrain Cost Var
	\code @pathfeeder_set_terrain_cost_var in_terrain_id, in_var_id
	\endcode

	\details The RPG::variable id value will be used as dynamical cost for the specified terrain for future path finding tasks.

	@param in_terrain_id Accepts value as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Identifies the terrain id the cost will be used for.
	@param in_var_id Accepts value as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). This will be used as an id to a RPG::variable, which holds the cost of the given terrain id.


	\subsubsection reset_terrain_cost Reset Terrain Cost
	\code @pathfeeder_reset_terrain_cost in_terrain_id
	\endcode

	\details The cost for the specified terrain will be reset to default (terrain id as cost).

	@param in_terrain_id Accepts value as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Identifies the terrain id to be reset.


	\subsubsection clear_terrain_costs Clear Terrain Costs
	\code @pathfeeder_clear_terrain_costs
	\endcode

	\details Clears every cached cost value.


	\subsubsection get_terrain_cost Get Terrain Cost
	\code @pathfeeder_get_terrain_cost in_terrain_id, out_cost
	\endcode

	\details Returns the current cached terrain cost into the RPG::variable with the specified id.

	@param in_terrain_id Accepts value as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). Identifies the terrain id to be reset.
	@param out_cost Accepts value as plain number (1, 02, 500, ...) or as variable (v1, vV2, V3, ...). It will insert the cost of the terrain at the passed index.
*/