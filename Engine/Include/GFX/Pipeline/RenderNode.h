#pragma once
#include "GFX/QueueType.h"
#include "PassDesc.h"
#include <unordered_set>

namespace ZE::GFX::Pipeline
{
	class RenderNode final
	{
		std::string name;
		QueueType passType;
		std::unordered_set<std::string> inputs;
		std::unordered_set<std::string> outputs;
		PassExecuteCallback passExecute;
		void* executeData;

	public:
		RenderNode(std::string&& name, QueueType passType, PassExecuteCallback passExecute, void* executeData = nullptr) noexcept
			: name(std::forward<std::string>(name)), passType(passType), passExecute(passExecute), executeData(executeData) {}
		RenderNode(RenderNode&&) = default;
		RenderNode(const RenderNode&) = default;
		RenderNode& operator=(RenderNode&&) = default;
		RenderNode& operator=(const RenderNode&) = default;
		~RenderNode() = default;

		constexpr const std::string& GetName() const noexcept { return name; }
		constexpr const QueueType GetPassType() const noexcept { return passType; }
		constexpr const std::unordered_set<std::string>& GetOutputs() const noexcept { return outputs; }
		constexpr PassExecuteCallback GetExecuteCallback() const noexcept { return passExecute; }
		constexpr void* GetExecuteData() const noexcept { return executeData; }

		bool ContainsInput(const std::string& name) const noexcept { return inputs.contains(name); }
		void AddInput(std::string&& name) noexcept { inputs.emplace(std::forward<std::string>(name)); }
		void AddOutput(std::string&& name) noexcept { outputs.emplace(std::forward<std::string>(name)); }
	};
}