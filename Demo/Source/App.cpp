#include "App.h"
#include "GFX/Primitive.h"

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
	while (window.Mouse().IsInput())
	{
		if (auto opt = window.Mouse().Read())
		{
			const auto& value = opt.value();
			if (value.IsRightDown() && window.IsCursorEnabled())
				Data::RotateCamera(currentCamera,
					rotateSpeed * Utils::SafeCast<float>(value.GetDY()) / Utils::SafeCast<float>(Settings::DisplaySize.Y),
					rotateSpeed * Utils::SafeCast<float>(value.GetDX()) / Utils::SafeCast<float>(Settings::DisplaySize.Y), cameraType);

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
					Data::RotateCamera(currentCamera,
						rotateSpeed * Utils::SafeCast<float>(value.GetDY()) / Utils::SafeCast<float>(Settings::DisplaySize.Y),
						rotateSpeed * Utils::SafeCast<float>(value.GetDX()) / Utils::SafeCast<float>(Settings::DisplaySize.Y), cameraType);
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
					engine.ToggleGui();
					break;
				}
				}
			}
		}
	}

	if (keyboard.IsKeyDown('W'))
		Data::MoveCameraZ(currentCamera, moveSpeed, cameraType);
	if (keyboard.IsKeyDown('S'))
		Data::MoveCameraZ(currentCamera, -moveSpeed, cameraType);
	if (keyboard.IsKeyDown('A'))
		Data::MoveCameraX(currentCamera, -moveSpeed, cameraType);
	if (keyboard.IsKeyDown('D'))
		Data::MoveCameraX(currentCamera, moveSpeed, cameraType);
	if (keyboard.IsKeyDown(VK_SPACE))
		Data::MoveCameraY(currentCamera, moveSpeed, cameraType);
	if (keyboard.IsKeyDown('C'))
		Data::MoveCameraY(currentCamera, -moveSpeed, cameraType);
	if (keyboard.IsKeyDown('Q'))
		Data::RollCamera(currentCamera, rollSpeed, cameraType);
	if (keyboard.IsKeyDown('E'))
		Data::RollCamera(currentCamera, -rollSpeed, cameraType);
}

void App::ShowOptionsWindow()
{
	if (ImGui::Begin("Options"/*, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/))
	{
		if (ImGui::Button("Exit"))
			run = false;
		ImGui::SameLine();
		switch (Settings::GetGfxApi())
		{
		case GfxApiType::DX11:
		{
			ImGui::Text("DirectX 11");
			break;
		}
		case GfxApiType::DX12:
		{
			ImGui::Text("DirectX 12");
			break;
		}
		case GfxApiType::OpenGL:
		{
			ImGui::Text("OpenGL");
			break;
		}
		case GfxApiType::Vulkan:
		{
			ImGui::Text("Vulkan");
			break;
		}
		}
		ImGui::SameLine();
		ImGui::Text("FPS: %.2f", ImGui::GetIO().Framerate);
		engine.Reneder().ShowWindow(engine.Gfx().GetDevice());
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
		for (EID parent : Settings::Data.view<std::string>(entt::exclude<ParentID>))
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

				ImGui::InputFloat3("Scale [X|Y|Z]", reinterpret_cast<float*>(&transform.Scale));
				if (transform.Scale.x < 0.001f)
					transform.Scale.x = 0.001f;
				if (transform.Scale.y < 0.001f)
					transform.Scale.y = 0.001f;
				if (transform.Scale.z < 0.001f)
					transform.Scale.z = 0.001f;
				ImGui::Columns(2, "##transform_options", false);

				ImGui::Text("Position");
				ImGui::SetNextItemWidth(SLIDER_WIDTH);
				ImGui::InputFloat("X##position", &transform.Position.x, 0.1f, 0.0f, "%.2f");
				ImGui::SetNextItemWidth(SLIDER_WIDTH);
				ImGui::InputFloat("Y##position", &transform.Position.y, 0.1f, 0.0f, "%.2f");
				ImGui::SetNextItemWidth(SLIDER_WIDTH);
				ImGui::InputFloat("Z##position", &transform.Position.z, 0.1f, 0.0f, "%.2f");
				ImGui::NextColumn();

				Float3 rotation = Math::ToDegrees(Math::GetEulerAngles(transform.Rotation));
				ImGui::Text("Rotation");
				ImGui::SetNextItemWidth(INPUT_WIDTH);
				bool angleSet = ImGui::InputFloat("X##angle", &rotation.x, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::SetNextItemWidth(INPUT_WIDTH);
				angleSet |= ImGui::InputFloat("Y##angle", &rotation.y, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::SetNextItemWidth(INPUT_WIDTH);
				angleSet |= ImGui::InputFloat("Z##angle", &rotation.z, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue);
				if (angleSet)
				{
					rotation = Math::ToRadians(rotation);
					Math::XMStoreFloat4(&transform.Rotation,
						Math::XMQuaternionRotationRollPitchYawFromVector(Math::XMLoadFloat3(&rotation)));
				}
				ImGui::Columns(1);
			}

			if (Settings::Data.all_of<Data::MaterialID>(selected))
			{
				ImGui::Separator();
				ImGui::NewLine();
				EID materialId = Settings::Data.get<Data::MaterialID>(selected).ID;
				auto& material = engine.GetAssetsStreamer().GetResources().get<Data::MaterialPBR>(materialId);

				ImGui::Text("Material ID: %llu", materialId);
				if (engine.GetAssetsStreamer().GetResources().all_of<std::string>(materialId))
				{
					ImGui::SameLine();
					ImGui::Text("Name: %s", engine.GetAssetsStreamer().GetResources().get<std::string>(materialId).c_str());
				}

				change = ImGui::ColorEdit4("Material color", reinterpret_cast<float*>(&material.Color),
					COLOR_FLAGS | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
				change |= ImGui::ColorEdit3("Specular color", reinterpret_cast<float*>(&material.Specular), COLOR_FLAGS);

				ImGui::Columns(2, "##specular", false);
				ImGui::Text("Specular intensity");
				ImGui::SetNextItemWidth(-1.0f);
				change |= ImGui::InputFloat("##spec_int", &material.SpecularIntensity, 0.1f, 0.0f, "%.2f");
				if (material.SpecularIntensity < 0.01f)
					material.SpecularIntensity = 0.01f;
				ImGui::NextColumn();

				ImGui::Text("Specular power");
				ImGui::SetNextItemWidth(-1.0f);
				change |= ImGui::InputFloat("##spec_pow", &material.SpecularPower, 0.001f, 0.0f, "%.3f");
				if (material.SpecularPower < 0.001f)
					material.SpecularPower = 0.001f;
				else if (material.SpecularPower > 1.0f)
					material.SpecularPower = 1.0f;
				ImGui::Columns(1);

				bool useSpecularAlpha = material.Flags & Data::MaterialPBR::UseSpecularPowerAlpha;
				change |= ImGui::Checkbox("Use specular map alpha", &useSpecularAlpha);
				if (useSpecularAlpha)
					material.Flags |= Data::MaterialPBR::UseSpecularPowerAlpha;
				else
					material.Flags &= ~Data::MaterialPBR::UseSpecularPowerAlpha;

				change |= ImGui::InputFloat("Parallax scale", &material.ParallaxScale, 0.01f, 0.0f, "%.2f");
				if (material.ParallaxScale < 0.0f)
					material.ParallaxScale = 0.0f;

				if (change)
				{
					auto& buffers = engine.GetAssetsStreamer().GetResources().get<Data::MaterialBuffersPBR>(materialId);
					auto& dev = engine.Gfx().GetDevice();

					dev.BeginUploadRegion();
					buffers.UpdateData(dev, material);
					dev.StartUpload();
					dev.EndUploadRegion();
				}
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
				change |= ImGui::ColorEdit3("Shadow##point", reinterpret_cast<float*>(&light.Shadow), COLOR_FLAGS);

				ImGui::Text("Attenuation:");
				change |= ImGui::InputFloat("Linear##point", &light.AttnLinear, 0.01f, 0.0f, "%.2f");
				change |= ImGui::InputFloat("Quad##point", &light.AttnQuad, 0.001f, 0.0f, "%.3f");

				if (change)
				{
					auto& buffer = Settings::Data.get<Data::PointLightBuffer>(selected);
					auto& dev = engine.Gfx().GetDevice();

					dev.BeginUploadRegion();
					buffer.Buffer.Update(dev, &light, sizeof(light));
					dev.StartUpload();
					buffer.Volume = Math::GetLightVolume(light.Color, light.Intensity, light.AttnLinear, light.AttnQuad);
					dev.EndUploadRegion();
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
				change |= ImGui::ColorEdit3("Shadow##point", reinterpret_cast<float*>(&light.Shadow), COLOR_FLAGS);

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
					auto& dev = engine.Gfx().GetDevice();

					dev.BeginUploadRegion();
					buffer.Buffer.Update(dev, &light, sizeof(light));
					dev.StartUpload();
					buffer.Volume = Math::GetLightVolume(light.Color, light.Intensity, light.AttnLinear, light.AttnQuad);
					dev.EndUploadRegion();
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
				change |= ImGui::ColorEdit3("Shadow##point", reinterpret_cast<float*>(&light.Shadow), COLOR_FLAGS);

				ImGui::Text("Direction [X|Y|Z]");
				ImGui::SetNextItemWidth(-5.0f);
				if (ImGui::SliderFloat3("##spot_dir", reinterpret_cast<float*>(&Settings::Data.get<Data::Direction>(selected).Direction), -1.0f, 1.0f, "%.2f"))
				{
					Math::NormalizeStore(Settings::Data.get<Data::Direction>(selected).Direction);
					change = true;
				}

				if (change)
				{
					auto& buffer = Settings::Data.get<Data::DirectionalLightBuffer>(selected);
					auto& dev = engine.Gfx().GetDevice();

					dev.BeginUploadRegion();
					buffer.Buffer.Update(dev, &light, sizeof(light));
					dev.StartUpload();
					dev.EndUploadRegion();
				}
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

				if (selected == currentCamera && change)
					engine.Reneder().UpdateSettingsData(camera.Projection);
			}
			ImGui::EndChild();
		}
	}
	ImGui::End();
}

EID App::AddCamera(std::string&& name, float nearZ, float fov,
	Float3&& position, const Float3& angle)
{
	EID camera = Settings::Data.create();
	Settings::Data.emplace<std::string>(camera, std::move(name));

	const Vector rotor = Math::XMQuaternionRotationRollPitchYaw(Math::ToRadians(angle.x), Math::ToRadians(angle.y), Math::ToRadians(angle.z));

	Float4 roation;
	Float3 eyeVector, upVector;
	XMStoreFloat3(&eyeVector, Math::XMVector3Rotate({ 0.0f, 0.0f, 1.0f, 0.0f }, rotor));
	XMStoreFloat3(&upVector, Math::XMVector3Rotate({ 0.0f, 1.0f, 0.0f, 0.0f }, rotor));
	XMStoreFloat4(&roation, rotor);

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
	const Float3& angle, float scale, const std::string& file)
{
	EID model = Settings::Data.create();
	Settings::Data.emplace<std::string>(model, std::move(name));

	auto& transform = Settings::Data.emplace<Data::TransformGlobal>(model,
		Settings::Data.emplace<Data::Transform>(model,
			Math::GetQuaternion(angle.x, angle.y, angle.z), std::move(position), Float3(scale, scale, scale)));
	if (Settings::ComputeMotionVectors())
		Settings::Data.emplace<Data::TransformPrevious>(model, transform);

	engine.GetAssetsStreamer().LoadModelData(engine.Gfx().GetDevice(), Settings::Data, model, file, true);
	return model;
}

EID App::AddPointLight(std::string&& name, Float3&& position,
	ColorF3&& color, float intensity, U64 range)
{
	EID light = Settings::Data.create();
	Settings::Data.emplace<std::string>(light, std::move(name));
	Settings::Data.emplace<Data::LightPoint>(light);

	Settings::Data.emplace<Data::TransformGlobal>(light,
		Settings::Data.emplace<Data::Transform>(light, Math::NoRotation(), std::move(position), Math::UnitScale()));

	Data::PointLight& pointLight = Settings::Data.emplace<Data::PointLight>(light, std::move(color), intensity, ColorF3(0.05f, 0.05f, 0.05f));
	pointLight.SetAttenuationRange(range);

	Data::PointLightBuffer& buffer = Settings::Data.emplace<Data::PointLightBuffer>(light,
		Math::GetLightVolume(pointLight.Color, pointLight.Intensity, pointLight.AttnLinear, pointLight.AttnQuad));
	//buffer.Buffer.Init(engine.Gfx().GetDevice(), &pointLight, sizeof(Data::PointLight));

	engine.Gfx().GetDevice().StartUpload();
	return light;
}

EID App::AddSpotLight(std::string&& name, Float3&& position,
	ColorF3&& color, float intensity, U64 range,
	float innerAngle, float outerAngle, const Float3& direction)
{
	EID light = Settings::Data.create();
	Settings::Data.emplace<std::string>(light, std::move(name));
	Settings::Data.emplace<Data::LightSpot>(light);

	Settings::Data.emplace<Data::TransformGlobal>(light,
		Settings::Data.emplace<Data::Transform>(light,
			Math::NoRotation(), std::move(position), Math::UnitScale()));

	Data::SpotLight& spotLight = Settings::Data.emplace<Data::SpotLight>(light,
		std::move(color), intensity, ColorF3(0.05f, 0.05f, 0.05f), Math::ToRadians(innerAngle),
		Math::NormalizeReturn(direction), Math::ToRadians(outerAngle));
	spotLight.SetAttenuationRange(range);

	Data::SpotLightBuffer& buffer = Settings::Data.emplace<Data::SpotLightBuffer>(light,
		Math::GetLightVolume(spotLight.Color, spotLight.Intensity, spotLight.AttnLinear, spotLight.AttnQuad));
	//buffer.Buffer.Init(engine.Gfx().GetDevice(), &spotLight, sizeof(Data::SpotLight));

	engine.Gfx().GetDevice().StartUpload();
	return light;
}

EID App::AddDirectionalLight(std::string&& name,
	ColorF3&& color, float intensity, const Float3& direction)
{
	EID light = Settings::Data.create();
	Settings::Data.emplace<std::string>(light, std::move(name));
	Settings::Data.emplace<Data::LightDirectional>(light);

	Data::DirectionalLight& dirLight = Settings::Data.emplace<Data::DirectionalLight>(light,
		std::move(color), intensity, ColorF3(0.05f, 0.05f, 0.05f));
	Settings::Data.emplace<Data::Direction>(light, Math::NormalizeReturn(direction));

	Data::DirectionalLightBuffer& buffer = Settings::Data.emplace<Data::DirectionalLightBuffer>(light);
	//buffer.Buffer.Init(engine.Gfx().GetDevice(), &dirLight, sizeof(Data::DirectionalLight));

	engine.Gfx().GetDevice().StartUpload();
	return light;
}

void App::MakeFrame()
{
	ZE_PERF_GUARD("Frame rendering");
	//ImGui::ShowDemoWindow();
	if (engine.IsGuiActive())
	{
		ZE_PERF_GUARD("GUI");
		ShowOptionsWindow();
		ShowObjectWindow();
	}
	engine.Reneder().UpdateWorldData(engine.Gfx().GetDevice(), currentCamera);
}

App::App(const CmdParser& params)
	: engine(SettingsInitParams::GetParsedParams(params, APP_NAME, Settings::ENGINE_VERSION, 0))
{
	EngineParams engineParams = {};
	EngineParams::SetParsedParams(params, engineParams);
	engineParams.WindowName = WINDOW_TITLE;
	engineParams.Renderer.SkyboxPath = "Skybox/Space";
	engineParams.Renderer.SkyboxExt = ".png";
	engine.Init(engineParams);

	engine.Gui().SetFont(engine.Gfx(), "Fonts/Arial.ttf", 14.0f);

	engine.Gfx().GetDevice().BeginUploadRegion();
	if (params.GetOption("cubePerfTest"))
	{
		currentCamera = AddCamera("Main camera", 0.075f, 60.0f, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f });

		AddDirectionalLight("Sun", { 0.7608f, 0.7725f, 0.8f }, 5.0f, { 0.15f, -1.0f, 0.05f });

		Data::Storage& assets = engine.GetAssetsStreamer().GetResources();

		// Create mesh for all the cubes
		EID meshId = assets.create();
		assets.emplace<std::string>(meshId, "Cube");
		assets.emplace<Math::BoundingBox>(meshId, GFX::Primitive::MakeCubeBoundingBox());

		std::vector<U32> indices = GFX::Primitive::MakeCubeIndex();
		std::vector<GFX::Vertex> vertices = GFX::Primitive::MakeCubeVertex(indices);
		GFX::Resource::MeshData meshData =
		{
			meshId,
			vertices.data(), indices.data(),
			Utils::SafeCast<U32>(vertices.size()),
			Utils::SafeCast<U32>(indices.size()),
			sizeof(GFX::Vertex), sizeof(U32)
		};
		assets.emplace<GFX::Resource::Mesh>(meshId, engine.Gfx().GetDevice(), engine.Resources().GetDisk(), meshData);

		// And some materials for them all
		std::array<EID, 255> materialIds;
		for (U32 i = 0; EID & materialId : materialIds)
		{
			materialId = assets.create();
			assets.emplace<std::string>(materialId, "Cube_mat_" + std::to_string(i++));

			Data::MaterialPBR& data = assets.emplace<Data::MaterialPBR>(materialId);
			Data::MaterialBuffersPBR& buffers = assets.emplace<Data::MaterialBuffersPBR>(materialId);
			assets.emplace<Data::PBRFlags>(materialId);

			const float seed = static_cast<float>(i) / static_cast<float>(materialIds.size());
			data.Color = { seed, seed * 0.8f, 1.0f - seed, 1.0f };
			data.Specular = { seed * 0.35f, seed, 1.0f };
			data.SpecularIntensity = 0.1f + seed;
			data.SpecularPower = 0.409f;
			data.ParallaxScale = 0.1f;

			const GFX::Resource::Texture::Schema& texSchema = engine.GetAssetsStreamer().GetSchemaLibrary().Get(Data::MaterialPBR::TEX_SCHEMA_NAME);
			GFX::Resource::Texture::PackDesc texDesc;
			texDesc.Init(texSchema);
			buffers.Init(engine.Gfx().GetDevice(), engine.Resources().GetDisk(), data, texDesc);
		}
		engine.Gfx().GetDevice().StartUpload();

		// Create test cube entities
		std::mt19937_64 randEngine;
		for (U32 i = 0, size = params.GetNumber("cubePerfTestSize"); i < size; ++i)
		{
			EID model = Settings::Data.create();
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
	else
	{
		// Sample Scene
		currentCamera = AddCamera("Main camera", 0.075f, 60.0f, { -8.0f, 0.0f, 0.0f }, { 0.0f, 90.0f, 0.0f });

		AddPointLight("Light bulb", { -20.0f, 2.0f, -4.0f }, { 1.0f, 1.0f, 1.0f }, 1.0f, 50);
		AddModel("Nanosuit", { 0.0f, -8.2f, 6.0f }, Math::NoRotationAngles(), 0.7f, "Models/nanosuit/nanosuit.obj");
		AddModel("Brick wall", { -5.0f, -2.0f, 7.0f }, Math::NoRotationAngles(), 2.0f, "Models/bricks/brick_wall.obj");

#if !_ZE_MODE_DEBUG
		AddCamera("Camera #2", 2.0f, 60.0f, { 0.0f, 40.0f, -4.0f }, { 0.0f, 45.0f, 0.0f });
		AddPointLight("Pumpkin candle", { 14.0f, -6.3f, -5.0f }, { 1.0f, 0.96f, 0.27f }, 5.0f, 85);
		AddPointLight("Blue ilumination", { 43.0f, 27.0f, 1.8f }, { 0.0f, 0.46f, 1.0f }, 10.0f, 70);
		AddPointLight("Torch", { 21.95f, -1.9f, 9.9f }, { 1.0f, 0.0f, 0.2f }, 5.0f, 70);

		AddSpotLight("Space light", { 7.5f, 60.0f, -5.0f }, { 1.3f, 2.3f, 1.3f }, 8.0f, 126, 15.0f, 24.5f, { -0.64f, -1.0f, 0.5f });
		AddSpotLight("Lion flare", { -61.0f, -6.0f, 5.0f }, { 0.8f, 0.0f, 0.8f }, 9.0f, 150, 35.0f, 45.0f, { -1.0f, 1.0f, -0.7f });
		AddSpotLight("Dragon flame", { -35.0f, -8.0f, 2.0f }, { 0.04f, 0.0f, 0.52f }, 9.0f, 175, 27.0f, 43.0f, { -0.6f, 0.75f, 0.3 });

		AddDirectionalLight("Moon", { 0.7608f, 0.7725f, 0.8f }, 0.1f, { 0.0f, -0.7f, -0.7f });

		AddModel("Sponza", { 0.0f, -8.0f, 0.0f }, Math::NoRotationAngles(), 0.045f, "Models/Sponza/sponza.obj");
		AddModel("Jack'O'Lantern", { 13.5f, -8.2f, -5.0f }, Math::NoRotationAngles(), 13.0f, "Models/Jack/Jack_O_Lantern.3ds");
		AddModel("Black dragon", { -39.0f, -8.1f, 2.0f }, { 0.0f, 290.0f, 0.0f }, 0.15f, "Models/Black Dragon/Dragon 2.5.fbx");
		AddModel("Sting sword", { -20.0f, 0.0f, -6.0f }, { 35.0f, 270.0f, 110.0f }, 0.2f, "Models/Sting_Sword/Sting_Sword.obj");
		AddModel("TIE", { 41.6f, 18.5f, 8.5f }, { 0.0f, 87.1f, 301.0f }, 3.6f, "Models/tie/tie.obj");
#endif
	}
	engine.Gfx().GetDevice().StartUpload();
	engine.Gfx().GetDevice().EndUploadRegion();
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