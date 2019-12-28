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

	\code @find_path out_id, out_s_success
	\endcode
*/