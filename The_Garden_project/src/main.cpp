/*
* Author: Richard Borbely
* 3D scene - The Garden
* 40283185
* Note: Use ARROW KEYS to navigate menu
		
		To build: use CMAKE
*/
#include <glm\glm.hpp>
#include <graphics_framework.h>
#include <glm/ext.hpp>
#include "rendertype.h"
#include "postprocess.h"
using namespace std;
using namespace graphics_framework;
using namespace glm;

#pragma region Variables
//Render types
	map<string, RenderType> RenderTypes;	// Map that stores the RenderType and effects for meshes
//Scene
	map<string, mesh> meshes; map<string, texture> textures, normal_maps; map<string, effect> effects;
	directional_light light;
	shadow_map shadow;
	vector<point_light> points(7); vector<spot_light> spots(5);
	cubemap cube_map, cube_map2;
	frame_buffer scene;			// scene frame buffer
	free_camera cam_free;		// camNo 0
	target_camera cam_target_1;	// camNo 1
	target_camera cam_target_2;	// camNo 2
	int activeCamNo = 1;		// Assigned number for active camera
	double cursor_x = 0.0, cursor_y = 0.0;
//Planets
	float seed = 0.0f;	// variable that keeps increasing during runtime so that sin and cos waves can use it
	float x, z, y;		// planet trajectory // y for spotlight outside 
// PostProcess
	PostProcess PP;
	geometry screen, menu_geom;
	map<int, texture> menu_tex;
	bool renderMenu = true;
	float frameTime = 0.0f; // for noise, add delta time each update
#pragma endregion

#pragma region Additional functions
bool cam_free_initialize() {
	// Capture initial mouse position
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	// Disable cursor
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return true;
}
bool cam_free_update(float delta_time)
{// Function for interacting and updating the free camera
	// The ratio of pixels to rotation - remember the fov
	static const float sh = static_cast<float>(renderer::get_screen_height());
	static const float sw = static_cast<float>(renderer::get_screen_width());
	static const double ratio_width = quarter_pi<float>() / sw;
	static const double ratio_height = (quarter_pi<float>() * (sh / sw)) / sh;
	double current_x;
	double current_y;

	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);

	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = current_y - cursor_y;

	// Multiply deltas by ratios - gets actual change in orientation
	delta_x *= ratio_width;
	delta_y *= ratio_height;

	// Rotate cameras by delta
	cam_free.rotate(delta_x, -delta_y);

	// Camera translation vectors
	vec3 forward(.0f, .0f, .0f), backward(.0f, .0f, .0f), left(.0f, .0f, .0f), right(.0f, .0f, .0f), up(.0f, .0f, .0f), down(.0f, .0f, .0f);
	float speed(10.0f);

	// Use keyboard to move the camera - WSAD
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_W)) {
		forward += vec3(.0f, .0f, 1.0f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_S)) {
		backward += vec3(.0f, .0f, -1.0f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_A)) {
		left += vec3(-1.0f, .0f, .0f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_D)) {
		right += vec3(1.0f, .0f, .0f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_SPACE)) {
		up += vec3(.0f, 1.0f, .0f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT_CONTROL)) {
		down += vec3(.0f, -1.0f, .0f);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_LEFT_SHIFT)) {
		speed += 20.0f;
	}

	// Move camera
	vec3 camTranslation = forward + backward + left + right + up + down;
	if (camTranslation != vec3(.0f, .0f, .0f)) {
		cam_free.move(camTranslation * delta_time * speed);
	}

	// Update the camera
	cam_free.update(delta_time);

	// Update cursor pos
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);

	return true;
}
mat4 getMVP(mat4 M)
{// Returns MVP matrix with defined mesh, using the currently active camera
	mat4 V;
	mat4 P;
	switch (activeCamNo)
	{
	case 0:
		V = cam_free.get_view();
		P = cam_free.get_projection();
		break;
	case 1:
		V = cam_target_1.get_view();
		P = cam_target_1.get_projection();
		break;
	case 2:
		V = cam_target_2.get_view();
		P = cam_target_2.get_projection();
		break;
	}
	return P * V * M;
}
vec3 getEyePos()
{// Returns active camera's position
	switch (activeCamNo)
	{
	case 0: return cam_free.get_position();
		break;
	case 1: return cam_target_1.get_position();
		break;
	case 2: return cam_target_2.get_position();
		break;
	}
}
void movePlanets(float delta_time)
{
	meshes["planet1"].get_transform().rotate(vec3(0, quarter_pi<float>() * delta_time / 2, 0));
	seed += 0.03f; // this affects the radius (very sensitive)
	x = sinf(seed) / 10; // division affects the speed
	z = cosf(seed) / 10;

	meshes["planet2"].get_transform().translate(vec3(x, 0, z));
	meshes["planet2"].get_transform().rotate(vec3(0, 0, quarter_pi<float>() * delta_time));
	meshes["planet3"].get_transform().translate(vec3(0, x * 0.4f, 0)); //add sine wave to Y axes
	meshes["planet6"].get_transform().translate(vec3(-x * 0.6f, 0, z * 0.6f)); // reverse direction
	meshes["planet6"].get_transform().rotate(vec3(0, 0, half_pi<float>() * delta_time));
	meshes["planet7"].get_transform().translate(vec3(x * 0.2f, 0, z * 0.2f)); //add sine wave to all axes
	spots[1].set_position(meshes["planet2"].get_transform().position);
	spots[2].set_position(meshes["planet3"].get_transform().position + meshes["planet2"].get_transform().position + vec3(0.5f, 0, -1));
	spots[3].set_position(meshes["planet6"].get_transform().position);
}
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{// Menu Functionality
	float CAscale = 0.001f; // ChromaticAbberation scale
	float scale = 0.03f;	// Standard scale for other effects

	// Handle press and hold functionality as well
	// PRESS FUNCTIONALITY
	if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_M:
			if (renderMenu) {
				renderMenu = false;
				cout << "Menu Closed." << endl;
			}
			else
			{
				renderMenu = true;
				cout << "Menu Open." << endl;
			}
			break;

		case GLFW_KEY_R:
			PP.setDefaultValues();
			break;
		}

		if (renderMenu) {

			switch (key)
			{
				case GLFW_KEY_UP:
					if (PP.getActiveMenuItem() > 0) { PP.setActiveMenuItem(PP.getActiveMenuItem() - 1); } break; // go up on the menu list items
				case GLFW_KEY_DOWN:
					if (PP.getActiveMenuItem() < 5) { PP.setActiveMenuItem(PP.getActiveMenuItem() + 1); } break;// go down on the menu list items
				case GLFW_KEY_LEFT:
					switch (PP.getActiveMenuItem()) // DECREASE EFFECT
					{// depending on the value, set different constraints
						case 0: if (PP.getBrightness() > -1.0f) { PP.setBrightness(PP.getBrightness() - scale); } break; //brightness MIN value
						case 1: if (PP.getSaturation() > 0.0f) { PP.setSaturation(PP.getSaturation() - scale); } break; //saturation MIN value
						case 2: if (PP.getChromaticAbberation() > 0.0f) { PP.setChromaticAbberation(PP.getChromaticAbberation() - CAscale); } break; //chromatic abberation MIN value
						case 3: PP.setSepia(0.0f); break; // Sepia OFF
						case 4: if (PP.getFilmgrain() > 0.0f) { PP.setFilmgrain(PP.getFilmgrain() - scale * 5); } break; //filmgrain MIN value
					}
					cout << "Br: " + to_string(PP.getBrightness()) + " Sat: " + to_string(PP.getSaturation()) + " CA: " + to_string(PP.getChromaticAbberation()) + " Sep: " + to_string(PP.getSepia()) + " FG: " + to_string(PP.getFilmgrain()) << endl;
					break;
				case GLFW_KEY_RIGHT:
					switch (PP.getActiveMenuItem()) // INCREASE EFFECT
					{// depending on the value, set different constraints
						case 0: if (PP.getBrightness() < 1.0f) { PP.setBrightness(PP.getBrightness() + scale); } break; //brightness MAX value
						case 1: if (PP.getSaturation() < 3.0f) { PP.setSaturation(PP.getSaturation() + scale); } break; //saturation MAX value
						case 2: if (PP.getChromaticAbberation() < 0.004f) { PP.setChromaticAbberation(PP.getChromaticAbberation() + CAscale); } break; //chromatic abberation MAX value
						case 3: PP.setSepia(1.0f); break; // Sepia ON
						case 4: if (PP.getFilmgrain() < 8.0f) { PP.setFilmgrain(PP.getFilmgrain() + scale * 5); } break; //filmgrain MAX value
					}
					cout << "Br: " + to_string(PP.getBrightness()) + " Sat: " + to_string(PP.getSaturation()) + " CA: " + to_string(PP.getChromaticAbberation()) + " Sep: " + to_string(PP.getSepia()) + " FG: " + to_string(PP.getFilmgrain()) << endl;
					break;
			}
		}
	}

	// REPEATED KEYPRESS FUNCTIONALITY
	if (action == GLFW_REPEAT) {
		if (renderMenu) {
			switch (key)
			{
				case GLFW_KEY_LEFT:
					switch (PP.getActiveMenuItem()) // DECREASE EFFECT
					{// depending on the value, set different constraints
						case 0: if (PP.getBrightness() > -1.0f) { PP.setBrightness(PP.getBrightness() - scale); } break; //brightness MIN value
						case 1: if (PP.getSaturation() > 0.00f) { PP.setSaturation(PP.getSaturation() - scale * 2); } break; //saturation MIN value
						case 2: if (PP.getChromaticAbberation() > 0.0f) { PP.setChromaticAbberation(PP.getChromaticAbberation() - CAscale); } break; //chromatic abberation MIN value
						case 4: if (PP.getFilmgrain() > 0.0f) { PP.setFilmgrain(PP.getFilmgrain() - scale * 5); } break; //filmgrain MIN value
					}
					cout << "Br: " + to_string(PP.getBrightness()) + " Sat: " + to_string(PP.getSaturation()) + " CA: " + to_string(PP.getChromaticAbberation()) + " Sep: " + to_string(PP.getSepia()) + " FG: " + to_string(PP.getFilmgrain()) << endl;
					break;
				case GLFW_KEY_RIGHT:
					switch (PP.getActiveMenuItem()) // INCREASE EFFECT
					{// depending on the value, set different constraints
						case 0: if (PP.getBrightness() < 1.0f) { PP.setBrightness(PP.getBrightness() + scale); } break; //brightness MAX value
						case 1: if (PP.getSaturation() < 3.0f) { PP.setSaturation(PP.getSaturation() + scale); } break; //saturation MAX value
						case 2: if (PP.getChromaticAbberation() < 0.004f) { PP.setChromaticAbberation(PP.getChromaticAbberation() + CAscale); } break; //chromatic abberation MAX value
						case 4: if (PP.getFilmgrain() < 8.0f) { PP.setFilmgrain(PP.getFilmgrain() + scale * 5); } break; //filmgrain MAX value
					}
					cout << "Br: " + to_string(PP.getBrightness()) + " Sat: " + to_string(PP.getSaturation()) + " CA: " + to_string(PP.getChromaticAbberation()) + " Sep: " + to_string(PP.getSepia()) + " FG: " + to_string(PP.getFilmgrain()) << endl;
					break;
			}
		}
	}
}
//RENDERS
bool renderSkybox()
{// Renders skybox

 // Disable depth test, depth mask, face culling
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	// Bind skybox effect
	renderer::bind(effects["sky_eff"]);

	// Calculate MVP for the skybox
	auto MVP = getMVP(meshes["skybox"].get_transform().get_transform_matrix());
	// Set MVP matrix uniform
	glUniformMatrix4fv(effects["sky_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Set cubemap uniform
	renderer::bind(cube_map, 0);
	glUniform1i(effects["sky_eff"].get_uniform_location("cubemap"), 0);

	// Render skybox
	renderer::render(meshes["skybox"]);

	// Re-Enable depth test, depth mask, face culling
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);

	return true;
}
bool renderShadowMap(mat4 LightProjectionMat)
{
	// Set render target to shadow map
	renderer::set_render_target(shadow);
	// Clear depth buffer bit
	glClear(GL_DEPTH_BUFFER_BIT);
	// Set face cull mode to front
	glCullFace(GL_FRONT);

	// Bind shader
	renderer::bind(effects["shadow_eff"]);

	// Render meshes
	for (auto &e : meshes) {
		auto mesh = e.second;
		auto meshName = e.first;

		if (meshName == "gate") { // only render the gate

			// Create MVP matrix
			auto M = mesh.get_transform().get_transform_matrix();

			// View matrix taken from shadow map
			auto V = shadow.get_view();

			auto MVP = LightProjectionMat * V * M;
			// Set MVP matrix uniform
			glUniformMatrix4fv(effects["shadow_eff"].get_uniform_location("MVP"), // Location of uniform
				1,                                      // Number of values - 1 mat4
				GL_FALSE,                               // Transpose the matrix?
				value_ptr(MVP));                        // Pointer to matrix data

														// Render mesh
			renderer::render(mesh);
		}
	}

	// Set render target back to the screen
	renderer::set_render_target(scene);
	// Set face cull mode to back
	glCullFace(GL_BACK);

	return true;
}
bool renderStandard(string meshName, mesh m, mat4 MVP, mat4 lightMVP)
{
	// Bind effect
	renderer::bind(effects["main_eff"]);
	// Set MVP / M / N matrix uniform
	glUniformMatrix4fv(effects["main_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniformMatrix4fv(effects["main_eff"].get_uniform_location("M"), 1, GL_FALSE, value_ptr(m.get_transform().get_transform_matrix()));
	glUniformMatrix3fv(effects["main_eff"].get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));

	// Bind material / directional, spot, point lights / texture / shadowmap
	renderer::bind(m.get_material(), "mat");
	renderer::bind(light, "light");
	renderer::bind(points, "points");
	renderer::bind(spots, "spots");
	renderer::bind(shadow.buffer->get_depth(), 2);
	// If 'textures' does not have a key with the same name as the mesh, then the mesh doesn't have a texture defined, bind default
	if (textures.count(meshName) == 0)
	{// Bind default
		renderer::bind(textures["unTextured"], 0);
	}
	else
	{// Bind corresponding texture
		renderer::bind(textures[meshName], 0);
	}

	// Set the texture value / eye position / lightMVP for shadow / shadowmap
	glUniform1i(effects["main_eff"].get_uniform_location("tex"), 0);
	glUniform3fv(effects["main_eff"].get_uniform_location("eye_pos"), 1, value_ptr(getEyePos()));
	glUniformMatrix4fv(effects["main_eff"].get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(lightMVP));
	glUniform1i(effects["main_eff"].get_uniform_location("shadowMap"), 2);

	return true;
}
bool renderNormalMap(string meshName, mesh m, mat4 MVP, mat4 lightMVP)
{
	// Bind effect
	renderer::bind(effects["normal_eff"]);
	// Set MVP / M / N matrix uniform
	glUniformMatrix4fv(effects["normal_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniformMatrix4fv(effects["normal_eff"].get_uniform_location("M"), 1, GL_FALSE, value_ptr(m.get_transform().get_transform_matrix()));
	glUniformMatrix3fv(effects["normal_eff"].get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));

	// Bind material / directional, spot, point lights / texture / shadowmap
	renderer::bind(m.get_material(), "mat");
	renderer::bind(light, "light");
	renderer::bind(points, "points");
	renderer::bind(spots, "spots");
	renderer::bind(normal_maps[meshName], 1);
	renderer::bind(shadow.buffer->get_depth(), 2);
	// If 'textures' does not have a key with the same name as the mesh, then the mesh doesn't have a texture defined, bind default
	if (textures.count(meshName) == 0)
	{// Bind default
		renderer::bind(textures["unTextured"], 0);
	}
	else
	{// Bind corresponding texture
		renderer::bind(textures[meshName], 0);
	}

	// Set the texture value / eye position / lightMVP for shadow / shadowmap
	glUniform1i(effects["normal_eff"].get_uniform_location("tex"), 0);
	glUniform3fv(effects["normal_eff"].get_uniform_location("eye_pos"), 1, value_ptr(getEyePos()));
	glUniformMatrix4fv(effects["normal_eff"].get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(lightMVP));
	glUniform1i(effects["normal_eff"].get_uniform_location("normalMap"), 1);
	glUniform1i(effects["normal_eff"].get_uniform_location("shadowMap"), 2);

	return true;
}
bool renderReflection(string meshName, mesh m, mat4 MVP, mat4 lightMVP)
{
	// Bind effect
	renderer::bind(effects["reflection_eff"]);
	// Set MVP / M / N matrix uniform
	glUniformMatrix4fv(effects["reflection_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniformMatrix4fv(effects["reflection_eff"].get_uniform_location("M"), 1, GL_FALSE, value_ptr(m.get_transform().get_transform_matrix()));
	glUniformMatrix3fv(effects["reflection_eff"].get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));

	// Bind material / directional, spot, point lights / texture / shadowmap
	renderer::bind(m.get_material(), "mat");
	renderer::bind(light, "light");
	renderer::bind(points, "points");
	renderer::bind(spots, "spots");
	renderer::bind(shadow.buffer->get_depth(), 2);
	// If 'textures' does not have a key with the same name as the mesh, then the mesh doesn't have a texture defined, bind default
	if (textures.count(meshName) == 0)
	{// Bind default
		renderer::bind(textures["unTextured"], 0);
	}
	else
	{// Bind corresponding texture
		renderer::bind(textures[meshName], 0);
	}

	// Set the texture value / eye position / lightMVP for shadow / shadowmap
	glUniform1i(effects["reflection_eff"].get_uniform_location("tex"), 0);
	glUniform3fv(effects["reflection_eff"].get_uniform_location("eye_pos"), 1, value_ptr(getEyePos()));
	glUniformMatrix4fv(effects["reflection_eff"].get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(lightMVP));
	glUniform1i(effects["reflection_eff"].get_uniform_location("shadowMap"), 2);
	// Set reflection / refraction values for corresponding mesh
	glUniform1f(effects["reflection_eff"].get_uniform_location("reflectionAmount"), RenderTypes[meshName].getReflectionAmount());
	glUniform1f(effects["reflection_eff"].get_uniform_location("refractionAmount"), RenderTypes[meshName].getRefractionAmount());

	// Set different cubemap for refraction effect
	if (meshName == "lightBall")
	{
		renderer::bind(cube_map2, 3);
		glUniform1i(effects["reflection_eff"].get_uniform_location("cubeMap"), 3);
	}

	return true;
}
bool renderFresnel(string meshName, mesh m, mat4 MVP, mat4 lightMVP)
{
	// Bind effect
	renderer::bind(effects["fresnel_eff"]);
	// Set MVP / M / N matrix uniform
	glUniformMatrix4fv(effects["fresnel_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniformMatrix4fv(effects["fresnel_eff"].get_uniform_location("M"), 1, GL_FALSE, value_ptr(m.get_transform().get_transform_matrix()));
	glUniformMatrix3fv(effects["fresnel_eff"].get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));

	// Bind material / directional, spot, point lights / texture / shadowmap
	renderer::bind(m.get_material(), "mat");
	renderer::bind(light, "light");
	renderer::bind(points, "points");
	renderer::bind(spots, "spots");
	renderer::bind(shadow.buffer->get_depth(), 2);
	renderer::bind(cube_map2, 3);
	// If 'textures' does not have a key with the same name as the mesh, then the mesh doesn't have a texture defined, bind default
	if (textures.count(meshName) == 0)
	{// Bind default
		renderer::bind(textures["unTextured"], 0);
	}
	else
	{// Bind corresponding texture
		renderer::bind(textures[meshName], 0);
	}

	// Set the texture value / eye position / lightMVP for shadow / shadowmap
	glUniform1i(effects["fresnel_eff"].get_uniform_location("tex"), 0);
	glUniform3fv(effects["fresnel_eff"].get_uniform_location("eye_pos"), 1, value_ptr(getEyePos()));
	glUniformMatrix4fv(effects["fresnel_eff"].get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(lightMVP));
	glUniform1i(effects["fresnel_eff"].get_uniform_location("shadowMap"), 2);
	glUniform1i(effects["reflection_eff"].get_uniform_location("cubeMap"), 3);
	// Set fresnel intensity for corresponding mesh
	glUniform1f(effects["reflection_eff"].get_uniform_location("fresnelIntensity"), RenderTypes[meshName].getFresnelIntensity()); // currently not being used

	return true;
}
bool renderNormalMapAndReflection(string meshName, mesh m, mat4 MVP, mat4 lightMVP)
{
	// Bind effect
	renderer::bind(effects["normal-reflection_eff"]);
	// Set MVP / M / N matrix uniform
	glUniformMatrix4fv(effects["normal-reflection_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	glUniformMatrix4fv(effects["normal-reflection_eff"].get_uniform_location("M"), 1, GL_FALSE, value_ptr(m.get_transform().get_transform_matrix()));
	glUniformMatrix3fv(effects["normal-reflection_eff"].get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));

	// Bind material / directional, spot, point lights / texture / shadowmap
	renderer::bind(m.get_material(), "mat");
	renderer::bind(light, "light");
	renderer::bind(points, "points");
	renderer::bind(spots, "spots");
	renderer::bind(normal_maps[meshName], 1);
	renderer::bind(shadow.buffer->get_depth(), 2);
	renderer::bind(cube_map2, 3);
	// If 'textures' does not have a key with the same name as the mesh, then the mesh doesn't have a texture defined, bind default
	if (textures.count(meshName) == 0)
	{// Bind default
		renderer::bind(textures["unTextured"], 0);
	}
	else
	{// Bind corresponding texture
		renderer::bind(textures[meshName], 0);
	}

	// Set the texture value / eye position / lightMVP for shadow / shadowmap
	glUniform1i(effects["normal-reflection_eff"].get_uniform_location("tex"), 0);
	glUniform3fv(effects["normal-reflection_eff"].get_uniform_location("eye_pos"), 1, value_ptr(getEyePos()));
	glUniformMatrix4fv(effects["normal-reflection_eff"].get_uniform_location("lightMVP"), 1, GL_FALSE, value_ptr(lightMVP));
	glUniform1i(effects["normal-reflection_eff"].get_uniform_location("normalMap"), 1);
	glUniform1i(effects["normal-reflection_eff"].get_uniform_location("shadowMap"), 2);
	glUniform1i(effects["normal-reflection_eff"].get_uniform_location("cubeMap"), 3);
	// Set reflection / refraction values for corresponding mesh
	glUniform1f(effects["normal-reflection_eff"].get_uniform_location("reflectionAmount"), RenderTypes[meshName].getReflectionAmount());
	glUniform1f(effects["normal-reflection_eff"].get_uniform_location("refractionAmount"), RenderTypes[meshName].getRefractionAmount());

	// Set different cubemap for refraction effect
	if (meshName == "statue")
	{
		renderer::bind(cube_map2, 3);
		glUniform1i(effects["normal-reflection_eff"].get_uniform_location("cubeMap"), 3);
	}

	return true;
}
#pragma endregion

#pragma region Main functions
bool load_content() {
// Frame buffer to render to
	scene = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
// MESHES
	// Sphere to test location of lights
	//meshes["locationTest"] = mesh(geometry_builder::create_sphere(10, 10));
	//meshes["locationTest"].get_transform().scale = vec3(0.2f);
	// Create shadow map
	shadow = shadow_map(renderer::get_screen_width(), renderer::get_screen_height());
	// Create meshes
	meshes["skybox"] = mesh(geometry_builder::create_box());
	meshes["grass"] = mesh(geometry_builder::create_plane());
	meshes["statue"] = mesh(geometry("res/models/statue.obj"));
	meshes["gate"] = mesh(geometry("res/models/gate.obj"));
	meshes["gate2"] = meshes["gate"];
	meshes["water"] = mesh(geometry_builder::create_disk(10));
	meshes["pool"] = mesh(geometry_builder::create_torus(45, 6, 0.5f, 6.0f));
	meshes["hedge"] = mesh(geometry_builder::create_box());
	meshes["hedge2"] = mesh(geometry_builder::create_box());
	meshes["hedge3"] = mesh(geometry_builder::create_box());
	meshes["hedge4"] = mesh(geometry_builder::create_box());
	meshes["lampp"] = mesh(geometry("res/models/lamp_post.obj"));
	meshes["lampp2"] = meshes["lampp"];
	meshes["bench"] = mesh(geometry("res/models/bench.obj"));
	meshes["planet1"] = mesh(geometry_builder::create_sphere(25, 25));
	meshes["planet2"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["planet3"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["planet4"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["planet5"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["planet6"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["planet7"] = mesh(geometry_builder::create_sphere(20, 20));
	meshes["lightBall"] = meshes["lightBall"] = mesh(geometry_builder::create_sphere(10, 10));
	// Transform objects
	meshes["skybox"].get_transform().scale = vec3(500);
	meshes["grass"].get_transform().scale = vec3(0.6f);
	meshes["statue"].get_transform().scale = vec3(0.5f);
	meshes["statue"].get_transform().translate(vec3(0.0f, 3.0f, 0.0f));
	meshes["gate"].get_transform().scale = vec3(0.02f);
	meshes["gate"].get_transform().translate(vec3(-12, 2, -5));
	meshes["gate2"].get_transform().scale = vec3(0.02f);
	meshes["gate2"].get_transform().translate(vec3(12, 2, 17));
	meshes["water"].get_transform().scale = vec3(12, 1, 12);
	meshes["water"].get_transform().translate(vec3(0, 0.5f, 7));
	meshes["pool"].get_transform().translate(vec3(0, 0.2f, 7));
	meshes["hedge"].get_transform().scale = vec3(25, 5, 1);
	meshes["hedge"].get_transform().translate(vec3(0, 2, -8));
	meshes["hedge2"].get_transform().scale = vec3(22, 5, 1);
	meshes["hedge2"].get_transform().translate(vec3(-12, 2, 8.5f));
	meshes["hedge2"].get_transform().rotate(vec3(0, half_pi<float>(), 0));
	meshes["hedge2"].get_transform().rotate(vec3(0, half_pi<float>(), 0));
	meshes["hedge3"].get_transform().scale = vec3(22, 5, 1);
	meshes["hedge3"].get_transform().translate(vec3(12, 2, 3.5));
	meshes["hedge3"].get_transform().rotate(vec3(0, half_pi<float>(), 0));
	meshes["hedge3"].get_transform().rotate(vec3(0, half_pi<float>(), 0));
	meshes["hedge4"].get_transform().scale = vec3(25, 5, 1);
	meshes["hedge4"].get_transform().translate(vec3(0, 2, 20));
	meshes["lampp"].get_transform().scale = vec3(0.35);
	meshes["lampp"].get_transform().translate(vec3(-11, 0, -2));
	meshes["lampp2"].get_transform().scale = vec3(0.35);
	meshes["lampp2"].get_transform().translate(vec3(3, 0, 18));
	meshes["lampp2"].get_transform().rotate(vec3(0, half_pi<float>(), 0));
	meshes["lampp2"].get_transform().rotate(vec3(0, half_pi<float>(), 0));
	meshes["bench"].get_transform().scale = vec3(0.02);
	meshes["bench"].get_transform().translate(vec3(0, 0, 18));
	meshes["bench"].get_transform().rotate(vec3(0, pi<float>(), 0));
	meshes["bench"].get_transform().rotate(vec3(0, pi<float>(), 0));
	meshes["planet1"].get_transform().scale = vec3(0.6f);
	meshes["planet1"].get_transform().translate(vec3(0, 1.5f, 7));
	meshes["planet2"].get_transform().scale = vec3(0.3f);
	meshes["planet2"].get_transform().translate(vec3(-3.5f, 0.9f, 7.5f));
	meshes["planet3"].get_transform().scale = vec3(0.85f);
	meshes["planet3"].get_transform().translate(vec3(1.5f, 0, 0));
	meshes["planet4"].get_transform().scale = vec3(0.25f);
	meshes["planet4"].get_transform().translate(vec3(0.8f, 2.55f, 0.6f));
	meshes["planet5"].get_transform().scale = vec3(0.3f);
	meshes["planet5"].get_transform().translate(vec3(0.3f, 2.65f, 0.75f));
	meshes["planet6"].get_transform().scale = vec3(0.15f);
	meshes["planet6"].get_transform().translate(vec3(2.5f, 1.2f, 7.5f));
	meshes["planet7"].get_transform().scale = vec3(0.9f);
	meshes["planet7"].get_transform().translate(vec3(0, 2, 0));
	meshes["lightBall"].get_transform().scale = vec3(0.1f);

// GEOMETRY
	// Geometry for the post processing effects to render to
	screen.set_type(GL_TRIANGLE_STRIP);
	vector<vec3> screen_positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> screen_tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };

	screen.add_buffer(screen_positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen.add_buffer(screen_tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);

	// Geometry for the menu
	menu_geom.set_type(GL_TRIANGLE_STRIP);
	vector<vec3> menu_positions{ vec3(-1.0f, -1.0f, -1.0f), vec3(-0.8f, -1.0f, -1.0f), vec3(-1.0f, -0.3f, -1.0f), vec3(-0.8f, -0.3f, -1.0f) }; //-1 on Z so its in "front" of post process quad
	vector<vec2> menu_tex_coords{ vec2(0.0, 0.0), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };

	menu_geom.add_buffer(menu_positions, BUFFER_INDEXES::POSITION_BUFFER);
	menu_geom.add_buffer(menu_tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);

// LIGHTING
	// Set materials
	material mat;
	mat.set_emissive(vec4(0, 0, 0, 1));

	// Pool
	mat.set_diffuse(vec4(0.6f, 0.6f, 0.6f, 1));
	mat.set_specular(vec4(1));
	mat.set_shininess(10.0f);
	meshes["pool"].set_material(mat);
	// Water
	mat.set_diffuse(vec4(0.9f, 0.9f, 0.9f, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(1);
	meshes["water"].set_material(mat);
	// Grass
	mat.set_diffuse(vec4(0.3f, 0.5f, 0.3f, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(5.0f);
	meshes["grass"].set_material(mat);
	// Lamp Post
	mat.set_diffuse(vec4(0.5f, 0.5f, 0.5f, 1));
	mat.set_specular(vec4(0.8f));
	mat.set_shininess(5.0f);
	meshes["lampp"].set_material(mat);
	meshes["lampp2"].set_material(mat);
	// Bench
	mat.set_diffuse(vec4(1, 1, 1, 1));
	mat.set_specular(vec4(0.1f));
	mat.set_shininess(5.0f);
	meshes["bench"].set_material(mat);
	// Gate
	mat.set_diffuse(vec4(1, 1, 1, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(5);
	meshes["gate"].set_material(mat);
	meshes["gate2"].set_material(mat);
	// Hedge
	mat.set_diffuse(vec4(0.4f, 0.5f, 0.4f, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(5.0f);
	meshes["hedge"].set_material(mat);
	meshes["hedge2"].set_material(mat);
	meshes["hedge3"].set_material(mat);
	meshes["hedge4"].set_material(mat);
	// Planet1
	mat.set_emissive(vec4(0.1f, 0.1f, 0.2f, 1));
	mat.set_diffuse(vec4(1, 1, 1, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(5.0f);
	meshes["planet1"].set_material(mat);
	// Planet2
	mat.set_diffuse(vec4(.5f, .1f, .5f, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(5.0f);
	meshes["planet2"].set_material(mat);
	// Planet3
	mat.set_emissive(vec4(0.15f, 0.05f, 0.05f, 1));
	mat.set_diffuse(vec4(1, 1, 1, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(25.0f);
	meshes["planet3"].set_material(mat);
	// Planet4
	mat.set_emissive(vec4(0.05f, 0.1f, 0.05f, 1));
	mat.set_diffuse(vec4(1, 0.1f, 0, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(5.0f);
	meshes["planet4"].set_material(mat);
	// Planet5
	mat.set_emissive(vec4(.09f, .09f, .09f, 1));
	mat.set_diffuse(vec4(.5f, .5f, .5f, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(5.0f);
	meshes["planet5"].set_material(mat);
	// Planet6
	mat.set_diffuse(vec4(0, 0, 0, 1));
	mat.set_specular(vec4(1));
	mat.set_shininess(50.0f);
	meshes["planet6"].set_material(mat);
	// Planet7
	mat.set_diffuse(vec4(.1f, .5f, .1f, 1));
	mat.set_specular(vec4(1));
	mat.set_shininess(25.0f);
	meshes["planet7"].set_material(mat);
	// Statue
	mat.set_diffuse(vec4(0.1f, 0.1f, 0.1f, 1));
	mat.set_specular(vec4(0));
	mat.set_shininess(5);
	meshes["statue"].set_material(mat);
	
	// Set lighting values
	// Point 0 - lamp post 1
	points[0].set_position(meshes["lampp"].get_transform().position + vec3(2.2f, 6, 0));
	points[0].set_light_colour(vec4(1, 0.8f, 0, 1.0f));
	points[0].set_range(10.0f);
	// Point 1 - lamp post 1
	points[1].set_position(meshes["lampp"].get_transform().position + vec3(1.4f, 6, 0.4f));
	points[1].set_light_colour(vec4(1, 0.8f, 0, 1.0f));
	points[1].set_range(10.0f);
	// Point 2 - lamp post 1
	points[2].set_position(meshes["lampp"].get_transform().position + vec3(1.4f, 6, -0.4f));
	points[2].set_light_colour(vec4(1, 0.8f, 0, 1.0f));
	points[2].set_range(10.0f);
	// Point 3 - lamp post 2
	points[3].set_position(meshes["lampp2"].get_transform().position + vec3(0, 6, -1.2f));
	points[3].set_light_colour(vec4(1, 0.8f, 0, 1.0f));
	points[3].set_range(10.0f);
	// Point 4 - lamp post 2
	points[4].set_position(meshes["lampp2"].get_transform().position + vec3(-0.5f, 6, -2));
	points[4].set_light_colour(vec4(1, 0.8f, 0, 1.0f));
	points[4].set_range(10.0f);
	// Point 5 - lamp post 2
	points[5].set_position(meshes["lampp2"].get_transform().position + vec3(0.5f, 6, -2));
	points[5].set_light_colour(vec4(1, 0.8f, 0, 1.0f));
	points[5].set_range(10.0f);

	// Spot 0 - under planet 1
	spots[0].set_position(vec3(meshes["planet1"].get_transform().position - vec3(0, 0, 0)));
	spots[0].set_light_colour(vec4(0.2f, 0.2f, 1, 1));
	spots[0].set_direction(normalize(vec3(0, -1, 0)));
	spots[0].set_range(10.0f);
	spots[0].set_power(20);
	// Spot 1 - under planet 2
	spots[1].set_light_colour(vec4(0.6f, 0, 0.8f, 1));
	spots[1].set_direction(normalize(vec3(0, -1, 0)));
	spots[1].set_range(5.0f);
	spots[1].set_power(20);
	// Spot 2 - under planet 3
	spots[2].set_position(vec3(0, -10, 0));
	spots[2].set_light_colour(vec4(0.7f, 0, 0, 1));
	spots[2].set_direction(normalize(vec3(0, -1, 0)));
	spots[2].set_range(7);
	spots[2].set_power(20);
	// Spot 3 - under planet 6
	spots[3].set_light_colour(vec4(0.4f, 0.4f, 0.4f, 1));
	spots[3].set_direction(normalize(vec3(0, -1, 0)));
	spots[3].set_range(5.0f);
	spots[3].set_power(20);
	// Spot 5 - outside the gate
	spots[4].set_position(meshes["gate"].get_transform().position + vec3(-4, 6, 0));
	spots[4].set_light_colour(vec4(0.0f, 0.6f, 1.0f, 1.0f));
	spots[4].set_direction(normalize(vec3(1.01f, -1.01f, 0.0f)));
	spots[4].set_range(10.0f);
	spots[4].set_power(10.0f);

	// Directional
	light.set_ambient_intensity(vec4(0.1f, 0.1f, 0.1f, 1));
	light.set_light_colour(vec4(0.1f, 0.1f, 0.2f, 1));
	light.set_direction(vec3(1.0f, -1.0f, 1.0f));
	
	// Sphere to test location of lights
	//meshes["locationTest"].get_transform().position = spots[4].get_position() + vec3(-1, 0, 0);

// SHADERS
	// Load in main shaders 
	effects["main_eff"].add_shader("res/shaders/main_shader.vert", GL_VERTEX_SHADER);
	vector<string> fragment_shaders{"res/shaders/main_shader.frag", "res/shaders/part_spot.frag", "res/shaders/part_point.frag", "res/shaders/part_shadow.frag", "res/shaders/part_direction.frag" };
	effects["main_eff"].add_shader(fragment_shaders, GL_FRAGMENT_SHADER);
	effects["main_eff"].build();
	// Load in normal shaders
	effects["normal_eff"].add_shader("res/shaders/normal_shader.vert", GL_VERTEX_SHADER);
	fragment_shaders = { "res/shaders/normal_shader.frag", "res/shaders/part_spot.frag", "res/shaders/part_point.frag", "res/shaders/part_shadow.frag", "res/shaders/part_direction.frag", "res/shaders/part_normal.frag" };
	effects["normal_eff"].add_shader(fragment_shaders, GL_FRAGMENT_SHADER);
	effects["normal_eff"].build();
	// Load in fresnel shaders 
	effects["fresnel_eff"].add_shader("res/shaders/fresnel_shader.vert", GL_VERTEX_SHADER);
	fragment_shaders = { "res/shaders/fresnel_shader.frag", "res/shaders/part_spot.frag", "res/shaders/part_point.frag", "res/shaders/part_shadow.frag", "res/shaders/part_direction.frag" };
	effects["fresnel_eff"].add_shader(fragment_shaders, GL_FRAGMENT_SHADER);
	effects["fresnel_eff"].build();
	// Load in reflection shaders 
	effects["reflection_eff"].add_shader("res/shaders/reflection_shader.vert", GL_VERTEX_SHADER);
	fragment_shaders = { "res/shaders/reflection_shader.frag", "res/shaders/part_spot.frag", "res/shaders/part_point.frag", "res/shaders/part_shadow.frag", "res/shaders/part_direction.frag" };
	effects["reflection_eff"].add_shader(fragment_shaders, GL_FRAGMENT_SHADER);
	effects["reflection_eff"].build();
	// Load in normal-reflection shaders
	effects["normal-reflection_eff"].add_shader("res/shaders/normal-reflection_shader.vert", GL_VERTEX_SHADER);
	fragment_shaders = { "res/shaders/normal-reflection_shader.frag", "res/shaders/part_spot.frag", "res/shaders/part_point.frag", "res/shaders/part_shadow.frag", "res/shaders/part_direction.frag", "res/shaders/part_normal.frag" };
	effects["normal-reflection_eff"].add_shader(fragment_shaders, GL_FRAGMENT_SHADER);
	effects["normal-reflection_eff"].build();
	// Load in skybox shaders
	effects["sky_eff"].add_shader("res/shaders/skybox.vert", GL_VERTEX_SHADER);
	effects["sky_eff"].add_shader("res/shaders/skybox.frag", GL_FRAGMENT_SHADER);
	effects["sky_eff"].build();
	// Load in shadow shaders
	effects["shadow_eff"].add_shader("res/shaders/shadow.vert", GL_VERTEX_SHADER);
	effects["shadow_eff"].add_shader("res/shaders/shadow.frag", GL_FRAGMENT_SHADER);
	effects["shadow_eff"].build();
	// Load in post process shaders
	effects["post_process_eff"].add_shader("res/shaders/post_process.vert", GL_VERTEX_SHADER);
	effects["post_process_eff"].add_shader("res/shaders/post_process.frag", GL_FRAGMENT_SHADER);
	effects["post_process_eff"].build();
	// Load in menu shaders
	effects["menu_eff"].add_shader("res/shaders/menu.vert", GL_VERTEX_SHADER);
	effects["menu_eff"].add_shader("res/shaders/menu.frag", GL_FRAGMENT_SHADER);
	effects["menu_eff"].build();

// TEXTURES
	// Load in textures
	array<string, 6> skybox_tex = { "res/textures/skybox/miramar_ft.png", "res/textures/skybox/miramar_bk.png", "res/textures/skybox/miramar_up.png", "res/textures/skybox/miramar_dn.png", "res/textures/skybox/miramar_rt.png", "res/textures/skybox/miramar_lf.png" };
	array<string, 6> skybox2_tex = { "res/textures/skybox/miramar_ft.png", "res/textures/skybox/miramar_bk.png", "res/textures/skybox/miramar_up.png", "res/textures/skybox/miramar_rt.png", "res/textures/skybox/miramar_rt.png", "res/textures/skybox/miramar_lf.png" };
	normal_maps["gate"] = texture("res/textures/gate_normal.png"); normal_maps["gate2"] = normal_maps["gate"];
	normal_maps["statue"] = texture("res/textures/statue_normal.jpg");
	textures["statue"] = texture("res/textures/statue_tex.jpg");
	textures["unTextured"] = texture("res/textures/checked.gif");
	textures["grass"] = texture("res/textures/grassHD.jpg");
	textures["water"] = texture("res/textures/water.jpg");
	textures["pool"] = texture("res/textures/marble.jpg");
	textures["gate"] = texture("res/textures/gate.png"); textures["gate2"] = textures["gate"];
	textures["hedge"] = texture("res/textures/hedge.png"); textures["hedge2"] = textures["hedge"]; textures["hedge3"] = textures["hedge"]; textures["hedge4"] = textures["hedge"];
	textures["lampp"] = texture("res/textures/metal.jpg"); textures["lampp2"] = textures["lampp"];
	textures["bench"] = texture("res/textures/wood.jpg");
	textures["planet1"] = texture("res/textures/planet1.jpg"); textures["planet2"] = textures["planet1"]; textures["planet3"] = textures["planet1"]; textures["planet4"] = textures["planet1"]; textures["planet7"] = textures["planet1"];
	textures["planet5"] = texture("res/textures/planet2.jpg"); textures["planet6"] = textures["planet5"];
	// Add textures to cubemap
	cube_map = cubemap(skybox_tex);
	cube_map2 = cubemap(skybox2_tex); // for statue reflection
	// Load in menu textures
	menu_tex[0] = texture("res/textures/menu_brightness.png");	// Brightness
	menu_tex[1] = texture("res/textures/menu_saturation.png");	// Saturation
	menu_tex[2] = texture("res/textures/menu_chromaticAbberation.png");	//ChromaticAbberation
	menu_tex[3] = texture("res/textures/menu_sepia.png");		// Sepia
	menu_tex[4] = texture("res/textures/menu_filmGrain.png");	// Filmgrain
// CAMERAS
	// Set camera properties
	cam_free.set_position(vec3(0.0f, 5.0f, 10.0f));
	cam_free.set_target(vec3(0.0f, 0.0f, 0.0f));
	cam_free.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	cam_target_1.set_position(vec3(0.0f, 3.0f, 20.0f));
	cam_target_1.set_target(vec3(0.0f, 0.0f, 2.0f));
	cam_target_1.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	cam_target_2.set_target(meshes["planet1"].get_transform().position);
	cam_target_2.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);

// RENDER TYPES AND EFFECTS - assign render effects to meshes
	// TYPE -	1 - normal map / 2 - reflection / 3 - fresnel / 4 - reflection and normal map / novalue - texture only
	// RenderType(type, reflectionAmount, refractionAmount, fresnelIntensity)
	RenderTypes["statue"] = RenderType(4, 0, 1.0f, 0);
	RenderTypes["statue"] = RenderType(4, 0, 1.0f, 0);
	RenderTypes["lightBall"] = RenderType(2, 0, 1.0f, 0);
	RenderTypes["water"] = RenderType(3, 0, 0.0f, 0); // Fresnel intensity is 0, the effect is modified for aesthetic reasons, edit in shader for the classic effect
	RenderTypes["gate"] = RenderType(4, 0.2f, 0, 0);
	RenderTypes["gate2"] = RenderTypes["gate"];
	RenderTypes["lampp"] = RenderType(2, 0.3f, 0.0f, 0);
	RenderTypes["lampp2"] = RenderTypes["lampp"];
	RenderTypes["planet1"] = RenderType(2, 0.4f, 0.2f, 0);
	RenderTypes["planet2"] = RenderTypes["planet1"];
	RenderTypes["planet3"] = RenderTypes["planet1"];
	RenderTypes["planet4"] = RenderTypes["planet1"];
	RenderTypes["planet5"] = RenderTypes["planet1"];
	RenderTypes["planet6"] = RenderTypes["planet1"];
	RenderTypes["planet7"] = RenderTypes["planet1"];

	return true;
}
bool update(float delta_time) {
	// Camera change
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_1)){
		activeCamNo = 0;
		cam_free_initialize();
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_2)){
		activeCamNo = 1;
		// Re-Enable cursor
		glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_3)) {
		activeCamNo = 2;
		// Re-Enable cursor
		glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	// SHADOW
	// Move spotlight outside the gates
		//The shadow will clip over the hedge because thats not being rendered on the shadow map
		//If the hedges were rendered they would obscure too much of the scene, point of this is to have a basic dynamic shadow going.
	y = sinf(seed) / 8; //using the radius variable from planet movement
	spots[4].move(vec3(0, 0, y));

	shadow.light_position = spots[4].get_position();
	shadow.light_dir = spots[4].get_direction();

	meshes["lightBall"].get_transform().position = spots[4].get_position() - vec3(0.5f, 0.0f, 0.0f);

	if (glfwGetKey(renderer::get_window(), 'I') == GLFW_PRESS)
		shadow.buffer->save("test.png");


	// Update active camera
	switch (activeCamNo)
	{
	case 0: cam_free_update(delta_time);
			// Keep camera in the centre of skybox
			meshes["skybox"].get_transform().position = cam_free.get_position();
		break;
	case 1: cam_target_1.update(delta_time);
			// Keep camera in the centre of skybox
			meshes["skybox"].get_transform().position = cam_target_1.get_position();
		break;
	case 2: cam_target_2.update(delta_time);
			// Keep camera in the centre of skybox
			meshes["skybox"].get_transform().position = cam_target_2.get_position();
			cam_target_2.set_position(meshes["planet2"].get_transform().position);
		break;
	}

	// Move planets around
	movePlanets(delta_time);

	// Add time for noise
	frameTime += delta_time;

	return true;
}
bool render() {
	// Set render target
	renderer::set_render_target(scene);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

// SKYBOX
	renderSkybox();

// SHADOW
	// Create light proj mat with fov 90
	mat4 LightProjectionMat = perspective<float>(90.0f, renderer::get_screen_aspect(), 0.1f, 1000.0f);
	renderShadowMap(LightProjectionMat);

// MESHES LOOP
	for (auto &e : meshes)
	{
		// Set Variables for each mesh
		auto m = e.second; // mesh object
		auto meshName = e.first;
		auto MVP = mat4(0);
		auto shadow_viewMatrix = shadow.get_view();
		auto lightMVP = LightProjectionMat * shadow_viewMatrix * m.get_transform().get_transform_matrix();

		// Apply Transformation Inheritance
			// Each planet has their own transformation but 3 and 7 inherit from other ones
		if (e.first == "planet3")
		{// If planet3 comes, apply planet2's transform
			MVP = getMVP(m.get_transform().get_transform_matrix() * meshes["planet2"].get_transform().get_transform_matrix());
		}
		else if (e.first == "planet7")
		{// If planet7 comes, apply planet2's and planet3's transform
			MVP = getMVP(m.get_transform().get_transform_matrix() * meshes["planet2"].get_transform().get_transform_matrix() * meshes["planet3"].get_transform().get_transform_matrix());
		}
		else {// Create MVP matrix
			MVP = getMVP(m.get_transform().get_transform_matrix());
		}

	// SWITCH ON RENDER TYPE
		switch (RenderTypes[meshName].getRenderType())
		{
			case 1: renderNormalMap(meshName, m, MVP, lightMVP);
				break;
			case 2: renderReflection(meshName, m, MVP, lightMVP);
				break;
			case 3: renderFresnel(meshName, m, MVP, lightMVP);
				break;
			case 4: renderNormalMapAndReflection(meshName, m, MVP, lightMVP);
				break;
			default: renderStandard(meshName, m, MVP, lightMVP);
				break;
		}

		// Render geometry
		renderer::render(m);
	}

// POST PROCESSING
	// Set render target
	renderer::set_render_target();
	// Bind effect
	renderer::bind(effects["post_process_eff"]);

	auto MVP = mat4(1.0f);
	glUniformMatrix4fv(effects["post_process_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

	// Bind frame
	renderer::bind(scene.get_frame(), 0);
	glUniform1i(effects["post_process_eff"].get_uniform_location("tex"), 0);

	// Set Uniforms
	glUniform1f(effects["post_process_eff"].get_uniform_location("brightness"), PP.getBrightness());
	glUniform1f(effects["post_process_eff"].get_uniform_location("saturation"), PP.getSaturation());
	glUniform1f(effects["post_process_eff"].get_uniform_location("chromaticAbberation"), PP.getChromaticAbberation());
	glUniform1i(effects["post_process_eff"].get_uniform_location("sepia"), PP.getSepia());
	glUniform1f(effects["post_process_eff"].get_uniform_location("noiseStrength"), PP.getFilmgrain());
	glUniform1f(effects["post_process_eff"].get_uniform_location("frameTime"), frameTime);

	renderer::render(screen);

// MENU
	if (renderMenu) {
		// Bind effect
		renderer::bind(effects["menu_eff"]);

		glUniformMatrix4fv(effects["menu_eff"].get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

		// Bind texture
		renderer::bind(menu_tex[PP.getActiveMenuItem()], 0);
		glUniform1i(effects["menu_eff"].get_uniform_location("tex"), 0);

		renderer::render(menu_geom);
	}

	return true;
}
void main() {
	// Create application
	app application("The_Garden");
	// Set load content, update and render methods
	application.set_load_content(load_content);
	application.set_initialise(cam_free_initialize);
	application.set_keyboard_callback(key_callback); // keyboard callback
	application.set_update(update);
	application.set_render(render);
	// Run application
	application.run();
}
#pragma endregion