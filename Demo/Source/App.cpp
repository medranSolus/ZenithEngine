#include "App.h"
#include "GFX/Primitive.h"
#include "Data/SceneManager.h"
#include "Data/Tags.h"

template<typename T>
void App::EnableProperty(EID entity)
{
	if (!Settings::Data.all_of<T>(entity))
		Settings::Data.emplace<T>(entity);

	// Process childs
	if (Settings::Data.all_of<Children>(entity))
		for (EID child : Settings::Data.get<Children>(entity).Childs)
			EnableProperty<T>(child);
}

template<typename T>
void App::DisableProperty(EID entity)
{
	if (Settings::Data.all_of<T>(entity))
		Settings::Data.remove<T>(entity);

	// Process childs
	if (Settings::Data.all_of<Children>(entity))
		for (EID child : Settings::Data.get<Children>(entity).Childs)
			DisableProperty<T>(child);
}

void App::ProcessInput()
{
	ZE_PERF_GUARD("Input processing");
	Window::MainWindow& window = engine.Window();
	bool cameraChanged = false;

	while (window.Mouse().IsInput())
	{
		if (auto opt = window.Mouse().Read())
		{
			const auto& value = opt.value();
			if (value.IsRightDown() && window.IsCursorEnabled())
			{
				Data::RotateCamera(currentCamera,
					rotateSpeed * Utils::SafeCast<float>(value.GetDY()) / Utils::SafeCast<float>(Settings::DisplaySize.Y),
					rotateSpeed * Utils::SafeCast<float>(value.GetDX()) / Utils::SafeCast<float>(Settings::DisplaySize.Y), cameraType);
				cameraChanged = true;
			}

			switch (value.GetType())
			{
			case Window::Mouse::Event::Type::WheelForward:
			{
				if (!window.IsCursorEnabled() && moveSpeed <= MAX_MOVE_SPEED - 0.01f - FLT_EPSILON)
					moveSpeed += 0.01f;
				break;
			}
			case Window::Mouse::Event::Type::WheelBackward:
			{
				if (!window.IsCursorEnabled() && moveSpeed >= 0.012f + FLT_EPSILON)
					moveSpeed -= 0.01f;
				break;
			}
			case Window::Mouse::Event::Type::RawMove:
			{
				if (!window.IsCursorEnabled())
				{
					Data::RotateCamera(currentCamera,
						rotateSpeed * Utils::SafeCast<float>(value.GetDY()) / Utils::SafeCast<float>(Settings::DisplaySize.Y),
						rotateSpeed * Utils::SafeCast<float>(value.GetDX()) / Utils::SafeCast<float>(Settings::DisplaySize.Y), cameraType);
					cameraChanged = true;
				}
				break;
			}
			default:
				break;
			}
		}
	}

	Window::Keyboard& keyboard = window.Keyboard();
	while (keyboard.IsKeyReady())
	{
		if (auto opt = keyboard.ReadKey())
		{
			if (opt->IsDown())
			{
				switch (opt->GetCode())
				{
				case VK_ESCAPE:
				{
					window.SwitchCursor();
					break;
				}
				case VK_F1:
				{
					engine.SwitchDebugUI();
					break;
				}
				}
			}
		}
	}

	if (keyboard.IsKeyDown('W'))
	{
		Data::MoveCameraZ(currentCamera, moveSpeed, cameraType);
		cameraChanged = true;
	}
	if (keyboard.IsKeyDown('S'))
	{
		Data::MoveCameraZ(currentCamera, -moveSpeed, cameraType);
		cameraChanged = true;
	}
	if (keyboard.IsKeyDown('A'))
	{
		Data::MoveCameraX(currentCamera, -moveSpeed, cameraType);
		cameraChanged = true;
	}
	if (keyboard.IsKeyDown('D'))
	{
		Data::MoveCameraX(currentCamera, moveSpeed, cameraType);
		cameraChanged = true;
	}
	if (keyboard.IsKeyDown(VK_SPACE))
	{
		Data::MoveCameraY(currentCamera, moveSpeed, cameraType);
		cameraChanged = true;
	}
	if (keyboard.IsKeyDown('C'))
	{
		Data::MoveCameraY(currentCamera, -moveSpeed, cameraType);
		cameraChanged = true;
	}
	if (keyboard.IsKeyDown('Q'))
	{
		Data::RollCamera(currentCamera, rollSpeed, cameraType);
		cameraChanged = true;
	}
	if (keyboard.IsKeyDown('E'))
	{
		Data::RollCamera(currentCamera, -rollSpeed, cameraType);
		cameraChanged = true;
	}

	if (cameraChanged)
	{
		if (Settings::Data.try_get<ParentID>(currentCamera))
			PropagateTransformChange(currentCamera);
		else
		{
			Settings::Data.get<Data::TransformGlobal>(currentCamera) = static_cast<Data::TransformGlobal>(Settings::Data.get<Data::Transform>(currentCamera));
			if (Children* children = Settings::Data.try_get<Children>(currentCamera))
			{
				for (EID child : children->Childs)
					PropagateTransformChange(child);
			}
		}
	}
}

void App::ShowOptionsWindow()
{
	if (ImGui::Begin("Options"/*, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/))
	{
		if (ImGui::Button("Exit"))
			run = false;

		ImGui::Text("ImGui Demo window");
		ImGui::SameLine();
		if (ImGui::Button(demoWindow ? "Hide" : "Show"))
			demoWindow = !demoWindow;

		engine.ShowRenderGraphDebugUI();
	}
	ImGui::End();
}

void App::BuiltObjectTree(EID currentEntity, EID& selected)
{
	const bool children = Settings::Data.all_of<Children>(currentEntity);
	const bool expanded = ImGui::TreeNodeEx(reinterpret_cast<void*>(currentEntity),
		ImGuiTreeNodeFlags_OpenOnArrow |
		(children ? 0 : ImGuiTreeNodeFlags_Leaf) |
		(currentEntity == selected ? ImGuiTreeNodeFlags_Selected : 0),
		Settings::Data.get<std::string>(currentEntity).c_str());

	if (ImGui::IsItemClicked() && selected != currentEntity)
	{
		if (selected != INVALID_EID)
			DisableProperty<Data::RenderOutline>(selected);
		selected = currentEntity;
		EnableProperty<Data::RenderOutline>(selected);
	}

	if (expanded)
	{
		if (children)
			for (EID child : Settings::Data.get<Children>(currentEntity).Childs)
				BuiltObjectTree(child, selected);
		ImGui::TreePop();
	}
}

void App::ShowObjectWindow()
{
	static EID selected = INVALID_EID;

	if (ImGui::Begin("Objects"/*, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/))
	{
		ImGui::Columns(2);
		ImGui::BeginChild("##node_tree", { 0.0f, 231.5f }, false, ImGuiWindowFlags_HorizontalScrollbar);
		for (EID parent : Settings::Data.view<std::string>(entt::exclude<ParentID, Data::AssetsStreamer::PackID>))
		{
			BuiltObjectTree(parent, selected);
		}
		ImGui::EndChild();
		ImGui::NextColumn();
		if (selected != INVALID_EID)
		{
			constexpr float SLIDER_WIDTH = -15.0f;
			constexpr float INPUT_WIDTH = 80.0f;
			constexpr ImGuiColorEditFlags COLOR_FLAGS = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float;

			ImGui::Text("Node ID: %llu", static_cast<U64>(selected));
			ImGui::BeginChild("##node_options");
			ImGui::Columns(2, "##model_node_options", false);

			bool change = Settings::Data.all_of<Data::RenderOutline>(selected);
			if (ImGui::Checkbox("Model outline", &change))
			{
				if (change)
					EnableProperty<Data::RenderOutline>(selected);
				else
					DisableProperty<Data::RenderOutline>(selected);
			}
			change = Settings::Data.all_of<Data::RenderWireframe>(selected);
			if (ImGui::Checkbox("Wireframe", &change))
			{
				if (change)
					EnableProperty<Data::RenderWireframe>(selected);
				else
					DisableProperty<Data::RenderWireframe>(selected);
			}
			ImGui::NextColumn();
			change = Settings::Data.all_of<Data::RenderLambertian>(selected);
			if (ImGui::Checkbox("Render mesh", &change))
			{
				if (change)
					EnableProperty<Data::RenderLambertian>(selected);
				else
					DisableProperty<Data::RenderLambertian>(selected);
			}
			change = Settings::Data.all_of<Data::ShadowCaster>(selected);
			if (ImGui::Checkbox("Shadows", &change))
			{
				if (change)
					EnableProperty<Data::ShadowCaster>(selected);
				else
					DisableProperty<Data::ShadowCaster>(selected);
			}
			ImGui::Columns(1);

			if (Settings::Data.all_of<Data::Transform>(selected))
			{
				ImGui::Separator();
				ImGui::NewLine();
				auto& transform = Settings::Data.get<Data::Transform>(selected);

				change = ImGui::InputFloat3("Scale [X|Y|Z]", reinterpret_cast<float*>(&transform.Scale));
				if (transform.Scale.x < 0.001f)
					transform.Scale.x = 0.001f;
				if (transform.Scale.y < 0.001f)
					transform.Scale.y = 0.001f;
				if (transform.Scale.z < 0.001f)
					transform.Scale.z = 0.001f;
				ImGui::Columns(2, "##transform_options", false);

				ImGui::Text("Position");
				ImGui::SetNextItemWidth(SLIDER_WIDTH);
				change |= ImGui::InputFloat("X##position", &transform.Position.x, 0.1f, 0.0f, "%.2f");
				ImGui::SetNextItemWidth(SLIDER_WIDTH);
				change |= ImGui::InputFloat("Y##position", &transform.Position.y, 0.1f, 0.0f, "%.2f");
				ImGui::SetNextItemWidth(SLIDER_WIDTH);
				change |= ImGui::InputFloat("Z##position", &transform.Position.z, 0.1f, 0.0f, "%.2f");
				ImGui::NextColumn();

				Float3 rotation = Math::ToDegrees(Math::GetEulerAngles(transform.Rotation));
				ImGui::Text("Rotation");
				ImGui::SetNextItemWidth(INPUT_WIDTH);
				bool angleSet = ImGui::InputFloat("X##angle", &rotation.x, 0.0f, 0.0f, "%.2f");
				ImGui::SetNextItemWidth(INPUT_WIDTH);
				angleSet |= ImGui::InputFloat("Y##angle", &rotation.y, 0.0f, 0.0f, "%.2f");
				ImGui::SetNextItemWidth(INPUT_WIDTH);
				angleSet |= ImGui::InputFloat("Z##angle", &rotation.z, 0.0f, 0.0f, "%.2f");
				if (angleSet)
				{
					change = true;
					rotation = Math::ToRadians(rotation);
					Math::XMStoreFloat4(&transform.Rotation,
						Math::XMQuaternionRotationRollPitchYawFromVector(Math::XMLoadFloat3(&rotation)));
				}
				ImGui::Columns(1);

				if (change)
				{
					if (Settings::Data.try_get<ParentID>(selected))
						PropagateTransformChange(selected);
					else
					{
						Settings::Data.get<Data::TransformGlobal>(selected) = static_cast<Data::TransformGlobal>(transform);
						if (Children* children = Settings::Data.try_get<Children>(selected))
						{
							for (EID child : children->Childs)
								PropagateTransformChange(child);
						}
					}
				}
			}

			if (Settings::Data.all_of<Data::MaterialID>(selected))
			{
				ImGui::Separator();
				ImGui::NewLine();
				EID materialId = Settings::Data.get<Data::MaterialID>(selected).ID;
				auto& material = Settings::Data.get<Data::MaterialPBR>(materialId);
				auto pbrFlags = Settings::Data.get<Data::PBRFlags>(materialId);

				ImGui::Text("Material ID: %llu", materialId);
				if (Settings::Data.all_of<std::string>(materialId))
				{
					ImGui::SameLine();
					ImGui::Text("Name: %s", Settings::Data.get<std::string>(materialId).c_str());
				}

				change = ImGui::ColorEdit4("Albedo", reinterpret_cast<float*>(&material.Albedo),
					COLOR_FLAGS | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);

				auto useTexture = [pbrFlags, &material, &change](const char* text, Data::MaterialPBR::Flag flag)
					{
						ImGui::BeginDisabled(!(pbrFlags & flag));
						bool useTex = material.Flags & flag;
						ImGui::Checkbox(text, &useTex);
						if (useTex != static_cast<bool>(material.Flags & flag))
						{
							change = true;
							if (useTex)
								material.Flags |= flag;
							else
								material.Flags &= ~flag;
						}
						ImGui::EndDisabled();
					};

				ImGui::Columns(2, "##pbr_params", false);
				useTexture("Use albedo texture", Data::MaterialPBR::Flag::UseAlbedoTex);

				ImGui::Text("Metalness");
				ImGui::SetNextItemWidth(-1.0f);
				change |= GUI::InputClamp(0.0f, 1.0f, material.Metalness,
					ImGui::InputFloat("##metalness", &material.Metalness, 0.1f, 0.0f, "%.3f"));

				useTexture("Use metal texture", Data::MaterialPBR::Flag::UseMetalnessTex);
				ImGui::NextColumn();

				useTexture("Use normal texture", Data::MaterialPBR::Flag::UseNormalTex);
				ImGui::Text("Roughness");
				ImGui::SetNextItemWidth(-1.0f);
				change |= GUI::InputClamp(0.0f, 1.0f, material.Roughness,
					ImGui::InputFloat("##roughness", &material.Roughness, 0.1f, 0.0f, "%.3f"));
				useTexture("Use roughness texture", Data::MaterialPBR::Flag::UseRoughnessTex);
				ImGui::Columns(1);

				change |= ImGui::InputFloat("Parallax scale", &material.ParallaxScale, 0.01f, 0.0f, "%.2f");
				if (material.ParallaxScale < 0.0f)
					material.ParallaxScale = 0.0f;
				useTexture("Use parallax texture", Data::MaterialPBR::Flag::UseParallaxTex);

				if (change)
					Settings::Data.get<Data::MaterialBuffersPBR>(materialId).UpdateData(engine.Gfx().GetDevice(), engine.Assets().GetDisk(), materialId, material);
			}

			if (Settings::Data.all_of<Data::PointLight>(selected))
			{
				ImGui::Separator();
				ImGui::NewLine();
				auto& light = Settings::Data.get<Data::PointLight>(selected);

				ImGui::Text("Point Light Intensity");
				change = ImGui::InputFloat("##point_intensity", &light.Intensity, 0.001f, 0.0f, "%.3f");
				if (light.Intensity < 0.0f)
					light.Intensity = 0.0f;

				ImGui::Text("Color:");
				change |= ImGui::ColorEdit3("Light##point", reinterpret_cast<float*>(&light.Color), COLOR_FLAGS);

				ImGui::Text("Attenuation:");
				change |= ImGui::InputFloat("Linear##point", &light.AttnLinear, 0.01f, 0.0f, "%.2f");
				change |= ImGui::InputFloat("Quad##point", &light.AttnQuad, 0.001f, 0.0f, "%.3f");

				if (change)
				{
					auto& buffer = Settings::Data.get<Data::PointLightBuffer>(selected);

					buffer.Buffer.Update(engine.Gfx().GetDevice(), engine.Assets().GetDisk(), { selected, &light, nullptr, sizeof(light) });
					buffer.Volume = Math::Light::GetLightVolume(light.Color, light.Intensity, light.AttnLinear, light.AttnQuad);
				}
			}

			if (Settings::Data.all_of<Data::SpotLight>(selected))
			{
				ImGui::Separator();
				ImGui::NewLine();
				auto& light = Settings::Data.get<Data::SpotLight>(selected);

				ImGui::Text("Spot Light Intensity");
				change = ImGui::InputFloat("##spot_intensity", &light.Intensity, 0.001f, 0.0f, "%.3f");
				if (light.Intensity < 0.0f)
					light.Intensity = 0.0f;

				ImGui::Text("Color:");
				change |= ImGui::ColorEdit3("Light##point", reinterpret_cast<float*>(&light.Color), COLOR_FLAGS);

				ImGui::Text("Attenuation:");
				change |= ImGui::InputFloat("Linear##point", &light.AttnLinear, 0.01f, 0.0f, "%.2f");
				change |= ImGui::InputFloat("Quad##point", &light.AttnQuad, 0.001f, 0.0f, "%.3f");

				ImGui::Text("Direction [X|Y|Z]");
				ImGui::SetNextItemWidth(-5.0f);
				if (ImGui::SliderFloat3("##spot_dir", reinterpret_cast<float*>(&light.Direction), -1.0f, 1.0f, "%.2f"))
				{
					Math::NormalizeStore(light.Direction);
					change = true;
				}

				ImGui::Columns(2, "##spotlight", false);
				ImGui::Text("Outer angle");
				ImGui::SetNextItemWidth(-1.0f);
				change |= ImGui::SliderAngle("##outer_spot", &light.OuterAngle, 0.0f, 45.0f, "%.2f");
				if (light.InnerAngle > light.OuterAngle)
				{
					light.InnerAngle = light.OuterAngle;
					change = true;
				}
				ImGui::NextColumn();
				ImGui::Text("Inner angle");
				ImGui::SetNextItemWidth(-1.0f);
				change |= ImGui::SliderAngle("##inner_spot", &light.InnerAngle, 0.0f, Math::ToDegrees(light.OuterAngle), "%.2f");
				ImGui::Columns(1);

				if (change)
				{
					auto& buffer = Settings::Data.get<Data::SpotLightBuffer>(selected);

					buffer.Buffer.Update(engine.Gfx().GetDevice(), engine.Assets().GetDisk(), { selected, &light, nullptr, sizeof(light) });
					buffer.Volume = Math::Light::GetLightVolume(light.Color, light.Intensity, light.AttnLinear, light.AttnQuad);
				}
			}

			if (Settings::Data.all_of<Data::DirectionalLight>(selected))
			{
				ImGui::Separator();
				ImGui::NewLine();
				auto& light = Settings::Data.get<Data::DirectionalLight>(selected);

				ImGui::Text("Directional Light Intensity");
				change = ImGui::InputFloat("##dir_intensity", &light.Intensity, 0.001f, 0.0f, "%.3f");
				if (light.Intensity < 0.0f)
					light.Intensity = 0.0f;

				ImGui::Text("Color:");
				change |= ImGui::ColorEdit3("Light##point", reinterpret_cast<float*>(&light.Color), COLOR_FLAGS);

				ImGui::Text("Direction [X|Y|Z]");
				ImGui::SetNextItemWidth(-5.0f);
				if (ImGui::SliderFloat3("##spot_dir", reinterpret_cast<float*>(&Settings::Data.get<Data::Direction>(selected).Direction), -1.0f, 1.0f, "%.2f"))
				{
					Math::NormalizeStore(Settings::Data.get<Data::Direction>(selected).Direction);
					change = true;
				}

				if (change)
					Settings::Data.get<Data::DirectionalLightBuffer>(selected).Buffer.Update(engine.Gfx().GetDevice(),
						engine.Assets().GetDisk(), { selected, &light, nullptr, sizeof(light) });
			}

			if (Settings::Data.all_of<Data::Camera>(selected))
			{
				ImGui::Separator();
				ImGui::NewLine();
				auto& camera = Settings::Data.get<Data::Camera>(selected);

				ImGui::Text("FOV");
				ImGui::SetNextItemWidth(-1.0f);
				change = ImGui::SliderAngle("##fov", &camera.Projection.FOV, 1.0f, 179.0f, "%.1f");
				ImGui::NextColumn();

				ImGui::Text("Ratio");
				ImGui::SetNextItemWidth(-1.0f);
				change |= ImGui::SliderFloat("##screen_ratio", &camera.Projection.ViewRatio, 0.1f, 5.0f, "%.2f");
				ImGui::Columns(2, "##camera", false);

				ImGui::Text("Near clip");
				ImGui::SetNextItemWidth(-1.0f);
				change |= GUI::InputClamp(0.1f, 20.0f, camera.Projection.NearClip,
					ImGui::InputFloat("##near_clip", &camera.Projection.NearClip, 0.01f, 0.0f, "%.3f"));
			}
			ImGui::EndChild();
		}
	}
	ImGui::End();
}

void App::PropagateTransformChange(EID childEntity)
{
	ZE_VALID_EID(childEntity);
	ZE_ASSERT(Settings::Data.try_get<ParentID>(childEntity), "Incorrect child-parent structure defined!");

	auto& parent = Settings::Data.get<Data::TransformGlobal>(Settings::Data.get<ParentID>(childEntity).ID);
	auto& global = Settings::Data.get<Data::TransformGlobal>(childEntity);
	auto& local = Settings::Data.get<Data::Transform>(childEntity);
	Math::XMStoreFloat4(&global.Rotation,
		Math::XMQuaternionNormalize(Math::XMQuaternionMultiply(Math::XMLoadFloat4(&parent.Rotation), Math::XMLoadFloat4(&local.Rotation))));
	Math::XMStoreFloat3(&global.Position,
		Math::XMVectorAdd(Math::XMLoadFloat3(&parent.Position), Math::XMLoadFloat3(&local.Position)));
	Math::XMStoreFloat3(&global.Scale,
		Math::XMVectorMultiply(Math::XMLoadFloat3(&parent.Scale), Math::XMLoadFloat3(&local.Scale)));

	if (Children* children = Settings::Data.try_get<Children>(childEntity))
	{
		for (EID child : children->Childs)
			PropagateTransformChange(child);
	}
}

EID App::AddCamera(std::string&& name, float nearZ, float fov,
	Float3&& position, const Float3& angle)
{
	EID camera = Settings::CreateEntity();
	Settings::Data.emplace<std::string>(camera, std::move(name));

	const Vector rotor = Math::XMQuaternionRotationRollPitchYaw(Math::ToRadians(angle.x), Math::ToRadians(angle.y), Math::ToRadians(angle.z));

	Float4 roation;
	Float3 eyeVector, upVector;
	Math::XMStoreFloat3(&eyeVector, Math::XMVector3Rotate({ 0.0f, 0.0f, 1.0f, 0.0f }, rotor));
	Math::XMStoreFloat3(&upVector, Math::XMVector3Rotate({ 0.0f, 1.0f, 0.0f, 0.0f }, rotor));
	Math::XMStoreFloat4(&roation, rotor);

	Settings::Data.emplace<Data::Camera>(camera,
		Data::Camera(std::move(eyeVector), std::move(upVector),
			{
				Math::ToRadians(60.0f),
				Settings::GetDisplayRatio(),
				nearZ
			}));

	Settings::Data.emplace<Data::TransformGlobal>(camera,
		Settings::Data.emplace<Data::Transform>(camera,
			std::move(roation), std::move(position), Math::UnitScale()));

	return camera;
}

EID App::AddModel(std::string&& name, Float3&& position,
	const Float3& angle, float scale, const std::string& file, Data::ExternalModelOptions options)
{
	EID model = Settings::CreateEntity();
	Settings::Data.emplace<std::string>(model, std::move(name));
	Data::Transform transform = { Math::GetQuaternion(angle.x, angle.y, angle.z), std::move(position), Float3(scale, scale, scale) };

	if (!Data::LoadExternalModel(engine.Gfx().GetDevice(), engine.Assets(), model, transform, file, options).Get())
	{
		// TODO: correct error handling
		ZE_FAIL("Error loading model!");
	}
	return model;
}

EID App::AddPointLight(std::string&& name, Float3&& position,
	ColorF3&& color, float intensity, U64 range)
{
	EID light = Settings::CreateEntity();
	Settings::Data.emplace<std::string>(light, std::move(name));
	Settings::Data.emplace<Data::LightPoint>(light);

	Settings::Data.emplace<Data::TransformGlobal>(light,
		Settings::Data.emplace<Data::Transform>(light, Math::NoRotation(), std::move(position), Math::UnitScale()));

	Data::PointLight& pointLight = Settings::Data.emplace<Data::PointLight>(light, std::move(color), intensity);
	pointLight.SetAttenuationRange(range);

	Data::PointLightBuffer& buffer = Settings::Data.emplace<Data::PointLightBuffer>(light,
		Math::Light::GetLightVolume(pointLight.Color, pointLight.Intensity, pointLight.AttnLinear, pointLight.AttnQuad));
	buffer.Buffer.Init(engine.Gfx().GetDevice(), engine.Assets().GetDisk(), { light, &pointLight, nullptr, sizeof(Data::PointLight) });

	return light;
}

EID App::AddSpotLight(std::string&& name, Float3&& position,
	ColorF3&& color, float intensity, U64 range,
	float innerAngle, float outerAngle, const Float3& direction)
{
	EID light = Settings::CreateEntity();
	Settings::Data.emplace<std::string>(light, std::move(name));
	Settings::Data.emplace<Data::LightSpot>(light);

	Settings::Data.emplace<Data::TransformGlobal>(light,
		Settings::Data.emplace<Data::Transform>(light,
			Math::NoRotation(), std::move(position), Math::UnitScale()));

	Data::SpotLight& spotLight = Settings::Data.emplace<Data::SpotLight>(light, std::move(color),
		intensity, Math::NormalizeReturn(direction), Math::ToRadians(innerAngle), Math::ToRadians(outerAngle));
	spotLight.SetAttenuationRange(range);

	Data::SpotLightBuffer& buffer = Settings::Data.emplace<Data::SpotLightBuffer>(light,
		Math::Light::GetLightVolume(spotLight.Color, spotLight.Intensity, spotLight.AttnLinear, spotLight.AttnQuad));
	buffer.Buffer.Init(engine.Gfx().GetDevice(), engine.Assets().GetDisk(), { light, &spotLight, nullptr, sizeof(Data::SpotLight) });

	return light;
}

EID App::AddDirectionalLight(std::string&& name,
	ColorF3&& color, float intensity, const Float3& direction)
{
	EID light = Settings::CreateEntity();
	Settings::Data.emplace<std::string>(light, std::move(name));
	Settings::Data.emplace<Data::LightDirectional>(light);

	Data::DirectionalLight& dirLight = Settings::Data.emplace<Data::DirectionalLight>(light, std::move(color), intensity);
	Settings::Data.emplace<Data::Direction>(light, Math::NormalizeReturn(direction));

	Data::DirectionalLightBuffer& buffer = Settings::Data.emplace<Data::DirectionalLightBuffer>(light);
	buffer.Buffer.Init(engine.Gfx().GetDevice(), engine.Assets().GetDisk(), { light, &dirLight, nullptr, sizeof(Data::DirectionalLight) });

	return light;
}

void App::MakeFrame()
{
	ZE_PERF_GUARD("Frame rendering");
	if (demoWindow)
		ImGui::ShowDemoWindow();
	if (Settings::IsEnabledImGui())
	{
		ZE_PERF_GUARD("GUI");
		ShowOptionsWindow();
		ShowObjectWindow();
	}
}

App::App(const CmdParser& params)
	: engine(SettingsInitParams::GetParsedParams(params, APP_NAME, Settings::ENGINE_VERSION, 0))
{
	EngineParams engineParams = {};
	EngineParams::SetParsedParams(params, engineParams);
	engineParams.WindowName = WINDOW_TITLE;
	engineParams.CoreRendererParams.BrdfLutSource = "";
	engineParams.CoreRendererParams.SkyboxSource.InitFolder("Skybox/Space", ".png");
	engineParams.CoreRendererParams.EnvMapSource = engineParams.CoreRendererParams.SkyboxSource;
	engine.Init(engineParams);

	engine.ImGui().SetFont("Fonts/Arial.ttf", 14.0f);

	if (params.GetOption("cubePerfTest"))
	{
		currentCamera = AddCamera("Main camera", 0.075f, 60.0f, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });

		AddDirectionalLight("Sun", { 0.7608f, 0.7725f, 0.8f }, 5.0f, { 0.15f, -1.0f, 0.05f });

		// Create mesh for all the cubes
		EID meshId = Settings::CreateEntity();
		Settings::Data.emplace<std::string>(meshId, "Cube");
		Settings::Data.emplace<Math::BoundingBox>(meshId, GFX::Primitive::Cube::MakeBoundingBox());

		std::vector<U16> indices = GFX::Primitive::Cube::MakeIndex();
		std::vector<GFX::Vertex> vertices = GFX::Primitive::Cube::MakeVertex(indices);
		GFX::Resource::MeshData meshData =
		{
			meshId, nullptr,
			Utils::SafeCast<U32>(vertices.size()),
			Utils::SafeCast<U32>(indices.size()),
			sizeof(GFX::Vertex), 0
		};
		meshData.PackedMesh = GFX::Primitive::GetPackedMeshPackIndex(vertices, indices, meshData.IndexSize);
		Settings::Data.emplace<GFX::Resource::Mesh>(meshId, engine.Gfx().GetDevice(), engine.Assets().GetDisk(), meshData);

		// And some materials for them all
		std::array<EID, 255> materialIds;
		for (U32 i = 0; EID& materialId : materialIds)
		{
			materialId = Settings::CreateEntity();
			Settings::Data.emplace<std::string>(materialId, "Cube_mat_" + std::to_string(i++));

			Data::MaterialPBR& data = Settings::Data.emplace<Data::MaterialPBR>(materialId);
			Data::MaterialBuffersPBR& buffers = Settings::Data.emplace<Data::MaterialBuffersPBR>(materialId);
			Settings::Data.emplace<Data::PBRFlags>(materialId);

			const float seed = static_cast<float>(i) / static_cast<float>(materialIds.size());
			data.Albedo = { seed, seed * 0.8f, 1.0f - seed, 1.0f };
			data.Metalness = seed;
			data.Roughness = 1.0f - seed;
			data.ParallaxScale = 0.1f;

			const GFX::Resource::Texture::Schema& texSchema = engine.Assets().GetSchemaLib().Get(Data::MaterialPBR::TEX_SCHEMA_NAME);
			GFX::Resource::Texture::PackDesc texDesc;
			texDesc.Init(texSchema);
			buffers.Init(engine.Gfx().GetDevice(), engine.Assets().GetDisk(), data, texDesc);
		}

		// Create test cube entities
		std::mt19937_64 randEngine;
		for (U32 i = 0, size = params.GetNumber("cubePerfTestSize"); i < size; ++i)
		{
			EID model = Settings::CreateEntity();
			Settings::Data.emplace<std::string>(model, "Cube_" + std::to_string(i));

			const float angleX = Math::Rand(0.0f, 360.0f, randEngine);
			const float angleY = Math::Rand(0.0f, 360.0f, randEngine);
			const float angleZ = Math::Rand(0.0f, 360.0f, randEngine);
			const float scale = Math::Rand(0.5f, 5.0f, randEngine);

			auto& transform = Settings::Data.emplace<Data::TransformGlobal>(model,
				Settings::Data.emplace<Data::Transform>(model,
					Math::GetQuaternion(angleX, angleY, angleZ),
					Math::RandPosition(-200.0f, 200.0f, randEngine),
					Float3(scale, scale, scale)));
			if (Settings::ComputeMotionVectors())
				Settings::Data.emplace<Data::TransformPrevious>(model, transform);

			Settings::Data.emplace<Data::RenderLambertian>(model);
			Settings::Data.emplace<Data::ShadowCaster>(model);
			Settings::Data.emplace<Data::MeshID>(model, meshId);
			Settings::Data.emplace<Data::MaterialID>(model, materialIds.at(i % materialIds.size()));
		}
	}
	else if (params.GetOption("lightParamsTest"))
	{
		currentCamera = AddCamera("Main camera", 0.075f, 60.0f, { 0.0f, 1.5f, -23.0f }, { 0.0f, 0.0f, 0.0f });

		AddDirectionalLight("Sun", { 0.7608f, 0.7725f, 0.8f }, 5.0f, { 0.57f, -0.58f, 0.59f });

		// Mesh data for test sphere
		EID meshId = Settings::CreateEntity();
		Settings::Data.emplace<std::string>(meshId, "IcoSphere_6");
		Settings::Data.emplace<Math::BoundingBox>(meshId, GFX::Primitive::Sphere::MakeBoundingBox());

		GFX::Primitive::Data<GFX::Vertex> sphere = GFX::Primitive::Sphere::MakeIco(6);
		GFX::Resource::MeshData meshData =
		{
			meshId, nullptr,
			Utils::SafeCast<U32>(sphere.Vertices.size()),
			Utils::SafeCast<U32>(sphere.Indices.size()),
			sizeof(GFX::Vertex), 0
		};
		meshData.PackedMesh = GFX::Primitive::GetPackedMeshPackIndex(sphere.Vertices, sphere.Indices, meshData.IndexSize);
		Settings::Data.emplace<GFX::Resource::Mesh>(meshId, engine.Gfx().GetDevice(), engine.Assets().GetDisk(), meshData);

		// Create test entities
		U32 testSize = params.GetNumber("lightParamsTestSize");
		if (testSize == 0)
			testSize = 10;
		S32 positionOffset = testSize >> 1;
		for (U32 metalness = 1; metalness <= testSize; ++metalness)
		{
			for (U32 roughness = 1; roughness <= testSize; ++roughness)
			{
				// Material creation
				EID materialId = Settings::CreateEntity();
				Settings::Data.emplace<std::string>(materialId, "Sphere_mat_Rgh_" + std::to_string(metalness) + "_Mtl_" + std::to_string(roughness));

				Data::MaterialPBR& data = Settings::Data.emplace<Data::MaterialPBR>(materialId);
				Data::MaterialBuffersPBR& buffers = Settings::Data.emplace<Data::MaterialBuffersPBR>(materialId);
				Settings::Data.emplace<Data::PBRFlags>(materialId);

				data.Albedo = { 1.0f, 0.0, 0.0f };
				data.Metalness = static_cast<float>(metalness) / static_cast<float>(testSize);
				data.Roughness = static_cast<float>(roughness) / static_cast<float>(testSize);

				const GFX::Resource::Texture::Schema& texSchema = engine.Assets().GetSchemaLib().Get(Data::MaterialPBR::TEX_SCHEMA_NAME);
				GFX::Resource::Texture::PackDesc texDesc;
				texDesc.Init(texSchema);
				buffers.Init(engine.Gfx().GetDevice(), engine.Assets().GetDisk(), data, texDesc);

				// Object creation
				EID model = Settings::CreateEntity();
				Settings::Data.emplace<std::string>(model, "Sphere_Rgh_" + std::to_string(metalness) + "_Mtl_" + std::to_string(roughness));

				auto& transform = Settings::Data.emplace<Data::TransformGlobal>(model,
					Settings::Data.emplace<Data::Transform>(model,
						Math::NoRotation(),
						Float3(static_cast<float>(static_cast<S32>(roughness) - positionOffset) * 2.5f, static_cast<float>(static_cast<S32>(metalness) - positionOffset) * 2.5f, 0.0f),
						Math::UnitScale()));
				if (Settings::ComputeMotionVectors())
					Settings::Data.emplace<Data::TransformPrevious>(model, transform);

				Settings::Data.emplace<Data::RenderLambertian>(model);
				Settings::Data.emplace<Data::ShadowCaster>(model);
				Settings::Data.emplace<Data::MeshID>(model, meshId);
				Settings::Data.emplace<Data::MaterialID>(model, materialId);
			}
		}
	}
	else
	{
		// Sample Scene
		currentCamera = AddCamera("Main camera", 0.075f, 60.0f, { 0.0f, 2.0f, 0.0f }, { 0.0f, 90.0f, 0.0f });

		AddPointLight("Light bulb", { -2.4f, 2.8f, -1.1f }, { 1.0f, 1.0f, 1.0f }, 0.5f, 50);
		AddModel("Sting sword", { -1.9f, 2.1f, -2.3f }, { 35.0f, 270.0f, 110.0f }, 0.045f, "Models/Sting_Sword/Sting_Sword.obj",
			Base(Data::ExternalModelOption::FlipUV));

#if !_ZE_MODE_DEBUG
		AddPointLight("Blue ilumination", { 10.8f, 5.9f, -0.1f }, { 0.0f, 0.46f, 1.0f }, 10.0f, 60);
		AddPointLight("Torch", { 15.6f, 3.2f, 0.9f }, { 1.0f, 0.0f, 0.2f }, 3.2f, 70);

		AddSpotLight("Space light", { 4.8f, 22.7f, -3.1f }, { 1.3f, 2.3f, 1.3f }, 3.5f, 126, 15.0f, 24.5f, { -0.64f, -1.0f, 0.5f });
		AddSpotLight("Lion flare", { -14.0f, 0.5f, 2.2f }, { 0.8f, 0.0f, 0.8f }, 5.0f, 150, 35.0f, 45.0f, { -1.0f, 1.0f, -0.7f });
		AddSpotLight("Dragon flame", { -6.5f, 0.0f, -2.9f }, { 0.04f, 0.0f, 0.52f }, 4.0f, 175, 27.0f, 43.0f, { -0.6f, 0.75f, 0.3 });

		AddDirectionalLight("Moon", { 0.7608f, 0.7725f, 0.8f }, 0.1f, { 0.0f, -0.7f, -0.7f });

		AddModel("TIE", { 1.2f, 6.7f, 0.2f }, { -23.2f, 9.41f, -28.72f }, 1.0f, "Models/tie/tie.obj",
			static_cast<Data::ExternalModelOptions>(Data::ExternalModelOption::FlipUV));

		if (!params.GetOption("noExternalAssets"))
		{
			AddModel("Sponza", { 0.0f, 0.0f, 0.0f }, Math::NoRotationAngles(), 1.0f, "Models/SponzaIntel/NewSponza_Main_glTF_002.gltf",
				Data::ExternalModelOption::ExtractMetalnessChannelB | Data::ExternalModelOption::ExtractRoughnessChannelG | Data::ExternalModelOption::FlipUV);
		}
#endif
	}
}

int App::Run()
{
	constexpr double DELTA_TIME = 0.01;

	engine.Start(currentCamera);
	double accumulator = 0.0;
	while (run)
	{
		accumulator += engine.BeginFrame(DELTA_TIME, 25);

		ZE_PERF_START("Input gather");
		const std::pair<bool, int> status = engine.Window().ProcessMessage();
		ZE_PERF_STOP();

		if (status.first)
			return status.second;

		ProcessInput();
		//scene.UpdateTransforms();

		while (accumulator >= DELTA_TIME)
		{
			// Physics
			accumulator -= DELTA_TIME;
		}
		//const double alpha = accumulator / DELTA_TIME;

		MakeFrame();

		engine.EndFrame();
	}
	return 0;
}